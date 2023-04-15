//#include"EasySelectServer.hpp"
//#include"EasyEpollServer.hpp"
#include"EasyIOCPServer.hpp"
#include"CELLMsgStream.hpp"
#include"CELLConfig.hpp"

class MyServer : public EasyIOCPServer
{
public:
	MyServer()
	{
		_bSendBack = CELLConfig::Instance().hasKey("-sendback");
		_bSendFull = CELLConfig::Instance().hasKey("-sendfull");
		_bCheckMsgID = CELLConfig::Instance().hasKey("-checkMsgID");
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetJoin(CELLClient* pClient)
	{
		EasyTcpServer::OnNetJoin(pClient);
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetLeave(CELLClient* pClient)
	{
		EasyTcpServer::OnNetLeave(pClient);
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetMsg(CELLServer* pServer, CELLClient* pClient, netmsg_DataHeader* header)
	{
		EasyTcpServer::OnNetMsg(pServer, pClient, header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			pClient->resetDTHeart();
			netmsg_Login* login = (netmsg_Login*)header;
			//检查消息ID
			if (_bCheckMsgID)
			{
				if (login->msgID != pClient->nRecvMsgID)
				{//当前消息ID和本地收消息次数不匹配
					CELLLog_Error("OnNetMsg socket<%d> msgID<%d> _nRecvMsgID<%d> %d", pClient->sockfd(), login->msgID, pClient->nRecvMsgID, login->msgID - pClient->nRecvMsgID);
				}
				++pClient->nRecvMsgID;
			}
			//登录逻辑
			//......
			//回应消息
			if (_bSendBack)
			{
				netmsg_LoginR ret;
				ret.msgID = pClient->nSendMsgID;
				if (SOCKET_ERROR == pClient->SendData(&ret))
				{
					//发送缓冲区满了，消息没发出去,目前直接抛弃了
					//客户端消息太多，需要考虑应对策略
					//正常连接，业务客户端不会有这么多消息
					//模拟并发测试时是否发送频率过高
					if (_bSendFull)
					{
						CELLLog_Warring("<Socket=%d> Send Full", pClient->sockfd());
					}
				}
				else {
					++pClient->nSendMsgID;
				}
			}

			//CELLLog_Info("recv <Socket=%d> msgType：CMD_LOGIN, dataLen：%d,userName=%s PassWord=%s", cSock, login->dataLength, login->userName, login->PassWord);
		}//接收 消息---处理 发送   生产者 数据缓冲区  消费者 
		break;
		case CMD_LOGOUT:
		{
			pClient->resetDTHeart();
			CELLReadStream r(header);
			//读取消息长度
			r.ReadInt16();
			//读取消息命令
			r.getNetCmd();
			auto n1 = r.ReadInt8();
			auto n2 = r.ReadInt16();
			auto n3 = r.ReadInt32();
			auto n4 = r.ReadFloat();
			auto n5 = r.ReadDouble();
			uint32_t n = 0;
			r.onlyRead(n);
			char name[32] = {};
			auto n6 = r.ReadArray(name, 32);
			char pw[32] = {};
			auto n7 = r.ReadArray(pw, 32);
			int ata[10] = {};
			auto n8 = r.ReadArray(ata, 10);
			///
			CELLWriteStream s(128);
			s.setNetCmd(CMD_LOGOUT_RESULT);
			s.WriteInt8(n1);
			s.WriteInt16(n2);
			s.WriteInt32(n3);
			s.WriteFloat(n4);
			s.WriteDouble(n5);
			s.WriteArray(name, n6);
			s.WriteArray(pw, n7);
			s.WriteArray(ata, n8);
			s.finsh();
			pClient->SendData(s.data(), s.length());
		}
		break;
		case CMD_C2S_HEART:
		{
			pClient->resetDTHeart();
			netmsg_s2c_Heart ret;
			pClient->SendData(&ret);
		}
		default:
		{
			CELLLog_Info("recv <socket=%d> undefine msgType,dataLen：%d", pClient->sockfd(), header->dataLength);
		}
		break;
		}
	}
private:
	//自定义标志 收到消息后将返回应答消息
	bool _bSendBack;
	//自定义标志 是否提示：发送缓冲区已写满
	bool _bSendFull;
	//是否检查接收到的消息ID是否连续
	bool _bCheckMsgID;
};

const char* argToStr(int argc, char* args[], int index, const char* def, const char* argName)
{
	if (index >= argc)
	{
		CELLLog_Error("argToStr, index=%d|argc=%d|argName=%s", index, argc, argName);
	}else {
		def = args[index];
	}
	CELLLog_Info("%s=%s",argName, def);
	return def;
}

int argToInt(int argc, char* args[], int index, int def, const char* argName)
{
	if (index >= argc)
	{
		CELLLog_Error("argToStr, index=%d|argc=%d|argName=%s", index, argc, argName);
	}
	else {
		def = atoi(args[index]);
	}
	CELLLog_Info("%s=%d", argName, def);
	return def;
}

int main(int argc,char* args[])
{
	//设置运行日志名称
	CELLLog::Instance().setLogPath("serverLog", "w", false);
	CELLConfig::Instance().Init(argc, args);

	const char* strIP = CELLConfig::Instance().getStr("strIP", "any");
	uint16_t nPort = CELLConfig::Instance().getInt("nPort", 4567);
	int nThread = CELLConfig::Instance().getInt("nThread", 1);

	if (CELLConfig::Instance().hasKey("-p"))
	{
		CELLLog_Info("hasKey -p");
	}

	if (strcmp(strIP, "any") == 0)
	{
		strIP = nullptr;
	}

	MyServer server;
	server.InitSocket();
	server.Bind(strIP, nPort);
	server.Listen(64);
	server.Start(nThread);

	//在主线程中等待用户输入命令
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			server.Close();
			break;
		}
		else {
			CELLLog_Info("undefine cmd");
		}
	}

	CELLLog_Info("exit.");
	return 0;
}
