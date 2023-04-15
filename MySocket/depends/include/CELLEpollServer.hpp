#ifndef _CELLEpollServer_HPP_
#define _CELLEpollServer_HPP_

#include"CELLServer.hpp"
#include"CELLEpoll.hpp"

//网络消息接收处理服务类
class CELLEpollServer:public CELLServer
{
public:
	CELLEpollServer()
	{
		_ep.create(10240);
	}

	~CELLEpollServer() noexcept
	{
		Close();
	}

	bool DoNetEvents()
	{
		for (auto iter : _clients)
		{	//需要写数据的客户端,才加入EPOLLOUT检测是否可写
			if (iter.second->needWrite())
			{
				_ep.ctl(EPOLL_CTL_MOD, iter.second, EPOLLIN|EPOLLOUT);
			}else{
				_ep.ctl(EPOLL_CTL_MOD, iter.second, EPOLLIN);
			}
		}
		//---
		int ret = _ep.wait(1);
		if (ret < 0)
		{
            CELLLog_Error("CELLEpollServer%d.OnRun.wait", _id);
			return false;
		}
		else if (ret == 0)
		{
			return true;
		}
		//---
		auto events = _ep.events();
		for(int i = 0; i < ret; i++)
		{
			CELLClient* pClient = (CELLClient*)events[i].data.ptr;
			//当服务端socket发生事件时，表示有新客户端连接
			if(pClient)
			{
				if(events[i].events & EPOLLIN)
				{
					if (SOCKET_ERROR == RecvData(pClient))
					{
						rmClient(pClient);
						continue;
					}
				}
				if(events[i].events & EPOLLOUT)
				{
					if (SOCKET_ERROR == pClient->SendDataReal())
					{
						rmClient(pClient);
					}
				}
			}
		}
		return true;
	}

	void rmClient(CELLClient* pClient)
	{
		auto iter = _clients.find(pClient->sockfd());
		if(iter != _clients.end())
		_clients.erase(iter);
		//
		OnClientLeave(pClient);
	}

	void OnClientJoin(CELLClient* pClient)
	{
		_ep.ctl(EPOLL_CTL_ADD, pClient, EPOLLIN);
	}
private:
	CELLEpoll _ep;
};

#endif // !_CELLEpollServer_HPP_
