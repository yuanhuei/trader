#ifndef JSTOOLS_H
#define JSTOOLS_H
#include<string>
#include<sstream>
#include<chrono>
#include<vector>
#include<regex>
#pragma warning( disable : 4996)
class Utils
{
public:
	static std::string Utils::getCurrentSystemTime()
	{
		auto tt = std::chrono::system_clock::to_time_t
			(std::chrono::system_clock::now());
		struct tm* ptm = localtime(&tt);
		char date[60] = { 0 };
		sprintf(date, "%d-%02d-%02d      %02d:%02d:%02d",
			(int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
			(int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
		return std::string(date);
	}

	static std::string Utils::getCurrentSystemDate()
	{
		auto tt = std::chrono::system_clock::to_time_t
			(std::chrono::system_clock::now());
		struct tm* ptm = localtime(&tt);
		char date[60] = { 0 };
		sprintf(date, "%d-%02d-%02d ", (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday);
		return std::string(date);
	}

	static std::string Utils::booltostring(bool var)
	{
		std::string varstring;
		if (var == true)
		{
			varstring = "true";
		}
		else if (var == false)
		{
			varstring = "false";
		}
		return varstring;
	}

	static bool Utils::stringtobool(std::string string)
	{
		bool result;
		if (string == "true")
		{
			result = true;
		}
		else if (string == "false")
		{
			result = false;
		}
		return result;
	}

	static std::string Utils::doubletostring(double var)
	{
		std::stringstream ss;
		std::string varstring;
		ss << var;
		ss >> varstring;
		return varstring;
	}

	static const time_t Utils::getsystemunixdatetime(std::string time, std::string type)
	{
		std::vector<std::string>v;
		char* pch = strtok(const_cast<char*>(time.c_str()), ":.");
		while (pch != NULL)
		{
			v.push_back(pch);
			pch = strtok(NULL, ":.");
		}
		if (v.size() == 4)
		{
			auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			struct tm *l = localtime(&tt);
			l->tm_hour = atoi(v[0].c_str());
			l->tm_min = atoi(v[1].c_str());
			l->tm_sec = atoi(v[2].c_str());
			time_t ft = mktime(l);
			if (type == "s")
			{
				return (ft);
			}
			else if (type == "ms")
			{
				return (ft * 1000 + atoi(v[3].c_str()) * 100);
			}
		}
		return 0;
	}

	static const time_t Utils::timetounixtimestamp(int hour, int minute, int seconds)
	{
		auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		struct tm *l = localtime(&tt);
		l->tm_hour = hour;
		l->tm_min = minute;
		l->tm_sec = seconds;
		time_t ft = mktime(l);
		return ft;
	}

	static	std::vector<std::string> Utils::split(std::string str, std::string pattern)
	{
		std::string::size_type pos;
		std::vector<std::string> result;
		str += pattern;
		std::string::size_type size = str.size();

		for (std::string::size_type i = 0; i < size; i++)
		{
			pos = str.find(pattern, i);
			if (pos < size)
			{
				std::string s = str.substr(i, pos - i);
				result.push_back(s);
				i = pos + pattern.size() - 1;
			}
		}
		return result;
	}

	static std::string Utils::regMySymbol(std::string symbol)
	{
		std::string s = symbol;
		std::regex regex("\\d");
		std::string trans_symbol = std::regex_replace(s, regex, "");
		return trans_symbol;
	}
	static bool isnum(std::string s)
	{
		std::stringstream sin(s);
		double t;
		char p;
		if (!(sin >> t))
			/*解释：
			sin>>t表示把sin转换成double的变量（其实对于int和float型的都会接收），如果转换成功，则值为非0，如果转换不成功就返回为0
			*/
			return false;
		if (sin >> p)
			/*解释：此部分用于检测错误输入中，数字加字符串的输入形式（例如：34.f），在上面的的部分（sin>>t）已经接收并转换了输入的数字部分，在stringstream中相应也会把那一部分给清除，如果此时传入字符串是数字加字符串的输入形式，则此部分可以识别并接收字符部分，例如上面所说的，接收的是.f这部分，所以条件成立，返回false;如果剩下的部分不是字符，那么则sin>>p就为0,则进行到下一步else里面
			*/
			return false;
		else
			return true;
	}

	static int Utils::getWeedDay(std::string date)
	{
		if (date.find("-") != std::string::npos)
		{
			std::vector<std::string>dateVec = Utils::split(date, "-");
			int y = 0, c = 0, m = 0, d = 0;
			int centrary = atoi(dateVec[0].substr(0, 2).c_str()) + 1;
			int year = atoi(dateVec[0].substr(2, 2).c_str());
			int month = atoi(dateVec[1].c_str());
			int day = atoi(dateVec[2].c_str());
			if (month == 1 || month == 2)
			{
				c = (year - 1) / 100;
				y = (year - 1) % 100;
				m = month + 12;
				d = day;
			}
			else
			{
				c = year / 100;
				y = year % 100;
				m = month;
				d = day;
			}

			int w = y + y / 4 + c / 4 - 2 * c + 26 * (m + 1) / 10 + d - 1;
			w = w >= 0 ? (w % 7) : (w % 7 + 7);
			if (w == 0)
			{
				w = 7;
			}
			return w;
		}
		else
		{
			std::string date = getCurrentSystemDate();
			std::vector<std::string>dateVec = Utils::split(date, "-");
			int y = 0, c = 0, m = 0, d = 0;
			int centrary = atoi(dateVec[0].substr(0, 2).c_str()) + 1;
			int year = atoi(dateVec[0].substr(2, 2).c_str());
			int month = atoi(dateVec[1].c_str());
			int day = atoi(dateVec[2].c_str());
			if (month == 1 || month == 2)
			{
				c = (year - 1) / 100;
				y = (year - 1) % 100;
				m = month + 12;
				d = day;
			}
			else
			{
				c = year / 100;
				y = year % 100;
				m = month;
				d = day;
			}

			int w = y + y / 4 + c / 4 - 2 * c + 26 * (m + 1) / 10 + d - 1;
			w = w >= 0 ? (w % 7) : (w % 7 + 7);
			if (w == 0)
			{
				w = 7;
			}
			return w;
		}
	}
};
#endif