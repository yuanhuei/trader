#include"portfolio.h"

//portfolio类

Portfolio::Portfolio(EventEngine *eventengine, Gatewaymanager *gatewaymanager)
{
	m_eventengine = eventengine;
	m_gatewaymanager = gatewaymanager;
	is_read = false;
	m_eventengine->RegEvent(EVENT_TICK, std::bind(&Portfolio::updateportfolio, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_TIMER, std::bind(&Portfolio::recordNetValue, this, std::placeholders::_1));
}

void Portfolio::calculate()
{
	m_portfoliomtx.lock();
	//读取
	if (is_read == false)
	{
		is_read = true;
		std::vector<std::string> name;
		struct _finddata_t files;
		int File_Handle;
		int i = 0;
		File_Handle = _findfirst("./traderecord/*.csv", &files);
		if (File_Handle == -1)
		{
			printf("error\n");
			m_portfoliomtx.unlock();
			return;
		}
		do
		{
			name.push_back(Utils::split(files.name, ".")[0]);
			i++;
		} while (0 == _findnext(File_Handle, &files));
		_findclose(File_Handle);

		//读取出来所有文件名

		for (std::vector<std::string>::iterator strategyname_iter = name.begin(); strategyname_iter != name.end(); strategyname_iter++)//所有策略
		{
			std::fstream f;
			f.open("./traderecord/" + *strategyname_iter + ".csv", std::ios::in);
			if (!f.is_open())
			{
				//如果打不开文件
				std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
				e->msg = "无法读取交易记录";
				e->gatewayname = "CTP";
				m_eventengine->Put(e);
				m_portfoliomtx.unlock();
				return;
			}

			std::map<std::string, std::vector<Event_Trade>>symbol_trade;
			std::string line;
			while (!f.eof())
			{
				getline(f, line);
				std::shared_ptr<Event_Trade>e = std::make_shared<Event_Trade>();
				std::vector<std::string>v = Utils::split(line, ",");
				if (v.size() == 7)
				{
					e->tradeTime = v[1];
					e->symbol = v[2];
					e->direction = v[3];
					e->offset = v[4];
					e->price = atof(v[5].c_str());
					e->volume = atof(v[6].c_str());

					if (symbol_trade.find(e->symbol) != symbol_trade.end())
					{
						//有就直接用
						symbol_trade[e->symbol].push_back(*e);
					}
					else
					{
						//创建
						std::vector<Event_Trade>trade_v;
						trade_v.push_back(*e);
						symbol_trade.insert(std::pair<std::string, std::vector<Event_Trade>>(e->symbol, trade_v));
					}
				}
			}
			f.close();
			m_tradelist.insert(std::pair<std::string, std::map<std::string, std::vector<Event_Trade>>>((*strategyname_iter), symbol_trade));
		}



		//计算portfolio
		for (std::map<std::string, std::map<std::string, std::vector<Event_Trade>>>::iterator name_trade_iter = m_tradelist.begin(); name_trade_iter != m_tradelist.end(); name_trade_iter++)
		{
			//遍历每一个策略
			Result res;
			std::shared_ptr<Event_UpdatePortfolio>eUpdatePortfolio = std::make_shared<Event_UpdatePortfolio>();
			eUpdatePortfolio->strategyname = name_trade_iter->first;
			eUpdatePortfolio->strategyrows.push_back(name_trade_iter->second.size());
			for (std::map<std::string, std::vector<Event_Trade>>::iterator symbol_trade_iter = name_trade_iter->second.begin(); symbol_trade_iter != name_trade_iter->second.end(); symbol_trade_iter++)
			{
				//遍历每一个合约

				std::vector<Event_Trade>longTrade_v;//未平仓多头交易
				std::vector<Event_Trade>shortTrade_v;//未平仓空头交易

				std::vector<TradingResult>resultList;//交易结果列表


				for (std::vector<Event_Trade>::iterator etradeiter = symbol_trade_iter->second.begin(); etradeiter != symbol_trade_iter->second.end(); etradeiter++)//遍历所有的单
				{
					//遍历所有etrade
					//多头
					if ((etradeiter)->direction == DIRECTION_LONG)
					{
						if (shortTrade_v.empty())
						{
							longTrade_v.push_back(*etradeiter);
						}
						else
						{
							Event_Trade exitTrade = *etradeiter;
							while (true)
							{
								Event_Trade *entryTrade = &shortTrade_v[0];
								//清算开平仓交易
								double closedVolume = std::min(exitTrade.volume, entryTrade->volume);
								TradingResult result = TradingResult(entryTrade->price, entryTrade->tradeTime, exitTrade.price, exitTrade.tradeTime, -closedVolume, m_gatewaymanager->getContract(symbol_trade_iter->first)->size);
								resultList.push_back(result);

								//计算未清算部分
								entryTrade->volume -= closedVolume;
								exitTrade.volume -= closedVolume;

								//如果开仓交易经清算
								if (entryTrade->volume == 0)
								{
									shortTrade_v.erase(shortTrade_v.begin());
								}

								//如果平仓交易已经清算退出循环
								if (exitTrade.volume == 0)
								{
									break;
								}

								//如果平仓未全部清算
								if (exitTrade.volume)
								{
									// 且开仓交易已经全部清算完，则平仓交易剩余的部分
									// 等于新的反向开仓交易，添加到队列中
									if (shortTrade_v.empty())
									{
										longTrade_v.push_back(exitTrade);
										break;
									}
								}
							}
						}
					}
					//空头
					else
					{
						//如果尚无多头交易
						if (longTrade_v.empty())
						{
							shortTrade_v.push_back(*etradeiter);
						}
						else
						{
							Event_Trade exitTrade = *etradeiter;
							while (true)
							{
								Event_Trade *entryTrade = &longTrade_v[0];
								//清算开平仓交易

								double closedVolume = std::min(exitTrade.volume, entryTrade->volume);
								TradingResult result = TradingResult(entryTrade->price, entryTrade->tradeTime, exitTrade.price, entryTrade->tradeTime, closedVolume, m_gatewaymanager->getContract(symbol_trade_iter->first)->size);
								resultList.push_back(result);

								//计算未清算部分

								entryTrade->volume -= closedVolume;
								exitTrade.volume -= closedVolume;

								//如果开仓交易已经全部清算，则从列表中移除
								if (entryTrade->volume == 0)
								{
									longTrade_v.erase(longTrade_v.begin());
								}

								//如果平仓交易已经全部清算，则退出循环
								if (exitTrade.volume == 0)
								{
									break;
								}
								//如果平仓交易未全部清算
								if (exitTrade.volume)
								{
									//且开仓交易已经全部清算完，则平仓交易剩余的部分
									// 等于新的反向开仓交易，添加到队列中
									if (longTrade_v.empty())
									{
										shortTrade_v.push_back(exitTrade);
										break;
									}
								}
							}
						}
					}
					if (etradeiter == (symbol_trade_iter->second.end() - 1))
					{
						//最后一句了
						if (!shortTrade_v.empty())
						{
							//创建
							std::vector<Event_Trade>trade_v;
							for (std::vector<Event_Trade>::iterator shortTradeit = shortTrade_v.begin(); shortTradeit != shortTrade_v.end(); shortTradeit++)
							{
								trade_v.push_back(*shortTradeit);
							}

							if (m_tradelist_memory.find(name_trade_iter->first) != m_tradelist_memory.end())
							{
								if (m_tradelist_memory[name_trade_iter->first].find((etradeiter)->symbol) != m_tradelist_memory[name_trade_iter->first].end())
								{
									m_tradelist_memory[name_trade_iter->first][(etradeiter)->symbol].clear();
									m_tradelist_memory[name_trade_iter->first][(etradeiter)->symbol] = trade_v;
								}
								else
								{
									m_tradelist_memory[name_trade_iter->first].insert(std::pair<std::string, std::vector<Event_Trade>>((etradeiter)->symbol, trade_v));
								}
							}
							else
							{
								std::map<std::string, std::vector<Event_Trade>>symbol_trade;
								symbol_trade.insert(std::pair<std::string, std::vector<Event_Trade>>((etradeiter)->symbol, trade_v));
								m_tradelist_memory.insert(std::pair<std::string, std::map<std::string, std::vector<Event_Trade>>>(name_trade_iter->first, symbol_trade));
							}
						}
						if (!longTrade_v.empty())
						{

							std::vector<Event_Trade>trade_v;
							for (std::vector<Event_Trade>::iterator longTradeit = longTrade_v.begin(); longTradeit != longTrade_v.end(); longTradeit++)
							{
								trade_v.push_back(*longTradeit);
							}

							if (m_tradelist_memory.find(name_trade_iter->first) != m_tradelist_memory.end())
							{
								if (m_tradelist_memory[name_trade_iter->first].find((etradeiter)->symbol) != m_tradelist_memory[name_trade_iter->first].end())
								{
									m_tradelist_memory[name_trade_iter->first][(etradeiter)->symbol].clear();
									m_tradelist_memory[name_trade_iter->first][(etradeiter)->symbol] = trade_v;
								}
								else
								{
									m_tradelist_memory[name_trade_iter->first].insert(std::pair<std::string, std::vector<Event_Trade>>((etradeiter)->symbol, trade_v));
								}
							}
							else
							{
								std::map<std::string, std::vector<Event_Trade>>symbol_trade;
								symbol_trade.insert(std::pair<std::string, std::vector<Event_Trade>>((etradeiter)->symbol, trade_v));
								m_tradelist_memory.insert(std::pair<std::string, std::map<std::string, std::vector<Event_Trade>>>(name_trade_iter->first, symbol_trade));
							}
						}
					}
				}
				//traderesult
				if (resultList.empty())
				{
					std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
					e->msg = "交易留底还没完成第一笔交易";
					e->gatewayname = "CTP";
					m_eventengine->Put(e);
				}

				//计算每一笔结果
				double  totalwinning = 0;
				double	maxCapital = 0;
				double	drawdown = 0;
				double	Winning = 0;
				double	Losing = 0;
				int totalResult = 0;

				double holdingwinning = 0;//持仓盈亏
				double holdingposition = 0;
				double holdingprice = 0;


				//计算持仓
				double totalcost = 0;
				for (std::vector<Event_Trade>::iterator symbol_tradememory_iter = m_tradelist_memory[name_trade_iter->first][symbol_trade_iter->first].begin();
					symbol_tradememory_iter != m_tradelist_memory[name_trade_iter->first][symbol_trade_iter->first].end(); symbol_tradememory_iter++)
				{
					totalcost += ((symbol_tradememory_iter)->volume) * ((symbol_tradememory_iter)->price);
					if ((symbol_tradememory_iter)->direction == DIRECTION_LONG)
					{
						holdingposition += (symbol_tradememory_iter)->volume;
					}
					else
					{
						holdingposition -= (symbol_tradememory_iter)->volume;
					}
				}
				holdingprice = totalcost / std::abs(holdingposition);

				for (std::vector<TradingResult>::iterator resultiter = resultList.begin(); resultiter != resultList.end(); resultiter++)
				{
					totalwinning += resultiter->m_pnl;
					maxCapital = std::max(totalwinning, maxCapital);
					drawdown = std::min(drawdown, totalwinning - maxCapital);
					totalResult += 1;

					if (resultiter->m_pnl >= 0)
					{
						Winning += resultiter->m_pnl;
					}
					else
					{
						Losing += resultiter->m_pnl;
					}
				}
				//存储结果
				UnitResult unitres;
				unitres.totalwinning = totalwinning;
				unitres.maxCapital = maxCapital;
				unitres.drawdown = drawdown;
				unitres.Winning = Winning;
				unitres.Losing = Losing;
				unitres.totalResult = totalResult;
				unitres.holdingwinning = holdingwinning;//持仓盈亏
				unitres.holdingposition = holdingposition;
				unitres.holdingprice = holdingprice;
				unitres.lastdayTotalwinning = totalwinning;
				unitres.delta = totalwinning - unitres.lastdayTotalwinning;
				res.insert(std::pair<std::string, UnitResult>(symbol_trade_iter->first, unitres));


				if (m_result.find(name_trade_iter->first) != m_result.end())
				{
					//有这个策略
					m_result[name_trade_iter->first] = res;
				}
				else
				{
					//没有这个策略
					m_result.insert(std::pair<std::string, Result>(name_trade_iter->first, res));
				}


				eUpdatePortfolio->symbol = symbol_trade_iter->first;
				eUpdatePortfolio->Portfoliodata.drawdown = unitres.drawdown;
				eUpdatePortfolio->Portfoliodata.holdingposition = unitres.holdingposition;
				eUpdatePortfolio->Portfoliodata.holdingprice = unitres.holdingprice;
				eUpdatePortfolio->Portfoliodata.holdingwinning = unitres.holdingwinning;
				eUpdatePortfolio->Portfoliodata.Losing = unitres.Losing;
				eUpdatePortfolio->Portfoliodata.maxCapital = unitres.maxCapital;
				eUpdatePortfolio->Portfoliodata.totalResult = unitres.totalResult;
				eUpdatePortfolio->Portfoliodata.totalwinning = unitres.totalwinning;
				eUpdatePortfolio->Portfoliodata.Winning = unitres.Winning;
				eUpdatePortfolio->Portfoliodata.holding_and_totalwinning = unitres.totalwinning + unitres.holdingwinning;
				eUpdatePortfolio->Portfoliodata.portfolio_winning = 0;
				m_eventengine->Put(eUpdatePortfolio);
			}
		}
	}
	m_portfoliomtx.unlock();
}

void Portfolio::calculate_memory(std::shared_ptr<Event_Trade>etrade, std::map<std::string, StrategyTemplate*>orderStrategymap)
{
	//计算平仓
	//m_tradelist_memory
	//m_result
	m_portfoliomtx.lock();
	if (orderStrategymap[etrade->orderID] == nullptr)
	{
		m_portfoliomtx.unlock();
		return;
	}

	std::string strategyname = orderStrategymap[etrade->orderID]->getparam("name");
	std::vector<Event_Trade>longTrade_v;//未平仓多头交易
	std::vector<Event_Trade>shortTrade_v;//未平仓空头交易
	std::vector<TradingResult>resultList;//交易结果列表
	std::unique_lock<std::mutex>lck(m_tradelistmtx);
	if (m_tradelist_memory.find(strategyname) != m_tradelist_memory.end())
	{
		if (m_tradelist_memory[strategyname].find(etrade->symbol) != m_tradelist_memory[strategyname].end())
		{
			for (std::vector<Event_Trade>::iterator tradeiter = m_tradelist_memory[strategyname][etrade->symbol].begin(); tradeiter != m_tradelist_memory[strategyname][etrade->symbol].end(); tradeiter++)
			{
				if ((tradeiter)->direction == DIRECTION_LONG)
				{
					longTrade_v.push_back(*tradeiter);
				}
				else
				{
					shortTrade_v.push_back(*tradeiter);
				}
			}
			m_tradelist_memory[strategyname][etrade->symbol].clear();//	清空，在最底下重新加入新的vector
		}
	}

	if ((etrade)->direction == DIRECTION_LONG)
	{
		if (shortTrade_v.empty())
		{
			longTrade_v.push_back(*etrade);
		}
		else
		{
			Event_Trade exitTrade = *etrade;
			while (true)
			{
				Event_Trade *entryTrade = &shortTrade_v[0];
				//清算开平仓交易
				double closedVolume = std::min(exitTrade.volume, entryTrade->volume);
				TradingResult result = TradingResult(entryTrade->price, entryTrade->tradeTime, exitTrade.price,
					exitTrade.tradeTime, -closedVolume, m_gatewaymanager->getContract(etrade->symbol)->size);
				resultList.push_back(result);

				//计算未清算部分
				entryTrade->volume -= closedVolume;
				exitTrade.volume -= closedVolume;

				//如果开仓交易经清算
				if (entryTrade->volume == 0)
				{
					shortTrade_v.erase(shortTrade_v.begin());
				}

				//如果平仓交易已经清算退出循环
				if (exitTrade.volume == 0)
				{
					break;
				}

				//如果平仓未全部清算
				if (exitTrade.volume)
				{
					// 且开仓交易已经全部清算完，则平仓交易剩余的部分
					// 等于新的反向开仓交易，添加到队列中
					if (shortTrade_v.empty())
					{
						longTrade_v.push_back(exitTrade);
						break;
					}
				}
			}
		}
	}
	//空头
	else
	{
		//如果尚无多头交易
		if (longTrade_v.empty())
		{
			shortTrade_v.push_back(*etrade);
		}
		else
		{
			Event_Trade exitTrade = *etrade;
			while (true)
			{
				Event_Trade *entryTrade = &longTrade_v[0];
				//清算开平仓交易

				double closedVolume = std::min(exitTrade.volume, entryTrade->volume);
				TradingResult result = TradingResult(entryTrade->price, entryTrade->tradeTime, exitTrade.price, entryTrade->tradeTime, closedVolume, m_gatewaymanager->getContract(etrade->symbol)->size);
				resultList.push_back(result);

				//计算未清算部分

				entryTrade->volume -= closedVolume;
				exitTrade.volume -= closedVolume;

				//如果开仓交易已经全部清算，则从列表中移除
				if (entryTrade->volume == 0)
				{
					longTrade_v.erase(longTrade_v.begin());
				}

				//如果平仓交易已经全部清算，则退出循环
				if (exitTrade.volume == 0)
				{
					break;
				}
				//如果平仓交易未全部清算
				if (exitTrade.volume)
				{
					//且开仓交易已经全部清算完，则平仓交易剩余的部分
					// 等于新的反向开仓交易，添加到队列中
					if (longTrade_v.empty())
					{
						shortTrade_v.push_back(exitTrade);
						break;
					}
				}
			}
		}
	}
	//最后一句了
	if (!shortTrade_v.empty())
	{
		//创建
		std::vector<Event_Trade>trade_v;
		for (std::vector<Event_Trade>::iterator shortTradeit = shortTrade_v.begin(); shortTradeit != shortTrade_v.end(); shortTradeit++)
		{
			trade_v.push_back(*shortTradeit);
		}
		if (m_tradelist_memory.find(strategyname) != m_tradelist_memory.end())
		{
			m_tradelist_memory[strategyname][etrade->symbol] = trade_v;
		}
		else
		{
			std::map<std::string, std::vector<Event_Trade>>symbol_vector;
			symbol_vector.insert(std::pair<std::string, std::vector<Event_Trade>>(etrade->symbol, trade_v));
			m_tradelist_memory.insert(std::pair<std::string, std::map<std::string, std::vector<Event_Trade>>>(strategyname, symbol_vector));
		}

	}

	if (!longTrade_v.empty())
	{
		std::vector<Event_Trade>trade_v;
		for (std::vector<Event_Trade>::iterator longTradeit = longTrade_v.begin(); longTradeit != longTrade_v.end(); longTradeit++)
		{
			trade_v.push_back(*longTradeit);
		}
		if (m_tradelist_memory.find(strategyname) != m_tradelist_memory.end())
		{
			m_tradelist_memory[strategyname][etrade->symbol] = trade_v;
		}
		else
		{
			std::map<std::string, std::vector<Event_Trade>>symbol_vector;
			symbol_vector.insert(std::pair<std::string, std::vector<Event_Trade>>(etrade->symbol, trade_v));
			m_tradelist_memory.insert(std::pair<std::string, std::map<std::string, std::vector<Event_Trade>>>(strategyname, symbol_vector));
		}
	}

	//计算每一笔结果

	double  totalwinning = 0;
	double	maxCapital = 0;
	double	drawdown = 0;
	double	Winning = 0;
	double	Losing = 0;
	int totalResult = 0;

	double holdingwinning = 0;
	double holdingposition = 0;
	double holdingprice = 0;

	if (m_result.find(strategyname) != m_result.end())
	{

		totalwinning = m_result[strategyname][etrade->symbol].totalwinning;
		maxCapital = m_result[strategyname][etrade->symbol].maxCapital;
		drawdown = m_result[strategyname][etrade->symbol].drawdown;
		Winning = m_result[strategyname][etrade->symbol].Winning;
		Losing = m_result[strategyname][etrade->symbol].Losing;
		totalResult = m_result[strategyname][etrade->symbol].totalResult;
		//计算持仓
		double totalcost = 0;
		for (std::vector<Event_Trade>::iterator symbol_tradememory_iter = m_tradelist_memory[strategyname][etrade->symbol].begin();
			symbol_tradememory_iter != m_tradelist_memory[strategyname][etrade->symbol].end(); symbol_tradememory_iter++)
		{
			totalcost += ((symbol_tradememory_iter)->volume) * ((symbol_tradememory_iter)->price);
			if ((symbol_tradememory_iter)->direction == DIRECTION_LONG)
			{
				holdingposition += (symbol_tradememory_iter)->volume;
			}
			else if ((symbol_tradememory_iter)->direction == DIRECTION_SHORT)
			{
				holdingposition -= (symbol_tradememory_iter)->volume;
			}
		}
		holdingprice = totalcost / std::abs(holdingposition);

		for (std::vector<TradingResult>::iterator resultiter = resultList.begin(); resultiter != resultList.end(); resultiter++)
		{
			totalwinning += resultiter->m_pnl;
			maxCapital = std::max(totalwinning, maxCapital);
			drawdown = std::min(drawdown, totalwinning - maxCapital);
			totalResult += 1;

			if (resultiter->m_pnl >= 0)
			{
				Winning += resultiter->m_pnl;
			}
			else
			{
				Losing += resultiter->m_pnl;
			}
		}
		//存储结果

		m_result[strategyname][etrade->symbol].totalwinning = totalwinning;
		m_result[strategyname][etrade->symbol].maxCapital = maxCapital;
		m_result[strategyname][etrade->symbol].drawdown = drawdown;
		m_result[strategyname][etrade->symbol].Winning = Winning;
		m_result[strategyname][etrade->symbol].Losing = Losing;
		m_result[strategyname][etrade->symbol].totalResult = totalResult;
		m_result[strategyname][etrade->symbol].holdingwinning = holdingwinning;//持仓盈亏
		m_result[strategyname][etrade->symbol].holdingposition = holdingposition;
		m_result[strategyname][etrade->symbol].holdingprice = holdingprice;

	}
	else
	{
		Result res;
		UnitResult unitres;
		unitres.totalwinning = totalwinning;
		unitres.maxCapital = maxCapital;
		unitres.drawdown = drawdown;
		unitres.Winning = Winning;
		unitres.Losing = Losing;
		unitres.holdingwinning = holdingwinning;//持仓盈亏
		unitres.holdingposition = holdingposition;
		unitres.holdingprice = holdingprice;
		res.insert(std::pair<std::string, UnitResult>(etrade->symbol, unitres));
		m_result.insert(std::pair<std::string, Result>(strategyname, res));

		//计算持仓
		double totalcost = 0;
		for (std::vector<Event_Trade>::iterator symbol_tradememory_iter = m_tradelist_memory[strategyname][etrade->symbol].begin();
			symbol_tradememory_iter != m_tradelist_memory[strategyname][etrade->symbol].end(); symbol_tradememory_iter++)
		{
			totalcost += ((symbol_tradememory_iter)->volume) * ((symbol_tradememory_iter)->price);
			if ((symbol_tradememory_iter)->direction == DIRECTION_LONG)
			{
				holdingposition += (symbol_tradememory_iter)->volume;
			}
			else
			{
				holdingposition -= (symbol_tradememory_iter)->volume;
			}
		}
		holdingprice = totalcost / std::abs(holdingposition);

		for (std::vector<TradingResult>::iterator resultiter = resultList.begin(); resultiter != resultList.end(); resultiter++)
		{
			totalwinning += resultiter->m_pnl;
			maxCapital = std::max(totalwinning, maxCapital);
			drawdown = std::min(drawdown, totalwinning - maxCapital);
			totalResult += 1;

			if (resultiter->m_pnl >= 0)
			{
				Winning += resultiter->m_pnl;
			}
			else
			{
				Losing += resultiter->m_pnl;
			}
		}
		//存储结果
		m_result[strategyname][etrade->symbol].totalwinning = totalwinning;
		m_result[strategyname][etrade->symbol].maxCapital = maxCapital;
		m_result[strategyname][etrade->symbol].drawdown = drawdown;
		m_result[strategyname][etrade->symbol].Winning = Winning;
		m_result[strategyname][etrade->symbol].Losing = Losing;
		m_result[strategyname][etrade->symbol].totalResult = totalResult;
		m_result[strategyname][etrade->symbol].holdingwinning = holdingwinning;//持仓盈亏
		m_result[strategyname][etrade->symbol].holdingposition = holdingposition;
		m_result[strategyname][etrade->symbol].holdingprice = holdingprice;
	}
	m_portfoliomtx.unlock();
}

void Portfolio::updateportfolio(std::shared_ptr<Event>e)
{
	std::shared_ptr<Event_Tick> etick = std::static_pointer_cast<Event_Tick>(e);
	m_portfoliomtx.lock();
	for (std::map<std::string, Result>::iterator it = m_result.begin(); it != m_result.end(); it++)
	{
		std::shared_ptr<Event_UpdatePortfolio>eUpdatePortfolio = std::make_shared<Event_UpdatePortfolio>();
		eUpdatePortfolio->strategyname = it->first;
		eUpdatePortfolio->symbol = etick->symbol;
		eUpdatePortfolio->strategyrows.push_back(it->second.size());
		for (std::map<std::string, UnitResult>::iterator iter = it->second.begin(); iter != it->second.end(); iter++)
		{
			//遍历所有合约
			if (iter->first == etick->symbol)
			{
				if (iter->second.holdingposition > 0)
				{
					iter->second.holdingwinning = (etick->lastprice - iter->second.holdingprice)*iter->second.holdingposition*m_gatewaymanager->getContract(etick->symbol)->size;
				}
				else if (iter->second.holdingposition < 0)
				{
					iter->second.holdingwinning = (iter->second.holdingprice - etick->lastprice)*(-iter->second.holdingposition)*m_gatewaymanager->getContract(etick->symbol)->size;
				}
				else if (iter->second.holdingposition == 0)
				{
					iter->second.holdingwinning = 0;
				}
				eUpdatePortfolio->Portfoliodata.drawdown = iter->second.drawdown;
				eUpdatePortfolio->Portfoliodata.holdingposition = iter->second.holdingposition;
				eUpdatePortfolio->Portfoliodata.holdingprice = iter->second.holdingprice;
				eUpdatePortfolio->Portfoliodata.holdingwinning = iter->second.holdingwinning;
				eUpdatePortfolio->Portfoliodata.Losing = iter->second.Losing;
				eUpdatePortfolio->Portfoliodata.maxCapital = iter->second.maxCapital;
				eUpdatePortfolio->Portfoliodata.totalResult = iter->second.totalResult;
				eUpdatePortfolio->Portfoliodata.totalwinning = iter->second.totalwinning;
				eUpdatePortfolio->Portfoliodata.Winning = iter->second.Winning;
				eUpdatePortfolio->Portfoliodata.holding_and_totalwinning = iter->second.totalwinning + iter->second.holdingwinning;
				eUpdatePortfolio->Portfoliodata.delta = eUpdatePortfolio->Portfoliodata.holding_and_totalwinning - iter->second.lastdayTotalwinning;
				double portfolio_winning = 0;
				for (std::map<std::string, Result>::iterator it_temp1 = m_result.begin(); it_temp1 != m_result.end(); it_temp1++)
				{
					for (std::map<std::string, UnitResult>::iterator it_temp2 = it_temp1->second.begin(); it_temp2 != it_temp1->second.end(); it_temp2++)
					{
						portfolio_winning += (it_temp2->second.holdingwinning + it_temp2->second.totalwinning);
					}
				}
				eUpdatePortfolio->Portfoliodata.portfolio_winning = portfolio_winning;
				m_eventengine->Put(eUpdatePortfolio);
			}
		}
	}
	m_portfoliomtx.unlock();
}


void Portfolio::recordNetValue(std::shared_ptr<Event>e)
{
	auto nowtime2 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	if (((nowtime2 > Utils::timetounixtimestamp(15, 00, 0)) && (nowtime2 < Utils::timetounixtimestamp(15, 01, 0))))
	{
		if (netValueRecorded == false)
		{
			netValueRecorded = true;
			for (std::map<std::string, Result>::iterator it = m_result.begin(); it != m_result.end(); it++)
			{

				for (std::map<std::string, UnitResult>::iterator iter = it->second.begin(); iter != it->second.end(); iter++)
				{
					std::fstream file;
					file.open("./Capital/" + it->first + iter->first + ".csv", std::ios::app | std::ios::out);
					if (file.is_open())
					{
						file << Utils::getCurrentSystemDate() << "," << iter->second.totalwinning << "\n";
						file.close();
					}
					iter->second.lastdayTotalwinning = iter->second.totalwinning;
				}
			}
			std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
			e->msg = "更新昨日净值";
			e->gatewayname = "CTP";
			m_eventengine->Put(e);


		}
	}
	else
	{
		//刷新状态
		if (netValueRecorded == true)
		{
			netValueRecorded = false;
		}
	}

}

void Portfolio::writelog(std::string log)
{
	std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
	e->msg = log;
	e->gatewayname = "Portfolio";
	m_eventengine->Put(e);
}

TradingResult::TradingResult(double entryPrice, std::string entryDt, double exitPrice, std::string exitDt, double volume, double size)
{
	//清算
	m_entryPrice = entryPrice;  //开仓价格
	m_exitPrice = exitPrice;    // 平仓价格
	m_entryDt = entryDt;        // 开仓时间datetime
	m_exitDt = exitDt;          // 平仓时间
	m_volume = volume;			//交易数量（ + / -代表方向）
	m_pnl = ((m_exitPrice - m_entryPrice) * volume * size);  //净盈亏
};