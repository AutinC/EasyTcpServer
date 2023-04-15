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
		//��һ��λ��
		NodeHeader* pNext;
		//�ڴ����
		int nID;
		//���ô���
		char nRef;
		//�Ƿ����ڴ����
		bool bPool;
	private:
		//Ԥ��
		char c1;
		char c2;
	};
public:
	//�ͷŶ����ڴ�
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
	//��������ڴ�
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
	//��ʼ�������
	void initPool()
	{
		//����
		assert(nullptr == _pBuf);
		if (_pBuf)
			return;
		//�������صĴ�С
		size_t realSzie = sizeof(Type) + sizeof(NodeHeader);
		size_t n = nPoolSzie*realSzie;
		//����ص��ڴ�
		_pBuf = new char[n];
		//��ʼ���ڴ��
		_pHeader = (NodeHeader*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pNext = nullptr;
		//�����ڴ����г�ʼ��
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
	//������ڴ滺������ַ
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
	{	//��������  �ɱ����
		Type* obj = new Type(args...);
		//������������������
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
	{	//��̬CELLObjectPool����
		static ClassTypePool sPool;
		return sPool;
	}
};

#endif // !_CELLObjectPool_hpp_
