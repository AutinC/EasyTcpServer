#include "EasyTcpServer.hpp"
#include<thread>

class MyServer : public EasyTcpServer
{
public:

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
			//send recv 
			netmsg_Login* login = (netmsg_Login*)header;
			//CELLLog::Info("recv <Socket=%d> msgType：CMD_LOGIN, dataLen：%d,userName=%s PassWord=%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//忽略判断用户密码是否正确的过程
			netmsg_LoginR ret;
			if(SOCKET_ERROR == pClient->SendData(&ret))
			{
				//发送缓冲区满了，消息没发出去
				CELLLog::Info("<Socket=%d> Send Full\n", pClient->sockfd());
			}
			//netmsg_LoginR* ret = new netmsg_LoginR();
			//pServer->addSendTask(pClient, ret);
		}//接收 消息---处理 发送   生产者 数据缓冲区  消费者 
		break;
		case CMD_LOGOUT:
		{
			netmsg_Logout* logout = (netmsg_Logout*)header;
			//CELLLog::Info("recv <Socket=%d> msgType：CMD_LOGOUT, dataLen：%d,userName=%s \n", cSock, logout->dataLength, logout->userName);
			//忽略判断用户密码是否正确的过程
			//netmsg_LogoutR ret;
			//SendData(cSock, &ret);
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
			CELLLog::Info("recv <socket=%d> undefine msgType,dataLen：%d\n", pClient->sockfd(), header->dataLength);
		}
		break;
		}
	}
private:

};

int main()
{
	CELLLog::Instance().setLogPath("serverLog.txt","w");
	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(64);
	server.Start(4);

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
			CELLLog::Info("undefine cmd\n");
		}
	}

	CELLLog::Info("exit.\n");
//#ifdef _WIN32
//	while (true)
//		Sleep(10);
//#endif
	return 0;
}
