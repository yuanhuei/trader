#include"AlgorithmOrder.h"
algorithmOrder::algorithmOrder(double unitLimit,enum Mode TradingMode, StrategyTemplate *strategy_ptr)
{
	m_TradingMode = TradingMode;
	m_strategy_ptr = strategy_ptr;
	m_unitLimit = unitLimit;//默认值
	stopMode = 999;
}

void algorithmOrder::setunitLimit(int unitLimit)
{
	m_unitLimit = unitLimit;
}

void algorithmOrder::setTradingMode(enum Mode TradingMode)
{
	m_TradingMode = TradingMode;
}

void algorithmOrder::checkPositions_Tick(const TickData *Tick)
{
	if (m_TradingMode == BacktestMode)
	{
		m_unitLimit = 999999;
	}
	std::string symbol = Tick->symbol;
	m_supposedPosmtx.lock();
	if (m_supposedPos[symbol] == m_strategy_ptr->getpos(symbol))//假设的仓位和真实仓位相等 不需要再加减仓了
	{
		m_supposedPosmtx.unlock();
		return;
	}


	if (Tick->bidprice1 == Tick->upperLimit || Tick->askprice1 == Tick->lowerLimit)
	{
		m_supposedPosmtx.unlock();
		return;
	}

	m_orderIDvmtx.lock();
	if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
	{
		for (std::vector<std::string>::iterator it = orderID_Vector_Map[symbol].begin(); it != orderID_Vector_Map[symbol].end(); it++)
		{
			m_strategy_ptr->cancelOrder(*it, m_strategy_ptr->gatewayname);
		}
	}
	m_orderIDvmtx.unlock();


	if (m_TradingMode == RealMode)
	{
		if (m_strategy_ptr->getpos(symbol) == 0)//如果空仓，可以直接开
		{
			if (m_supposedPos[symbol] > m_strategy_ptr->getpos(symbol))//开多
			{
				m_orderIDvmtx.lock();
				if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
				{
					orderID_Vector_Map[symbol].clear();
					std::vector<std::string>v;
					v = m_strategy_ptr->buy(false,Tick->askprice1, std::min(m_supposedPos[symbol] - m_strategy_ptr->getpos(symbol), m_unitLimit));
					for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
					{
						orderID_Vector_Map[symbol].push_back(*it);
					}
				}
				else
				{
					std::vector<std::string>v;
					v = m_strategy_ptr->buy( false,Tick->askprice1, std::min(m_supposedPos[symbol] - m_strategy_ptr->getpos(symbol), m_unitLimit));
					orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
				}
				m_orderIDvmtx.unlock();
			}
			else if (m_supposedPos[symbol] < m_strategy_ptr->getpos(symbol))//开空
			{
				m_orderIDvmtx.lock();
				if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
				{
					orderID_Vector_Map[symbol].clear();
					std::vector<std::string>v;
					v = m_strategy_ptr->sellshort(Tick->bidprice1, std::min(m_strategy_ptr->getpos(symbol) - m_supposedPos[symbol], m_unitLimit));
					for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
					{
						orderID_Vector_Map[symbol].push_back(*it);
					}
				}
				else
				{
					std::vector<std::string>v;
					v = m_strategy_ptr->sellshort(Tick->bidprice1, std::min(m_strategy_ptr->getpos(symbol) - m_supposedPos[symbol], m_unitLimit));
					orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
				}
				m_orderIDvmtx.unlock();
			}
		}
		else if (m_strategy_ptr->getpos(symbol) > 0)//现在有多头
		{
			if (m_supposedPos[symbol] > m_strategy_ptr->getpos(symbol))//加多
			{
				m_orderIDvmtx.lock();
				if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
				{
					orderID_Vector_Map[symbol].clear();
					std::vector<std::string>v;
					v = m_strategy_ptr->buy( Tick->askprice1, std::min(m_supposedPos[symbol] - m_strategy_ptr->getpos(symbol), m_unitLimit));
					for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
					{
						orderID_Vector_Map[symbol].push_back(*it);
					}
				}
				else
				{
					std::vector<std::string>v;
					v = m_strategy_ptr->buy(Tick->askprice1, std::min(m_supposedPos[symbol] - m_strategy_ptr->getpos(symbol), m_unitLimit));
					orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
				}
				m_orderIDvmtx.unlock();
			}
			else if (m_supposedPos[symbol] < m_strategy_ptr->getpos(symbol))//减仓或翻空
			{
				if (m_supposedPos[symbol] >= 0)//减仓
				{
					m_orderIDvmtx.lock();
					if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
					{
						orderID_Vector_Map[symbol].clear();
						std::vector<std::string>v;
						v = m_strategy_ptr->sell(Tick->bidprice1, std::min(m_strategy_ptr->getpos(symbol) - m_supposedPos[symbol], m_unitLimit));
						for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
						{
							orderID_Vector_Map[symbol].push_back(*it);
						}
					}
					else
					{
						std::vector<std::string>v;
						v = m_strategy_ptr->sell(Tick->bidprice1, std::min(m_strategy_ptr->getpos(symbol) - m_supposedPos[symbol], m_unitLimit));
						orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
					}
					m_orderIDvmtx.unlock();
				}
				else//全平掉等着翻空
				{
					m_orderIDvmtx.lock();
					if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
					{
						orderID_Vector_Map[symbol].clear();
						std::vector<std::string>v;
						v = m_strategy_ptr->sell(Tick->bidprice1, std::min(m_strategy_ptr->getpos(symbol), m_unitLimit));
						for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
						{
							orderID_Vector_Map[symbol].push_back(*it);
						}
					}
					else
					{
						std::vector<std::string>v;
						v = m_strategy_ptr->sell(Tick->bidprice1, std::min(m_strategy_ptr->getpos(symbol), m_unitLimit));
						orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
					}
					m_orderIDvmtx.unlock();
				}
			}
		}
		else if (m_strategy_ptr->getpos(symbol) < 0)//有空头
		{
			if (m_supposedPos[symbol] > m_strategy_ptr->getpos(symbol))//减仓或者翻多
			{
				if (m_supposedPos[symbol] <= 0)//减仓
				{
					m_orderIDvmtx.lock();
					if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
					{
						orderID_Vector_Map[symbol].clear();
						std::vector<std::string>v;
						v = m_strategy_ptr->buycover(Tick->askprice1, std::min(m_supposedPos[symbol] - m_strategy_ptr->getpos(symbol), m_unitLimit));
						for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
						{
							orderID_Vector_Map[symbol].push_back(*it);
						}
					}
					else
					{
						std::vector<std::string>v;
						v = m_strategy_ptr->buycover(Tick->askprice1, std::min(m_supposedPos[symbol] - m_strategy_ptr->getpos(symbol), m_unitLimit));
						orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
					}
					m_orderIDvmtx.unlock();
				}
				else//全平掉等着翻多
				{
					m_orderIDvmtx.lock();
					if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
					{
						orderID_Vector_Map[symbol].clear();
						std::vector<std::string>v;
						v = m_strategy_ptr->buycover(Tick->askprice1, std::min(std::abs(m_strategy_ptr->getpos(symbol)), m_unitLimit));
						for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
						{
							orderID_Vector_Map[symbol].push_back(*it);
						}
					}
					else
					{
						std::vector<std::string>v;
						v = m_strategy_ptr->buycover(Tick->askprice1, std::min(std::abs(m_strategy_ptr->getpos(symbol)), m_unitLimit));
						orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
					}
					m_orderIDvmtx.unlock();
				}
			}
			else if (m_supposedPos[symbol] < m_strategy_ptr->getpos(symbol))//加空
			{
				m_orderIDvmtx.lock();
				if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
				{
					orderID_Vector_Map[symbol].clear();
					std::vector<std::string>v;
					v = m_strategy_ptr->sellshort(Tick->bidprice1, std::min(m_strategy_ptr->getpos(symbol) - m_supposedPos[symbol], m_unitLimit));
					for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
					{
						orderID_Vector_Map[symbol].push_back(*it);
					}
				}
				else
				{
					std::vector<std::string>v;
					v = m_strategy_ptr->sellshort(Tick->bidprice1, std::min(m_strategy_ptr->getpos(symbol) - m_supposedPos[symbol], m_unitLimit));
					orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
				}
				m_orderIDvmtx.unlock();
			}
		}
	}
	else
	{
		//回测模式  照上面复制进行修改
	}
	m_supposedPosmtx.unlock();
}

void algorithmOrder::checkPositions_Bar(const BarData *Bar)
{
	if (m_TradingMode == BacktestMode)
	{
		m_unitLimit = 999999;
	}
	std::string symbol = Bar->symbol;
	m_supposedPosmtx.lock();
	if (m_supposedPos[symbol] == m_strategy_ptr->getpos(symbol))//假设的仓位和真实仓位相等 不需要再加减仓了
	{
		m_supposedPosmtx.unlock();
		return;
	}

	m_orderIDvmtx.lock();
	if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
	{
		for (std::vector<std::string>::iterator it = orderID_Vector_Map[symbol].begin(); it != orderID_Vector_Map[symbol].end(); it++)
		{
			m_strategy_ptr->cancelOrder(*it, m_strategy_ptr->gatewayname);
		}
	}
	m_orderIDvmtx.unlock();

	if (m_TradingMode == RealMode)
	{
		if (m_strategy_ptr->getpos(symbol) == 0)//如果空仓，可以直接开
		{
			if (m_supposedPos[symbol] > m_strategy_ptr->getpos(symbol))//开多
			{
				m_orderIDvmtx.lock();
				if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
				{
					orderID_Vector_Map[symbol].clear();
					std::vector<std::string>v;
					v = m_strategy_ptr->buy(Bar->high, std::min(m_supposedPos[symbol] - m_strategy_ptr->getpos(symbol), m_unitLimit));
					for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
					{
						orderID_Vector_Map[symbol].push_back(*it);
					}
				}
				else
				{
					std::vector<std::string>v;
					v = m_strategy_ptr->buy(Bar->high, std::min(m_supposedPos[symbol] - m_strategy_ptr->getpos(symbol), m_unitLimit));
					orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
				}
				m_orderIDvmtx.unlock();
			}
			else if (m_supposedPos[symbol] < m_strategy_ptr->getpos(symbol))//开空
			{
				m_orderIDvmtx.lock();
				if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
				{
					orderID_Vector_Map[symbol].clear();
					std::vector<std::string>v;
					v = m_strategy_ptr->sellshort(Bar->low, std::min(m_strategy_ptr->getpos(symbol) - m_supposedPos[symbol], m_unitLimit));
					for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
					{
						orderID_Vector_Map[symbol].push_back(*it);
					}
				}
				else
				{
					std::vector<std::string>v;
					v = m_strategy_ptr->sellshort( Bar->low, std::min(m_strategy_ptr->getpos(symbol) - m_supposedPos[symbol], m_unitLimit));
					orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
				}
				m_orderIDvmtx.unlock();
			}
		}
		else if (m_strategy_ptr->getpos(symbol) > 0)//现在有多头
		{
			if (m_supposedPos[symbol] > m_strategy_ptr->getpos(symbol))//加多
			{
				m_orderIDvmtx.lock();
				if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
				{
					orderID_Vector_Map[symbol].clear();
					std::vector<std::string>v;
					v = m_strategy_ptr->buy( Bar->high, std::min(m_supposedPos[symbol] - m_strategy_ptr->getpos(symbol), m_unitLimit));
					for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
					{
						orderID_Vector_Map[symbol].push_back(*it);
					}
				}
				else
				{
					std::vector<std::string>v;
					v = m_strategy_ptr->buy(Bar->high, std::min(m_supposedPos[symbol] - m_strategy_ptr->getpos(symbol), m_unitLimit));
					orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
				}
				m_orderIDvmtx.unlock();
			}
			else if (m_supposedPos[symbol] < m_strategy_ptr->getpos(symbol))//减仓或翻空
			{
				if (m_supposedPos[symbol] >= 0)//减仓
				{
					m_orderIDvmtx.lock();
					if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
					{
						orderID_Vector_Map[symbol].clear();
						std::vector<std::string>v;
						v = m_strategy_ptr->sell(Bar->low, std::min(m_strategy_ptr->getpos(symbol) - m_supposedPos[symbol], m_unitLimit));
						for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
						{
							orderID_Vector_Map[symbol].push_back(*it);
						}
					}
					else
					{
						std::vector<std::string>v;
						v = m_strategy_ptr->sell(Bar->low, std::min(m_strategy_ptr->getpos(symbol) - m_supposedPos[symbol], m_unitLimit));
						orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
					}
					m_orderIDvmtx.unlock();
				}
				else//全平掉等着翻空
				{
					m_orderIDvmtx.lock();
					if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
					{
						orderID_Vector_Map[symbol].clear();
						std::vector<std::string>v;
						v = m_strategy_ptr->sell(Bar->low, std::min(m_strategy_ptr->getpos(symbol), m_unitLimit));
						for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
						{
							orderID_Vector_Map[symbol].push_back(*it);
						}
					}
					else
					{
						std::vector<std::string>v;
						v = m_strategy_ptr->sell(Bar->low, std::min(m_strategy_ptr->getpos(symbol), m_unitLimit));
						orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
					}
					m_orderIDvmtx.unlock();
				}
			}
		}
		else if (m_strategy_ptr->getpos(symbol) < 0)//有空头
		{
			if (m_supposedPos[symbol] > m_strategy_ptr->getpos(symbol))//减仓或者翻多
			{
				if (m_supposedPos[symbol] <= 0)//减仓
				{
					m_orderIDvmtx.lock();
					if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
					{
						orderID_Vector_Map[symbol].clear();
						std::vector<std::string>v;
						v = m_strategy_ptr->buycover(Bar->high, std::min(m_supposedPos[symbol] - m_strategy_ptr->getpos(symbol), m_unitLimit));
						for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
						{
							orderID_Vector_Map[symbol].push_back(*it);
						}
					}
					else
					{
						std::vector<std::string>v;
						v = m_strategy_ptr->buycover(Bar->high, std::min(m_supposedPos[symbol] - m_strategy_ptr->getpos(symbol), m_unitLimit));
						orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
					}
					m_orderIDvmtx.unlock();
				}
				else//全平掉等着翻多
				{
					m_orderIDvmtx.lock();
					if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
					{
						orderID_Vector_Map[symbol].clear();
						std::vector<std::string>v;
						v = m_strategy_ptr->buycover(Bar->high, std::min(std::abs(m_strategy_ptr->getpos(symbol)), m_unitLimit));
						for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
						{
							orderID_Vector_Map[symbol].push_back(*it);
						}
					}
					else
					{
						std::vector<std::string>v;
						v = m_strategy_ptr->buycover( Bar->high, std::min(std::abs(m_strategy_ptr->getpos(symbol)), m_unitLimit));
						orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
					}
					m_orderIDvmtx.unlock();
				}
			}
			else if (m_supposedPos[symbol] < m_strategy_ptr->getpos(symbol))//加空
			{
				m_orderIDvmtx.lock();
				if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
				{
					orderID_Vector_Map[symbol].clear();
					std::vector<std::string>v;
					v = m_strategy_ptr->sellshort(Bar->low, std::min(m_strategy_ptr->getpos(symbol) - m_supposedPos[symbol], m_unitLimit));
					for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
					{
						orderID_Vector_Map[symbol].push_back(*it);
					}
				}
				else
				{
					std::vector<std::string>v;
					v = m_strategy_ptr->sellshort(Bar->low, std::min(m_strategy_ptr->getpos(symbol) - m_supposedPos[symbol], m_unitLimit));
					orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
				}
				m_orderIDvmtx.unlock();
			}
		}
	}
	else//回测
	{
		if (m_strategy_ptr->getpos(symbol) == 0)//如果空仓，可以直接开
		{
			if (m_supposedPos[symbol] > m_strategy_ptr->getpos(symbol))//开多
			{
				m_orderIDvmtx.lock();
				if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
				{
					orderID_Vector_Map[symbol].clear();
					std::vector<std::string>v;
					v = m_strategy_ptr->buy( Bar->high + 99, std::min(m_supposedPos[symbol] - m_strategy_ptr->getpos(symbol), m_unitLimit));
					for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
					{
						orderID_Vector_Map[symbol].push_back(*it);
					}
				}
				else
				{
					std::vector<std::string>v;
					v = m_strategy_ptr->buy(Bar->high + 99, std::min(m_supposedPos[symbol] - m_strategy_ptr->getpos(symbol), m_unitLimit));
					orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
				}
				m_orderIDvmtx.unlock();
			}
			else if (m_supposedPos[symbol] < m_strategy_ptr->getpos(symbol))//开空
			{
				m_orderIDvmtx.lock();
				if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
				{
					orderID_Vector_Map[symbol].clear();
					std::vector<std::string>v;
					v = m_strategy_ptr->sellshort(Bar->low - 99, std::min(m_strategy_ptr->getpos(symbol) - m_supposedPos[symbol], m_unitLimit));
					for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
					{
						orderID_Vector_Map[symbol].push_back(*it);
					}
				}
				else
				{
					std::vector<std::string>v;
					v = m_strategy_ptr->sellshort(Bar->low - 99, std::min(m_strategy_ptr->getpos(symbol) - m_supposedPos[symbol], m_unitLimit));
					orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
				}
				m_orderIDvmtx.unlock();
			}
		}
		else if (m_strategy_ptr->getpos(symbol) > 0)//现在有多头
		{
			if (m_supposedPos[symbol] > m_strategy_ptr->getpos(symbol))//加多
			{
				m_orderIDvmtx.lock();
				if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
				{
					orderID_Vector_Map[symbol].clear();
					std::vector<std::string>v;
					v = m_strategy_ptr->buy( Bar->high + 99, std::min(m_supposedPos[symbol] - m_strategy_ptr->getpos(symbol), m_unitLimit));
					for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
					{
						orderID_Vector_Map[symbol].push_back(*it);
					}
				}
				else
				{
					std::vector<std::string>v;
					v = m_strategy_ptr->buy( Bar->high + 99, std::min(m_supposedPos[symbol] - m_strategy_ptr->getpos(symbol), m_unitLimit));
					orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
				}
				m_orderIDvmtx.unlock();
			}
			else if (m_supposedPos[symbol] < m_strategy_ptr->getpos(symbol))//减仓或翻空
			{
				if (m_supposedPos[symbol] >= 0)//减仓
				{
					m_orderIDvmtx.lock();
					if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
					{
						orderID_Vector_Map[symbol].clear();
						std::vector<std::string>v;
						v = m_strategy_ptr->sell( Bar->low - 99, std::min(m_strategy_ptr->getpos(symbol) - m_supposedPos[symbol], m_unitLimit));
						for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
						{
							orderID_Vector_Map[symbol].push_back(*it);
						}
					}
					else
					{
						std::vector<std::string>v;
						v = m_strategy_ptr->sell( Bar->low - 99, std::min(m_strategy_ptr->getpos(symbol) - m_supposedPos[symbol], m_unitLimit));
						orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
					}
					m_orderIDvmtx.unlock();
				}
				else//全平掉等着翻空
				{
					m_orderIDvmtx.lock();
					if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
					{
						orderID_Vector_Map[symbol].clear();
						std::vector<std::string>v;
						v = m_strategy_ptr->sell( Bar->low - 99, std::min(m_strategy_ptr->getpos(symbol), m_unitLimit));
						for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
						{
							orderID_Vector_Map[symbol].push_back(*it);
						}
					}
					else
					{
						std::vector<std::string>v;
						v = m_strategy_ptr->sell(Bar->low - 99, std::min(m_strategy_ptr->getpos(symbol), m_unitLimit));
						orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
					}
					m_orderIDvmtx.unlock();
				}
			}
		}
		else if (m_strategy_ptr->getpos(symbol) < 0)//有空头
		{
			if (m_supposedPos[symbol] > m_strategy_ptr->getpos(symbol))//减仓或者翻多
			{
				if (m_supposedPos[symbol] <= 0)//减仓
				{
					m_orderIDvmtx.lock();
					if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
					{
						orderID_Vector_Map[symbol].clear();
						std::vector<std::string>v;
						v = m_strategy_ptr->buycover( Bar->high + 99, std::min(m_supposedPos[symbol] - m_strategy_ptr->getpos(symbol), m_unitLimit));
						for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
						{
							orderID_Vector_Map[symbol].push_back(*it);
						}
					}
					else
					{
						std::vector<std::string>v;
						v = m_strategy_ptr->buycover( Bar->high + 99, std::min(m_supposedPos[symbol] - m_strategy_ptr->getpos(symbol), m_unitLimit));
						orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
					}
					m_orderIDvmtx.unlock();
				}
				else//全平掉等着翻多
				{
					m_orderIDvmtx.lock();
					if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
					{
						orderID_Vector_Map[symbol].clear();
						std::vector<std::string>v;
						v = m_strategy_ptr->buycover( Bar->high + 99, std::min(std::abs(m_strategy_ptr->getpos(symbol)), m_unitLimit));
						for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
						{
							orderID_Vector_Map[symbol].push_back(*it);
						}
					}
					else
					{
						std::vector<std::string>v;
						v = m_strategy_ptr->buycover( Bar->high + 99, std::min(std::abs(m_strategy_ptr->getpos(symbol)), m_unitLimit));
						orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
					}
					m_orderIDvmtx.unlock();
				}
			}
			else if (m_supposedPos[symbol] < m_strategy_ptr->getpos(symbol))//加空
			{
				m_orderIDvmtx.lock();
				if (orderID_Vector_Map.find(symbol) != orderID_Vector_Map.end())
				{
					orderID_Vector_Map[symbol].clear();
					std::vector<std::string>v;
					v = m_strategy_ptr->sellshort( Bar->low - 99, std::min(m_strategy_ptr->getpos(symbol) - m_supposedPos[symbol], m_unitLimit));
					for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
					{
						orderID_Vector_Map[symbol].push_back(*it);
					}
				}
				else
				{
					std::vector<std::string>v;
					v = m_strategy_ptr->sellshort( Bar->low - 99, std::min(m_strategy_ptr->getpos(symbol) - m_supposedPos[symbol], m_unitLimit));
					orderID_Vector_Map.insert(std::pair<std::string, std::vector<std::string>>(symbol, v));
				}
				m_orderIDvmtx.unlock();
			}
		}
	}
	m_supposedPosmtx.unlock();
}

void algorithmOrder::set_supposedPos(std::string symbol, double volume)
{
	m_supposedPosmtx.lock();
	m_supposedPos[symbol] = volume;
	m_supposedPosmtx.unlock();
}

void algorithmOrder::setStop_tralingLose(const BarData *bar, double tralingPercent, std::string longshortCondition)
{
	stopMode = trailingStop;
	if (longshortCondition == "long")
	{
		longTrailingPercent = tralingPercent;
	}
	else if (longshortCondition == "short")
	{
		shortTrailingPercent = tralingPercent;
	}
}

void algorithmOrder::setStop_time(const BarData *bar, int seconds)
{
	stopMode = timeStop;
	waitCountLimit = seconds;
	waitCount = 0;
}

void algorithmOrder::setStop_timeandTrailing(const BarData *bar, int seconds, double tralingpercent, std::string longshortCondition, double lastopenprice)
{
	stopMode = trailingandtimeStop;
	waitCountLimit = seconds;
	waitCount = 0;
	lastOpenPrice = lastopenprice;
	if (longshortCondition == "long")
	{
		longTrailingPercent = tralingpercent;
	}
	else if (longshortCondition == "short")
	{
		shortTrailingPercent = tralingpercent;
	}
}

bool algorithmOrder::checkStop(const BarData *bar)
{
	if (stopMode == timeStop)
	{
		waitCount++;
		if (waitCount > waitCountLimit)
		{
			set_supposedPos(bar->symbol, 0);
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (stopMode == trailingStop)
	{
		if (m_strategy_ptr->getpos(bar->symbol) > 0)
		{
			intraTradeHigh = std::max(intraTradeHigh, bar->high);
			intraTradeLow = bar->close;

			double longStop = intraTradeHigh*(100 - longTrailingPercent) / 100;
			if (bar->close < longStop)
			{
				set_supposedPos(bar->symbol, 0);
				return true;
			}
			else
			{
				return false;
			}
		}
		if (m_strategy_ptr->getpos(bar->symbol) < 0)
		{
			intraTradeLow = std::min(intraTradeLow, bar->low);
			intraTradeHigh = bar->close;

			double shortStop = intraTradeLow*(100 + shortTrailingPercent) / 100;//4145*101.5/100
			if (bar->close > shortStop)
			{
				set_supposedPos(bar->symbol, 0);
				return true;
			}
			else
			{
				return false;
			}
		}
		if (m_strategy_ptr->getpos(bar->symbol) == 0)
		{
			intraTradeLow = 9999999999999;
			intraTradeHigh = 0;
		}
	}
	else if (stopMode == trailingandtimeStop)
	{
		if (m_strategy_ptr->getpos(bar->symbol) > 0)
		{
			intraTradeHigh = std::max(intraTradeHigh, bar->high);
			intraTradeLow = bar->close;
			double longStop = intraTradeHigh*(100 - longTrailingPercent) / 100;
			if (bar->close < longStop)
			{
				set_supposedPos(bar->symbol, 0);
				return true;
			}
			else
			{
				waitCount++;
				if (waitCount > waitCountLimit)
				{
					set_supposedPos(bar->symbol, 0);
					return true;
				}
				else
				{
					return false;
				}
			}
		}
		if (m_strategy_ptr->getpos(bar->symbol) < 0)
		{
			intraTradeLow = std::min(intraTradeLow, bar->low);
			intraTradeHigh = bar->close;

			double shortStop = intraTradeLow*(100 + shortTrailingPercent) / 100;
			if (bar->close > shortStop)
			{
				set_supposedPos(bar->symbol, 0);
				return true;
			}
			else
			{
				waitCount++;
				if (waitCount > waitCountLimit)
				{
					set_supposedPos(bar->symbol, 0);
					return true;
				}
				else
				{
					return false;
				}
			}
		}
	}
	else if (stopMode == winAndLoseStop)
	{
		if (m_strategy_ptr->getpos(bar->symbol) > 0)
		{
			intraTradeHigh = std::max(intraTradeHigh, bar->high);
			intraTradeLow = bar->close;
			double longStop = intraTradeHigh*(100 - longTrailingPercent) / 100;
			double longWinStop = lastOpenPrice*(100 + longWinTrailingPercent) / 100;
			if (bar->close < longStop || bar->close >longWinStop)
			{
				set_supposedPos(bar->symbol, 0);
				return true;
			}
			else
			{
				return false;
			}
		}
		if (m_strategy_ptr->getpos(bar->symbol) < 0)
		{
			intraTradeLow = std::min(intraTradeLow, bar->low);
			intraTradeHigh = bar->close;
			double shortWinStop = lastOpenPrice *(100 - shortWinTrailingPercent) / 100;
			double shortStop = intraTradeLow*(100 + shortTrailingPercent) / 100;
			if (bar->close > shortStop || bar->close < shortWinStop)
			{
				set_supposedPos(bar->symbol, 0);
				return true;
			}
			else
			{
				return false;
			}
		}
	}
}