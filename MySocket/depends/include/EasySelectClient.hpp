#ifndef _EasySelectClient_hpp_
#define _EasySelectClient_hpp_

#include"EasyTcpClient.hpp"
#include"CELLFDSet.hpp"

class EasySelectClient : public EasyTcpClient
{
public:
	
	//处理网络消息
	bool OnRun(int microseconds = 1)
	{
		if (isRun())
		{
			SOCKET _sock = _pClient->sockfd();


			_fdRead.zero();
			_fdRead.add(_sock);

			_fdWrite.zero();

			timeval t = { 0,microseconds };
			int ret = 0;
			if (_pClient->needWrite())
			{
				
				_fdWrite.add(_sock);
				ret = select(_sock + 1, _fdRead.fdset(), _fdWrite.fdset(), nullptr, &t);
			}else {
				ret = select(_sock + 1, _fdRead.fdset(), nullptr, nullptr, &t);
			}

			if (ret < 0)
			{
				CELLLog_Error("CELLSelectClient.OnRun.wait clientId<%d> sockfd<%d>", _pClient->id, (int)_sock);

				Close();
				return false;
			}

			if (_fdRead.has(_sock))
			{
				if (SOCKET_ERROR == RecvData())
				{
					CELLLog_Error("<socket=%d>OnRun.select RecvData exit", (int)_sock);
					Close();
					return false;
				}
			}

			if (_fdWrite.has(_sock))
			{
				if (SOCKET_ERROR == _pClient->SendDataReal())
				{
					CELLLog_Error("<socket=%d>OnRun.select SendDataReal exit", (int)_sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}
protected:
	CELLFDSet _fdRead;
	CELLFDSet _fdWrite;
};

#endif