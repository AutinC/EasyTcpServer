#ifndef _CELL_EPOLL_HPP_
#define _CELL_EPOLL_HPP_

#if __linux__
//----------
#include"CELL.hpp"
#include"CELLClient.hpp"
#include"CELLLog.hpp"
#include<sys/epoll.h>
#define EPOLL_ERROR            (-1)
//----------

class CELLEpoll
{
public:
	~CELLEpoll()
	{
		destory();
	}
	
	int create(int nMaxEvents)
	{
		if(_epfd > 0)
		{
			//Warring
			destory();
		}
		//linux 2.6.8 后size就没有作用了
		//由epoll动态管理，理论最大值为filemax
		//通过cat /proc/sys/fs/file-max来查询
		//ulimit -n
		_epfd = epoll_create(nMaxEvents);
		if(EPOLL_ERROR == _epfd)
		{
			CELLLog_PError("epoll_create");
			return _epfd;
		}
		_pEvents = new epoll_event[nMaxEvents];
		_nMaxEvents = nMaxEvents;
		return _epfd;
	}

	void destory()
	{
		if(_epfd > 0)
		{
			CELLNetWork::destorySocket(_epfd);
			_epfd = -1;
		}
			
		if(_pEvents)
		{
			delete[] _pEvents;
			_pEvents = nullptr;
		}
	}

	//向epoll对象注册需要管理、监听的Socket文件描述符
	int ctl(int op, SOCKET sockfd, uint32_t events)
	{
		epoll_event ev;
		//事件类型
		ev.events = events;
		//事件关联的socket描述符对象
		ev.data.fd = sockfd;
		//向epoll对象注册需要管理、监听的Socket文件描述符
		//并且说明关心的事件
		//返回0代表操作成功，返回负值代表失败 -1
		int ret = epoll_ctl(_epfd, op, sockfd, &ev);
		if(EPOLL_ERROR == ret)
		{
			CELLLog_PError("epoll_ctl1");
		}
		return ret;
	}

	//向epoll对象注册需要管理、监听的Socket文件描述符
	int ctl(int op, CELLClient* pClient, uint32_t events)
	{
		epoll_event ev;
		//事件类型
		ev.events = events;
		//事件关联的Client对象
		ev.data.ptr = pClient;
		//向epoll对象注册需要管理、监听的Socket文件描述符
		//并且说明关心的事件
		//返回0代表操作成功，返回负值代表失败 -1
		int ret = epoll_ctl(_epfd, op, pClient->sockfd(), &ev);
		if(EPOLL_ERROR == ret)
		{
			CELLLog_PError("epoll_ctl2");
		}
		return ret;
	}

	int wait(int timeout)
	{
		//epfd epoll对象的描述符
		//events 用于接收检测到的网络事件的数组
		//maxevents 接收数组的大小，能够接收的事件数量
		//timeout
		//		t=-1 直到有事件发生才返回
		//		t= 0 立即返回 std::map
		//		t> 0 如果没有事件那么等待t毫秒后返回。
		int ret = epoll_wait(_epfd, _pEvents, _nMaxEvents, timeout);
		if(EPOLL_ERROR == ret)
		{
			if(errno == EINTR)
			{
				CELLLog_Info("epoll_wait EINTR");
				return 0;
			}
			CELLLog_PError("epoll_wait");
		}
		return ret;
	}

	epoll_event* events()
	{
		return _pEvents;
	}
private:
	//用于接收检测到的网络事件的数组
	epoll_event* _pEvents = nullptr;
	//
	int _nMaxEvents = 1;
	//
	int _epfd = -1;
};

#endif //__linux__
#endif // !_CELL_EPOLL_HPP_
