#ifndef _CELL_BUFFER_HPP_
#define _CELL_BUFFER_HPP_

#include"CELL.hpp"

#ifdef CELL_USE_IOCP
#include"CELLIOCP.hpp"
#endif

class CELLBuffer
{
public:
	CELLBuffer(int nSize = 8192)
	{
		_nSize = nSize;
		_pBuff = new char[_nSize];
	}

	~CELLBuffer()
	{
		if (_pBuff)
		{
			delete[] _pBuff;
			_pBuff = nullptr;
		}
	}

	inline char* data()
	{
		return _pBuff;
	}

	//inline int dataLen()
	//{
	//	return _nLast;
	//}

	//inline int buffSize()
	//{
	//	return _nSize;
	//}

	bool push(const char* pData, int nLen)
	{
		////写入大量数据不一定要放到内存中
		////也可以存储到数据库或者磁盘等存储器中
		//if (_nLast + nLen > _nSize)
		//{
		//	//需要写入的数据大于可用空间
		//	int n = (_nLast + nLen) - _nSize;
		//	//拓展BUFF
		//	if (n < 8192)
		//		n = 8192;
		//	char* buff = new char[_nSize+n];
		//	memcpy(buff, _pBuff, _nLast);
		//	delete[] _pBuff;
		//	_pBuff = buff;
		//}

		if (_nLast + nLen <= _nSize)
		{
			//将要发送的数据 拷贝到发送缓冲区尾部
			memcpy(_pBuff + _nLast, pData, nLen);
			//计算数据尾部位置
			_nLast += nLen;

			if (_nLast == SEND_BUFF_SZIE)
			{
				++_fullCount;
			}

			return true;
		}
		else {
			++_fullCount;
		}

		return false;
	}

	void pop(int nLen)
	{
		int n = _nLast - nLen;
		if (n > 0)
		{
			memcpy(_pBuff, _pBuff + nLen, n);
		}
		_nLast = n;
		if (_fullCount > 0)
			--_fullCount;
	}

	int write2socket(SOCKET sockfd)
	{
		int ret = 0;
		//缓冲区有数据
		if (_nLast > 0 && INVALID_SOCKET != sockfd)
		{
			//发送数据
			ret = send(sockfd, _pBuff, _nLast, 0);
			if (ret <= 0)
			{
				CELLLog_PError("write2socket1:sockfd<%d> nSize<%d> nLast<%d> ret<%d>", sockfd, _nSize, _nLast, ret);
				return SOCKET_ERROR;
			}
			if (ret == _nLast)
			{//_nLast=2000 实际发送ret=2000
				//数据尾部位置清零
				_nLast = 0;
			}
			else {
				//_nLast=2000 实际发送ret=1000
				//CELLLog_Info("write2socket2:sockfd<%d> nSize<%d> nLast<%d> ret<%d>", sockfd, _nSize, _nLast, ret);
				_nLast -= ret;
				memcpy(_pBuff, _pBuff + ret, _nLast);
			}
			_fullCount = 0;
		}
		return ret;
	}

	int read4socket(SOCKET sockfd)
	{
		if (_nSize - _nLast > 0)
		{
			//接收客户端数据
			char* szRecv = _pBuff + _nLast;
			int nLen = (int)recv(sockfd, szRecv, _nSize - _nLast, 0);
			if (nLen <= 0)
			{
				CELLLog_PError("read4socket:sockfd<%d> nSize<%d> nLast<%d> nLen<%d>", sockfd, _nSize, _nLast, nLen);
				return SOCKET_ERROR;
			}
			//消息缓冲区的数据尾部位置后移
			_nLast += nLen;
			return nLen;
		}
		return 0;
	}

	bool hasMsg()
	{
		//判断消息缓冲区的数据长度大于消息头netmsg_DataHeader长度
		if (_nLast >= sizeof(netmsg_DataHeader))
		{
			//这时就可以知道当前消息的长度
			netmsg_DataHeader* header = (netmsg_DataHeader*)_pBuff;
			//判断消息缓冲区的数据长度大于消息长度
			return _nLast >= header->dataLength;
		}
		return false;
	}

	inline bool needWrite()
	{
		return _nLast > 0;
	}

#ifdef CELL_USE_IOCP
	IO_DATA_BASE* makeRecvIoData(SOCKET sockfd)
	{
		int nLen = _nSize - _nLast;
		if (nLen > 0)
		{
			_ioData.wsabuff.buf = _pBuff + _nLast;
			_ioData.wsabuff.len = nLen;
			_ioData.sockfd = sockfd;
			return &_ioData;
		}
		return nullptr;
	}

	IO_DATA_BASE* makeSendIoData(SOCKET sockfd)
	{
		if (_nLast > 0)
		{
			_ioData.wsabuff.buf = _pBuff;
			_ioData.wsabuff.len = _nLast;
			_ioData.sockfd = sockfd;
			return &_ioData;
		}
		return nullptr;
	}

	bool read4iocp(int nRecv)
	{
		if (nRecv > 0 && _nSize - _nLast >= nRecv)
		{
			_nLast += nRecv;
			return true;
		}
		CELLLog_Error("read4iocp:sockfd<%d> nSize<%d> nLast<%d> nRecv<%d>", _ioData.sockfd, _nSize, _nLast, nRecv);
		return false;
	}

	bool write2iocp(int nSend)
	{
		if (_nLast < nSend)
		{
			CELLLog_Error("write2iocp:sockfd<%d> nSize<%d> nLast<%d> nSend<%d>", _ioData.sockfd, _nSize, _nLast, nSend);
			return false;
		}
		if (_nLast == nSend)
		{//_nLast=2000 实际发送nSend=2000
		 //数据尾部位置清零
			_nLast = 0;
		}
		else {
			//_nLast=2000 实际发送ret=1000
			_nLast -= nSend;
			memcpy(_pBuff, _pBuff + nSend, _nLast);
		}
		_fullCount = 0;
		return true;
	}
#endif // CELL_USE_IOCP
private:
	//第二缓冲区 发送缓冲区
	char* _pBuff = nullptr;
	//可以用链表或队列来管理缓冲数据块
	//list<char*> _pBuffList;
	//缓冲区的数据尾部位置，已有数据长度
	int _nLast = 0;
	//缓冲区总的空间大小，字节长度
	int _nSize = 0;
	//缓冲区写满次数计数
	int _fullCount = 0;
#ifdef CELL_USE_IOCP
	IO_DATA_BASE _ioData = {};
#endif // CELL_USE_IOCP
};

#endif // !_CELL_BUFFER_HPP_
