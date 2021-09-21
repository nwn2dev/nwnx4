#include <windows.h>
#include <string.h>
#include <stdio.h>

struct NWN2MemoryHeader
{
	DWORD size;
	DWORD unk2;
	DWORD NWN2Heap;
	DWORD unk4; //0
};

//void *NWN2MemoryAllocate(ulong size);

/*
class NWN2MemoryMgr
{
public:
	void *(*Allocate)(UINT size);
};*/
/*
void *(*NWN2Heap_Allocate)(UINT size);

void *(*NWN2Heap_Allocate)(UINT size);*/
class NWN2_Heap
{
public:
	__declspec( dllimport ) void * __thiscall Allocate(unsigned int);
};

class NWN2_HeapMgr
{
public:
	__declspec( dllimport ) static NWN2_HeapMgr * __cdecl NWN2_HeapMgr::Instance(void);
	__declspec( dllimport ) NWN2_Heap * __thiscall NWN2_HeapMgr::GetDefaultHeap(void);
};

//__declspec( dllimport ) void * __thiscall NWN2_Heap::Allocate(unsigned int);