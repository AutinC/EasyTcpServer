#ifndef _EasyEpollServer_hpp_
#define _EasyEpollServer_hpp_

#include"EasyTcpServer.hpp"
#include"CELLEpollServer.hpp"
#include"CELLEpoll.hpp"

class EasyEpollServer : public EasyTcpServer
{
public:
	void Start(int nCELLServer)
	{
		EasyTcpServer::Start<CELLEpollServer>(nCELLServer);
	}
protected:
	//处理网络消息
	void OnRun(CELLThread* pThread)
	{
		CELLEpoll ep;
		ep.create(1);
		ep.ctl(EPOLL_CTL_ADD, sockfd(), EPOLLIN);
		while (pThread->isRun())
		{
			time4msg();
			//---
			int ret = ep.wait(1);
			if (ret < 0)
			{
				CELLLog_Error("EasyEpollServer.OnRun ep.wait exit.");
				pThread->Exit();
				break;
			}
			//---
			auto events = ep.events();
			for(int i = 0; i < ret; i++)
			{
				//当服务端socket发生事件时，表示有新客户端连接
				if(events[i].data.fd == sockfd())
				{
					if(events[i].events & EPOLLIN)
					{
						Accept();
					}
				}
			}
		}
	}
};

#endif // !_EasyEpollServer_hpp_
