#ifndef _CELLClient_HPP_
#define _CELLClient_HPP_

#include"CELL.hpp"
#include"CELLBuffer.hpp"

//客户端心跳检测死亡计时时间
#define CLIENT_HREAT_DEAD_TIME 60000
//在间隔指定时间后
//把发送缓冲区内缓存的消息数据发送给客户端
#define CLIENT_SEND_BUFF_TIME 200
//客户端数据类型
class CELLClient
{
public:
	int id = -1;
	//所属serverid
	int serverId = -1;
public:
	CELLClient(SOCKET sockfd = INVALID_SOCKET):
		_sendBuff(SEND_BUFF_SZIE),
		_recvBuff(RECV_BUFF_SZIE)
	{
		static int n = 1;
		id = n++;
		_sockfd = sockfd;

		resetDTHeart();
		resetDTSend();
	}

	~CELLClient()
	{
		CELLLog::Info("s=%d CELLClient%d.~CELLClient\n", serverId, id);
		if (INVALID_SOCKET != _sockfd)
		{
#ifdef _WIN32
			closesocket(_sockfd);
#else
			close(_sockfd);
#endif
			_sockfd = INVALID_SOCKET;
		}
	}


	SOCKET sockfd()
	{
		return _sockfd;
	}

	int RecvData()
	{
		return _recvBuff.read4socket(_sockfd);
	}

	bool hasMsg()
	{
		return _recvBuff.hasMsg();
	}

	netmsg_DataHeader* front_msg()
	{
		return (netmsg_DataHeader*)_recvBuff.data();
	}

	void pop_front_msg()
	{
		if(hasMsg())
			_recvBuff.pop(front_msg()->dataLength);
	}

	//立即将发送缓冲区的数据发送给客户端
	int SendDataReal()
	{
		resetDTSend();
		return _sendBuff.write2socket(_sockfd);
	}

	//缓冲区的控制根据业务需求的差异而调整
	//发送数据
	int SendData(netmsg_DataHeader* header)
	{
		if (_sendBuff.push((const char*)header, header->dataLength))
		{
			return header->dataLength;
		}
		return SOCKET_ERROR;
	}

	void resetDTHeart()
	{
		_dtHeart = 0;
	}

	void resetDTSend()
	{
		_dtSend = 0;
	}

	//心跳检测
	bool checkHeart(time_t dt)
	{
		_dtHeart += dt;
		if (_dtHeart >= CLIENT_HREAT_DEAD_TIME)
		{
			CELLLog::Info("checkHeart dead:s=%d,time=%ld\n",_sockfd, _dtHeart);
			return true;
		}
		return false;
	}

	//定时发送消息检测
	bool checkSend(time_t dt)
	{
		_dtSend += dt;
		if (_dtSend >= CLIENT_SEND_BUFF_TIME)
		{
			//CELLLog::Info("checkSend:s=%d,time=%d\n", _sockfd, _dtSend);
			//立即将发送缓冲区的数据发送出去
			SendDataReal();
			//重置发送计时
			resetDTSend();
			return true;
		}
		return false;
	}
private:
	// socket fd_set  file desc set
	SOCKET _sockfd;
	//第二缓冲区 接收消息缓冲区
	CELLBuffer _recvBuff;
	//发送缓冲区
	CELLBuffer _sendBuff;
	//心跳死亡计时
	time_t _dtHeart;
	//上次发送消息数据的时间 
	time_t _dtSend;
	//发送缓冲区遇到写满情况计数
	int _sendBuffFullCount = 0;
};

#endif // !_CELLClient_HPP_