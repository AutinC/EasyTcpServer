#ifndef _CELLObjectPool_hpp_
#define _CELLObjectPool_hpp_
#include<stdlib.h>
#include<assert.h>
#include<mutex>

#ifdef _DEBUG
	#ifndef xPrintf
		#include<stdio.h>
		#define xPrintf(...) printf(__VA_ARGS__)
	#endif
#else
	#ifndef xPrintf
		#define xPrintf(...)
	#endif
#endif // _DEBUG

template<class Type, size_t nPoolSzie>
class CELLObjectPool
{
public:
	CELLObjectPool()
	{
		initPool();
	}

	~CELLObjectPool()
	{
		if(_pBuf)
			delete[] _pBuf;
	}
private:
	class NodeHeader
	{
	public:
		//下一块位置
		NodeHeader* pNext;
		//内存块编号
		int nID;
		//引用次数
		char nRef;
		//是否在内存池中
		bool bPool;
	private:
		//预留
		char c1;
		char c2;
	};
public:
	//释放对象内存
	void freeObjMemory(void* pMem)
	{
		NodeHeader* pBlock = (NodeHeader*)((char*)pMem - sizeof(NodeHeader));
		xPrintf("freeObjMemory: %llx, id=%d\n", pBlock, pBlock->nID);
		assert(1 == pBlock->nRef);
		if (pBlock->bPool)
		{
			std::lock_guard<std::mutex> lg(_mutex);
			if (--pBlock->nRef != 0)
			{
				return;
			}
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else {
			if (--pBlock->nRef != 0)
			{
				return;
			}
			delete[] pBlock;
		}
	}
	//申请对象内存
	void* allocObjMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		NodeHeader* pReturn = nullptr;
		if (nullptr == _pHeader)
		{
			pReturn = (NodeHeader*)new char[sizeof(Type) + sizeof(NodeHeader)];
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pNext = nullptr;
		}
		else {
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}
		xPrintf("allocObjMemory: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		return ((char*)pReturn + sizeof(NodeHeader));
	}
private:
	//初始化对象池
	void initPool()
	{
		//断言
		assert(nullptr == _pBuf);
		if (_pBuf)
			return;
		//计算对象池的大小
		size_t realSzie = sizeof(Type) + sizeof(NodeHeader);
		size_t n = nPoolSzie*realSzie;
		//申请池的内存
		_pBuf = new char[n];
		//初始化内存池
		_pHeader = (NodeHeader*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pNext = nullptr;
		//遍历内存块进行初始化
		NodeHeader* pTemp1 = _pHeader;

		for (size_t n = 1; n < nPoolSzie; n++)
		{
			NodeHeader* pTemp2 = (NodeHeader*)(_pBuf + (n* realSzie));
			pTemp2->bPool = true;
			pTemp2->nID = n;
			pTemp2->nRef = 0;
			pTemp2->pNext = nullptr;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}
	}
private:
	//
	NodeHeader* _pHeader;
	//对象池内存缓存区地址
	char* _pBuf;
	//
	std::mutex _mutex;
};

template<class Type, size_t nPoolSzie>
class ObjectPoolBase
{
public:
	void* operator new(size_t nSize)
	{
		return objectPool().allocObjMemory(nSize);
	}

	void operator delete(void* p)
	{
		objectPool().freeObjMemory(p);
	}

	template<typename ...Args>
	static Type* createObject(Args ... args)
	{	//不定参数  可变参数
		Type* obj = new Type(args...);
		//可以做点想做的事情
		return obj;
	}

	static void destroyObject(Type* obj)
	{
		delete obj;
	}
private:
	//
	typedef CELLObjectPool<Type, nPoolSzie> ClassTypePool;
	//
	static ClassTypePool& objectPool()
	{	//静态CELLObjectPool对象
		static ClassTypePool sPool;
		return sPool;
	}
};

#endif // !_CELLObjectPool_hpp_
