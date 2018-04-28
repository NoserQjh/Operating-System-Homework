#include<iostream>
#include<string>
#include<windows.h>
using namespace std;
#define SOURCE_NAME "source.dat"

HANDLE M_ALLOCATE = CreateSemaphore(NULL, 0, 1, NULL);//分配信号量
HANDLE M_TRACK = CreateSemaphore(NULL, 0, 1, NULL);//跟踪信号量
DWORD pageSize = 4096;

class mem_info//用于存储虚拟内存信息
{
public:
	PVOID base_address;
	PVOID allocation_base;
	char * allocation_protect;
	SIZE_T region_size;
	char * state;
	char * protect;
	char * type;
};

void Allocator()//分配活动线程
{
	//打开共享内存
	LPVOID * memPtr;
	HANDLE hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, TEXT("QKSORT"));
	memPtr = (LPVOID * )MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LPVOID));
	char command[100];
	while (strcmp(command, "quit") != 0)
	{
		if (WaitForSingleObject(M_ALLOCATE, INFINITE) == WAIT_OBJECT_0)//P操作
		{
			//提示用户输入command
			cout << "Please input your command\n";
			cin >> command;
			system("cls");
			//reserve
			if (strcmp(command, "reserve") == 0)
			{
				*memPtr = VirtualAlloc(NULL, pageSize, MEM_RESERVE, PAGE_READWRITE);
				cout << "Reserve:" << endl;
				cout << "MemoryAddress:" << *memPtr << endl;
				ReleaseSemaphore(M_TRACK, 1, NULL);//V操作，可以进行track
			}
			//commit
			else if (strcmp(command, "commit") == 0)
			{
				cout << "Commit:" << endl;
				*memPtr = VirtualAlloc(*memPtr, pageSize, MEM_COMMIT, PAGE_READWRITE);
				ReleaseSemaphore(M_TRACK, 1, NULL);//V操作，可以进行track
			}
			//lock
			else if (strcmp(command, "lock") == 0)
			{
				cout << "Lock:" << endl;
				VirtualLock(*memPtr, pageSize);
				ReleaseSemaphore(M_TRACK, 1, NULL);//V操作，可以进行track
			}
			//unlock
			else if (strcmp(command, "unlock") == 0)
			{
				cout << "Unlock:" << endl;
				VirtualUnlock(*memPtr, pageSize);
				ReleaseSemaphore(M_TRACK, 1, NULL);//V操作，可以进行track
			}
			//recycle
			else if (strcmp(command, "recycle") == 0)
			{
				cout << "Recycle:" << endl;
				VirtualFree(*memPtr, pageSize, MEM_DECOMMIT);
				ReleaseSemaphore(M_TRACK, 1, NULL);//V操作，可以进行track
			}
			//release
			else if (strcmp(command, "release") == 0)
			{
				cout << "Release:" << endl;
				VirtualFree(*memPtr, 0, MEM_RELEASE);
				ReleaseSemaphore(M_TRACK, 1, NULL);//V操作，可以进行track
			}
			//help
			else if (strcmp(command, "help") == 0)
			{
				cout << "Enabled command:" << endl;
				cout << "reserve\ncommit\nlock\nunlock\nrecycle\nrelease\nquit\n\n";
				ReleaseSemaphore(M_ALLOCATE, 1, NULL);//V操作，可以进行allocate
			}
			//no such command
			else if(strcmp(command, "quit") != 0)
			{
				cout << "No such command, input \"help\" for guide\n";
				ReleaseSemaphore(M_ALLOCATE, 1, NULL);//V操作，可以进行allocate
			}
		}
		else
			ReleaseSemaphore(M_ALLOCATE, 1, NULL);//V操作
			
		
	}
	system("pause");
	return ;
}

mem_info MEMORY_BASIC_INFORMATION2mem_info(MEMORY_BASIC_INFORMATION &lpBuffer)//将 MEMORY_BASIC_INFORMATION 转换为 mem_info
{
	mem_info * info = new mem_info;
	//allocation_base
	info -> allocation_base = lpBuffer.AllocationBase;
	//base_address
	info -> base_address = lpBuffer.BaseAddress;
	//region_size
	info -> region_size = lpBuffer.RegionSize;
	//allocation_protect
	switch (lpBuffer.AllocationProtect)
	{
	case PAGE_EXECUTE:
		info -> allocation_protect = "execute";
		break;
	case PAGE_EXECUTE_READ:
		info -> allocation_protect = "execute read";
		break;

	case PAGE_EXECUTE_READWRITE:
		info -> allocation_protect = "execute readwrite";
		break;
	case PAGE_EXECUTE_WRITECOPY:
		info -> allocation_protect = "execute writecopy";
		break;
	case PAGE_GUARD:
		info -> allocation_protect = "guard";
		break;
	case PAGE_NOACCESS:
		info -> allocation_protect = "noaccess";
		break;
	case PAGE_NOCACHE:
		info -> allocation_protect = "nocache";
		break;
	case PAGE_READONLY:
		info -> allocation_protect = "readonly";
		break;
	case PAGE_READWRITE:
		info -> allocation_protect = "readwrite";
		break;
	case PAGE_WRITECOPY:
		info -> allocation_protect = "writecopy";
		break;
	case PAGE_WRITECOMBINE:
		info -> allocation_protect = "writecombine";
		break;
	default:
		info -> allocation_protect = "caller does not have access";
	}
	//protect
	switch (lpBuffer.Protect)
	{
	case PAGE_EXECUTE:
		info -> protect = "execute";
		break;
	case PAGE_EXECUTE_READ:
		info -> protect = "execute read";
		break;
	case PAGE_EXECUTE_READWRITE:
		info -> protect = "execute readwrite";
		break;
	case PAGE_EXECUTE_WRITECOPY:
		info -> protect = "execute writecopy";
		break;
	case PAGE_GUARD:
		info -> protect = "guard";
		break;
	case PAGE_NOACCESS:
		info -> protect = "noaccess";
		break;
	case PAGE_NOCACHE:
		info -> protect = "nocache";
		break;
	case PAGE_READONLY:
		info -> protect = "readonly";
		break;
	case PAGE_READWRITE:
		info -> protect = "readwrite";
		break;
	case PAGE_WRITECOPY:
		info -> protect = "writecopy";
		break;
	case PAGE_WRITECOMBINE:
		info -> protect = "writecombine";
		break;
	default:
		info -> protect = "caller does not have access";
	}
	//state
	switch (lpBuffer.State)
	{
	case MEM_COMMIT:
		info -> state = "memory commit"; 
		break;
	case MEM_FREE:
		info -> state = "memory free";
		break;
	case MEM_RESERVE:
		info -> state = "memory reserve";
		break;
	default:
		info -> state = "";
	}
	//type
	switch (lpBuffer.Type)
	{
	case MEM_IMAGE:
		info -> type = "memory image";
		break;
	case MEM_MAPPED:
		info -> type = "memory mapped";
		break;
	case MEM_PRIVATE:
		info -> type = "memory private";
		break;
	default:
		info -> type = "";
	}
	return * info;
}

void Tracker()//跟踪线程
{ 
	//打开共享内存
	LPVOID * memPtr;
	HANDLE hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, TEXT("QKSORT"));
	memPtr = (LPVOID * )MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LPVOID));
	//获取当前系统信息
	SYSTEM_INFO info; 
	GetSystemInfo(&info);
	cout << "HardwareInformation:" << endl;
	cout << "ProcessorNumber:" << info.dwNumberOfProcessors << endl;
	cout << "ProcessorType:" << info.dwProcessorType << endl;
	cout << "Mask:" << info.dwActiveProcessorMask << endl;
	cout << "PageSize:" << info.dwPageSize << endl;
	cout << "MinimumAddress:" << info.lpMinimumApplicationAddress << endl;
	cout << "MaximumAddress:" << info.lpMaximumApplicationAddress << endl << endl;
	MEMORY_BASIC_INFORMATION lpBuffer;
	ReleaseSemaphore(M_ALLOCATE, 1, NULL);//V操作，可以进行allocate
	while (true)
	{
		if (WaitForSingleObject(M_TRACK, INFINITE) == WAIT_OBJECT_0)//P操作，判断能否进行track
		{
			mem_info memInfo;
			VirtualQuery(* memPtr, &lpBuffer, sizeof(lpBuffer));//获取当前虚拟内存信息
			memInfo = MEMORY_BASIC_INFORMATION2mem_info(lpBuffer);//获取mem_info
			cout << "BaseAddress:" << memInfo.base_address << endl;
			cout << "AllocationBase:" << memInfo.allocation_base << endl;
			cout << "AllocationProtect:" << memInfo.allocation_protect << endl;
			cout << "RegionSize:" << memInfo.region_size << endl;
			cout << "State:" << memInfo.state << endl;
			cout << "Protect:" << memInfo.protect << endl;
			cout << "Type:" << memInfo.type << endl;
			cout << endl;
			ReleaseSemaphore(M_ALLOCATE, 1, NULL);//V操作，可以进行allocate
		}
	}
	return;
}

int main()
{
	//开辟共享内存
	LPVOID * memPtr;
	HANDLE hFile = CreateFile(TEXT(SOURCE_NAME), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, sizeof(LPVOID), TEXT("QKSORT"));
	memPtr = (LPVOID *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LPVOID));
	//打开跟踪活动线程
	HANDLE *tracker_thread = new HANDLE;
	DWORD *tracker_threadID = new DWORD;
	*tracker_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Tracker, NULL, 0, tracker_threadID);
	//打开分配活动线程
	HANDLE *allocator_thread = new HANDLE;
	DWORD *allocator_threadID = new DWORD;
	*allocator_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Allocator, NULL, 0, allocator_threadID);
	while (true);
}