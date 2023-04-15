#ifndef _ALLOCTOR_H_
#define _ALLOCTOR_H_

void* operator new(size_t size);
void operator delete(void* p);
void* operator new[](size_t size);
void operator delete[](void* p);
void* mem_alloc(size_t size);
void mem_free(void* p);
#endif // !_ALLOCTOR_H_
