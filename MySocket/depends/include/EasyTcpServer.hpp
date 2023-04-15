#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#include"CELL.hpp"
#include"CELLClient.hpp"
#include"CELLServer.hpp"
#include"INetEvent.hpp"
#include"CELLNetWork.hpp"
#include"CELLConfig.hpp"
#include"CELLFDSet.hpp"

#include<thread>
#include<mutex>
#include<atomic>

class EasyTcpServer : public INetEvent
{
private:
	//
	CELLThread _thread;
	//消息处理对象，内部会创建线程
	std::vector<CELLServer*> _cellServers;
	//每秒消息计时
	CELLTimestamp _tTime;
	//
	SOCKET _sock;
protected:
	//客户端发送缓冲区大小
	int _nSendBuffSize;
	//客户端接收缓冲区大小
	int _nRecvBuffSize;
	//客户端连接上限
	int _nMaxClient;
	//SOCKET recv计数
	std::atomic_int _recvCount;
	//收到消息计数
	std::atomic_int _msgCount;
	//客户端计数
	std::atomic_int _clientCount;
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
		_recvCount = 0;
		_msgCount = 0;
		_clientCount = 0;
		_nSendBuffSize = CELLConfig::Instance().getInt("nSendBuffSize", SEND_BUFF_SZIE);
		_nRecvBuffSize = CELLConfig::Instance().getInt("nRecvBuffSize", RECV_BUFF_SZIE);
		_nMaxClient = CELLConfig::Instance().getInt("nMaxClient", FD_SETSIZE);
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}
	//初始化Socket
	SOCKET InitSocket()
	{
		CELLNetWork::Init();
		if (INVALID_SOCKET != _sock)
		{
			CELLLog_Warring("initSocket close old socket<%d>...", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			CELLLog_PError("create socket failed...");
		}
		else {
			CELLNetWork::make_reuseaddr(_sock);
			CELLLog_Info("create socket<%d> success...", (int)_sock);
		}
		return _sock;
	}

	//绑定IP和端口号
	int Bind(const char* ip, unsigned short port)
	{
		//if (INVALID_SOCKET == _sock)
		//{
		//	InitSocket();
		//}
		// 2 bind 绑定用于接受客户端连接的网络端口
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);//host to net unsigned short

#ifdef _WIN32
		if (ip){
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
#else
		if (ip) {
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret)
		{
			CELLLog_PError("bind port<%d> failed...", port);
		}
		else {
			CELLLog_Info("bind port<%d> success...", port);
		}
		return ret;
	}

	//监听端口号
	int Listen(int n)
	{
		// 3 listen 监听网络端口
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			CELLLog_PError("listen socket<%d> failed...",_sock);
		}
		else {
			CELLLog_Info("listen port<%d> success...", _sock);
		}
		return ret;
	}

	//接受客户端连接
	SOCKET Accept()
	{
		// 4 accept 等待接受客户端连接
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t *)&nAddrLen);
#endif
		if (INVALID_SOCKET == cSock)
		{
			CELLLog_PError("accept INVALID_SOCKET...");
		}
		else
		{	
			if (_clientCount < _nMaxClient)
			{
				CELLNetWork::make_reuseaddr(cSock);
				//将新客户端分配给客户数量最少的cellServer
				addClientToCELLServer(new CELLClient(cSock, _nSendBuffSize, _nRecvBuffSize));
				//获取IP地址 inet_ntoa(clientAddr.sin_addr)
			}
			else {
				//获取IP地址 inet_ntoa(clientAddr.sin_addr)
				CELLNetWork::destorySocket(cSock);
				CELLLog_Warring("Accept to nMaxClient");
			}
		}
		return cSock;
	}
	
	void addClientToCELLServer(CELLClient* pClient)
	{
		//查找客户数量最少的CELLServer消息处理对象
		auto pMinServer = _cellServers[0];
		for(auto pServer : _cellServers)
		{
			if (pMinServer->getClientCount() > pServer->getClientCount())
			{
				pMinServer = pServer;
			}
		}
		pMinServer->addClient(pClient);
	}

	template<class ServerT>
	void Start(int nCELLServer)
	{
		for (int n = 0; n < nCELLServer; n++)
		{
			auto ser = new ServerT();
			ser->setId(n + 1);
			_cellServers.push_back(ser);
			//注册网络事件接受对象
			ser->setEventObj(this);
			//启动消息处理线程
			ser->Start();
		}
		_thread.Start(nullptr,
			[this](CELLThread* pThread) {
				OnRun(pThread);
			});
	}
	//关闭Socket
	void Close()
	{
		CELLLog_Info("EasyTcpServer.Close begin");
		_thread.Close();
		if (_sock != INVALID_SOCKET)
		{
			for (auto s : _cellServers)
			{
				delete s;
			}
			_cellServers.clear();
			//关闭套节字socket
			CELLNetWork::destorySocket(_sock);
			_sock = INVALID_SOCKET;
		}
		CELLLog_Info("EasyTcpServer.Close end");
	}

	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetJoin(CELLClient* pClient)
	{
		_clientCount++;
		//CELLLog_Info("client<%d> join", pClient->sockfd());
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetLeave(CELLClient* pClient)
	{
		_clientCount--;
		//CELLLog_Info("client<%d> leave", pClient->sockfd());
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetMsg(CELLServer* pServer, CELLClient* pClient, netmsg_DataHeader* header)
	{
		_msgCount++;
	}

	virtual void OnNetRecv(CELLClient* pClient)
	{
		_recvCount++;
		//CELLLog_Info("client<%d> leave", pClient->sockfd());
	}
protected:
	//处理网络消息
	virtual void OnRun(CELLThread* pThread) = 0;

	//计算并输出每秒收到的网络消息
	void time4msg()
	{
		auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{
			CELLLog_Info("thread<%d>,time<%lf>,socket<%d>,clients<%d>,recv<%d>,msg<%d>", (int)_cellServers.size(), t1, _sock, (int)_clientCount, (int)_recvCount, (int)_msgCount);
			_recvCount = 0;
			_msgCount = 0;
			_tTime.update();
		}
	}

	SOCKET sockfd()
	{
		return _sock;
	}
};

#endif // !_EasyTcpServer_hpp_
