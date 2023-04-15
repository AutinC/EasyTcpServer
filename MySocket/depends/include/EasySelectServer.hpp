#ifndef _EasySelectServer_hpp_
#define _EasySelectServer_hpp_

#include"EasyTcpServer.hpp"
#include"CELLSelectServer.hpp"
#include"CELLFDSet.hpp"

class EasySelectServer : public EasyTcpServer
{
public:
	void Start(int nCELLServer)
	{
		EasyTcpServer::Start<CELLSelectServer>(nCELLServer);
	}
protected:
	//处理网络消息
	void OnRun(CELLThread* pThread)
	{
		//伯克利套接字 BSD socket
		//描述符（socket） 集合
		CELLFDSet fdRead;
		while (pThread->isRun())
		{
			time4msg();
			//清理集合
			fdRead.zero();
			//将描述符（socket）加入集合
			fdRead.add(sockfd());
			///nfds 是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
			///既是所有文件描述符最大值+1 在Windows中这个参数可以写0
			timeval t = { 0, 1};
			int ret = select(sockfd() + 1, fdRead.fdset(), 0, 0, &t); //
			if (ret < 0)
			{
				if(errno == EINTR)
				{
					CELLLog_Info("EasySelectServer select EINTR");
					continue;
				}
					
				CELLLog_PError("EasySelectServer.OnRun select.");
				pThread->Exit();
				break;
			}
			//判断描述符（socket）是否在集合中
			if (fdRead.has(sockfd()))
			{
				//fdRead.del(_sock);
				Accept();
			}
		}
	}
};

#endif // !_EasySelectServer_hpp_
