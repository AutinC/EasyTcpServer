#ifndef _CELL_CONFIG_HPP_
#define _CELL_CONFIG_HPP_
/*
	专门用于读取配置数据
	目前我们的配置参数主要来源于main函数的args传入
*/
#include<string>
#include<map>
#include"CELLLog.hpp"

class CELLConfig
{
private:
	CELLConfig()
	{

	}

	~CELLConfig()
	{

	}
public:
	static CELLConfig& Instance()
	{
		static  CELLConfig obj;
		return obj;
	}

	void Init(int argc, char* args[])
	{
		_exePath = args[0];
		for (int n = 1; n < argc; n++)
		{
			madeCmd(args[n]);
		}
	}

	void madeCmd(char* cmd)
	{
		//cmd值:strIP=127.0.0.1
		char* val = strchr(cmd, '=');
		if (val)
		{	//val值:=127.0.0.1
			*val = '\0';
			//cmd值:strIP\0
			//val值:\0127.0.0.1
			val++;
			//val值:127.0.0.1
			_kv[cmd] = val;
			CELLLog_Debug("madeCmd k<%s> v<%s>", cmd, val);
		} else {
			_kv[cmd] = "";
			CELLLog_Debug("madeCmd k<%s>", cmd);
		}
	}

	const char* getStr(const char* argName, const char* def)
	{
		auto itr = _kv.find(argName);
		if (itr == _kv.end())
		{
			CELLLog_Error("CELLConfig::getStr not find <%s>", argName);
		} else {
			def = itr->second.c_str();
		}
		CELLLog_Info("CELLConfig::getStr %s=%s", argName, def);
		return def;
	}

	int getInt(const char* argName, int def)
	{
		auto itr = _kv.find(argName);
		if (itr == _kv.end())
		{
			CELLLog_Error("CELLConfig::getStr not find <%s>", argName);
		}
		else {
			def = atoi(itr->second.c_str());
		}
		CELLLog_Info("CELLConfig::getInt %s=%d", argName, def);
		return def;
	}

	bool hasKey(const char* key)
	{
		auto itr = _kv.find(key);
		return itr != _kv.end();
	}

private:
	//当前程序的路径
	std::string _exePath;
	//存储传入的key-val型数据
	std::map<std::string, std::string> _kv;

};

#endif // !_CELL_CONFIG_HPP_
