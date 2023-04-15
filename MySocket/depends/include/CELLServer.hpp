#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include"CELL.hpp"
#include"INetEvent.hpp"
#include"CELLClient.hpp"
#include"CELLSemaphore.hpp"
#include"CELLFDSet.hpp"

#include<vector>
#include<map>

//网络消息接收处理服务类
class CELLServer
{
public:
	virtual ~CELLServer()
	{
		CELLLog_Info("CELLServer%d.~CELLServer exit begin", _id);
		Close();
		CELLLog_Info("CELLServer%d.~CELLServer exit end", _id);
	}

	void setId(int id)
	{
		_id = id;
		_taskServer.serverId = id;
	}

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	//关闭Socket
	void Close()
	{
		CELLLog_Info("CELLServer%d.Close begin", _id);
		_taskServer.Close();
		_thread.Close();
		CELLLog_Info("CELLServer%d.Close end", _id);
	}

	//处理网络消息
	void OnRun(CELLThread* pThread)
	{
		while (pThread->isRun())
		{
			if (!_clientsBuff.empty())
			{//从缓冲队列里取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
					pClient->serverId = _id;
					if (_pNetEvent)
						_pNetEvent->OnNetJoin(pClient);
					OnClientJoin(pClient);
				}
				_clientsBuff.clear();
				_clients_change = true;
			}

			//如果没有需要处理的客户端，就跳过
			if (_clients.empty())
			{
				CELLThread::Sleep(1);
				//旧的时间戳
				_oldTime = CELLTime::getNowInMilliSec();
				continue;
			}

			CheckTime();
			if (!DoNetEvents())
			{
				pThread->Exit();
				break;
			}
			DoMsg();
		}
		CELLLog_Info("CELLServer%d.OnRun exit", _id);
	}

	virtual bool DoNetEvents() = 0;

	void CheckTime()
	{
		//当前时间戳
		auto nowTime = CELLTime::getNowInMilliSec();
		auto dt = nowTime - _oldTime;
		_oldTime = nowTime;

		CELLClient* pClient = nullptr;
		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			pClient = iter->second;
			//心跳检测
			if (pClient->checkHeart(dt))
			{
#ifdef CELL_USE_IOCP
				if(pClient->isPostIoAction())
					pClient->destory();
				else
					OnClientLeave(pClient);
#else
				OnClientLeave(pClient);
#endif // CELL_USE_IOCP
				iter = _clients.erase(iter);
				continue;
			}

			////定时发送检测
			//pClient->checkSend(dt);

			iter++;
		}
	}

	void OnClientLeave(CELLClient* pClient)
	{
		if (_pNetEvent)
			_pNetEvent->OnNetLeave(pClient);
		_clients_change = true;
		delete pClient;
	}

	virtual void OnClientJoin(CELLClient* pClient)
	{

	}

	void OnNetRecv(CELLClient* pClient)
	{
		if (_pNetEvent)
			_pNetEvent->OnNetRecv(pClient);
	}

	void DoMsg()
	{
		CELLClient* pClient = nullptr;
		for (auto itr : _clients)
		{
			pClient = itr.second;
			//循环 判断是否有消息需要处理
			while (pClient->hasMsg())
			{
				//处理网络消息
				OnNetMsg(pClient, pClient->front_msg());
				//移除消息队列（缓冲区）最前的一条数据
				pClient->pop_front_msg();
			}
		}
	}

	//接收数据 处理粘包 拆分包
	int RecvData(CELLClient* pClient)
	{
		//接收客户端数据
		int nLen = pClient->RecvData();
		//触发<接收到网络数据>事件
		if (_pNetEvent)
			_pNetEvent->OnNetRecv(pClient);
		return nLen;
	}

	//响应网络消息
	virtual void OnNetMsg(CELLClient* pClient, netmsg_DataHeader* header)
	{
		if (_pNetEvent)
			_pNetEvent->OnNetMsg(this, pClient, header);
	}

	void addClient(CELLClient* pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	void Start()
	{
		_taskServer.Start();
		_thread.Start(
			//onCreate
			nullptr,
			//onRun
			[this](CELLThread* pThread) {
				OnRun(pThread);
			},
			//onDestory
			[this](CELLThread* pThread) {
				ClearClients();
			}
		);
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

	//void addSendTask(CELLClient* pClient, netmsg_DataHeader* header)
	//{
	//	_taskServer.addTask([pClient, header]() {
	//		pClient->SendData(header);
	//		delete header;
	//	});
	//}
private:
	void ClearClients()
	{
		for (auto iter : _clients)
		{
			delete iter.second;
		}
		_clients.clear();

		for (auto iter : _clientsBuff)
		{
			delete iter;
		}
		_clientsBuff.clear();
	}
protected:
	//正式客户队列
	std::map<SOCKET, CELLClient*> _clients;
private:
	//缓冲客户队列
	std::vector<CELLClient*> _clientsBuff;
	//缓冲队列的锁
	std::mutex _mutex;
	//网络事件对象
	INetEvent* _pNetEvent = nullptr;
	//
	CELLTaskServer _taskServer;
	//旧的时间戳
	time_t _oldTime = CELLTime::getNowInMilliSec();
	//
	CELLThread _thread;
protected:
	//
	int _id = -1;
	//客户列表是否有变化
	bool _clients_change = true;
};

#endif // !_CELL_SERVER_HPP_
