#ifndef _EasyEpollClient_hpp_
#define _EasyEpollClient_hpp_

#include"EasyTcpClient.hpp"
#include"CELLEpoll.hpp"

class EasyEpollClient : public EasyTcpClient
{
public:
	virtual void OnInitSocket() {
		_ep.create(1);
		_ep.ctl(EPOLL_CTL_ADD, _pClient, EPOLLIN);
	};

	void Close()
	{
		_ep.destory();
		EasyTcpClient::Close();
	}

	//处理网络消息
	bool OnRun(int microseconds = 1)
	{
		if (isRun())
		{
			if (_pClient->needWrite())
			{
				_ep.ctl(EPOLL_CTL_MOD, _pClient, EPOLLIN|EPOLLOUT);
			}else{
				_ep.ctl(EPOLL_CTL_MOD, _pClient, EPOLLIN);
			}
			//---
			int ret = _ep.wait(microseconds);
			if (ret < 0)
			{
				CELLLog_Error("CELLEpollClient.OnRun.wait clientId<%d> sockfd<%d>", _pClient->id, (int)_pClient->sockfd());
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
						if (SOCKET_ERROR == RecvData())
						{
							CELLLog_Error("<socket=%d>OnRun.epoll RecvData exit", pClient->sockfd());
							Close();
							continue;
						}
					}
					if(events[i].events & EPOLLOUT)
					{
						if (SOCKET_ERROR == pClient->SendDataReal())
						{
							CELLLog_Error("<socket=%d>OnRun.epoll SendDataReal exit", pClient->sockfd());
							Close();
						}
					}
				}
			}
			return true;
		}
		return false;
	}
protected:
	CELLEpoll _ep;
};

#endif