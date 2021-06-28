#include<utility.h>

//#include<qstring.h>

QString str2qstr_new(std::string str)
{
	return QString::fromLocal8Bit(str.c_str());
}