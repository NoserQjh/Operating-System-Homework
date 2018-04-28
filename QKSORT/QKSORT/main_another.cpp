/*
#include <iostream>
#include <fstream>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <queue>

using namespace std;
#define INPUT_NAME "input.txt"
#define SOURCE_NAME "output.dat"
#define OUTPUT_NAME "output.txt"
#define N 1000000
//unsort类，用于记录还未排序的片段的信息
class unsort
{
public:
	int begin;
	int end;
	int length();
	unsort(int begin, int end);
};
unsort::unsort(int begin, int end)//unsort类构造函数
{
	this->begin = begin;
	this->end = end;
}

int unsort::length()//求当前片段长度
{
	return end - begin + 1;
}
int NUM_SORTED = 0;//用于记录已排序的数字数量
int NUM_THREAD = 0;//用于记录当前运行线程数量（用于调试，实际可删去）

HANDLE M_THREAD = CreateSemaphore(NULL, 20, 20, NULL);//线程信息量，用于判断是否可以增加线程
HANDLE M_UNSORTS = CreateSemaphore(NULL, 1, 1, NULL);//排序信息量，用于判断是否可以对unsorts队列进行操作

queue <unsort*> unsorts;//unsorts队列，用于记录还未排序的片段的信息



void Create_input()//随机创造输入信息
{
	fstream file;
	file.open(INPUT_NAME, ios::out);
	for (int i = 0; i < N; i++)
	{
		file << rand() << endl;
	}
}

void QKSORT(int begin, int end, int * x)//直接进行快速排序
{
	int a_stat = begin;
	int b_stat = end;
	int direct = -1;
	while (b_stat != a_stat)
	{
		if (*(x + a_stat)*direct < *(x + b_stat)*direct)
		{
			swap(*(x + a_stat), *(x + b_stat));
			swap(a_stat, b_stat);
			direct *= -1;
		}
		b_stat += direct;
	}
	if (b_stat - begin > 2)
		QKSORT(begin, b_stat - 1, x);
	if (end - a_stat > 2)
		QKSORT(b_stat + 1, end, x);
}

void Sort(unsort * target)//排序函数函数
{
	//读取共享内存
	HANDLE hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, TEXT("QKSORT"));
	int * x = (int *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, N * sizeof(int));
	int begin = target->begin;
	int end = target->end;
	if (target->length() >999)//判断当前序列长度是否大于1000
	{
		//进行一次划分
		int a_stat = begin;
		int b_stat = end;
		int direct = -1;
		while (b_stat != a_stat)
		{
			if (*(x + a_stat)*direct < *(x + b_stat)*direct)
			{
				swap(*(x + a_stat), *(x + b_stat));
				swap(a_stat, b_stat);
				direct *= -1;
			}
			b_stat += direct;
		}
		//划分结束，已排序数量加一
		NUM_SORTED++;

		//构造之前和之后的片段并判断是否需要进一步排序，若长度<2则对已排序数量进行更新，否则将其加入到未排序片段队列中
		unsort *front = new unsort(begin, a_stat - 1);
		unsort *behind = new unsort(a_stat + 1, end);
		if (front->length() < 2)
		{
			NUM_SORTED += front->length();
		}
		else
		{
			if (WaitForSingleObject(M_UNSORTS, INFINITE) == WAIT_OBJECT_0)//P操作
				unsorts.push(front);
			ReleaseSemaphore(M_UNSORTS, 1, NULL);//V操作
		}

		if (behind->length() < 2)
		{
			NUM_SORTED += behind->length();
		}
		else
		{
			if (WaitForSingleObject(M_UNSORTS, INFINITE) == WAIT_OBJECT_0)//P操作
				unsorts.push(behind);
			ReleaseSemaphore(M_UNSORTS, 1, NULL);//V操作
		}
		ReleaseSemaphore(M_THREAD, 1, NULL);//V操作
		NUM_THREAD--;
	}
	else
	{
		//长度<1000则直接进行快排并更新排序数量
		QKSORT(begin, end, x);
		NUM_SORTED += end - begin + 1;
		ReleaseSemaphore(M_THREAD, 1, NULL);//V操作，线程结束释放线程资源
		NUM_THREAD--;
	}
	UnmapViewOfFile(x);//关闭文件
}

void Start()
{
	DWORD start_time = GetTickCount();
	//打开共享内存
	int * pBuffer;
	HANDLE hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, TEXT("QKSORT"));
	pBuffer = (int *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, N * sizeof(int));
	//开始时全序列为匹配，将其放入unsorts队列中
	unsort *a = new unsort(0, N - 1);
	unsorts.push(a);

	HANDLE *thread = new HANDLE;
	DWORD *threadID = new DWORD;
	while (NUM_SORTED< N)//只要还未完成排序则不断检测
	{
		if (WaitForSingleObject(M_THREAD, INFINITE) == WAIT_OBJECT_0)//P操作，判断是否可以添加线程
		{
			NUM_THREAD++;
			if (unsorts.size() > 0)//判断是否有未排序的片段
			{
				if (WaitForSingleObject(M_UNSORTS, INFINITE) == WAIT_OBJECT_0)//P操作，判断是否可以对unsorts进行操作
				{
					*thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Sort, unsorts.front(), 0, threadID);
					unsorts.pop();
					ReleaseSemaphore(M_UNSORTS, 1, NULL);//V操作，释放unsorts
				}
				else
					ReleaseSemaphore(M_UNSORTS, 1, NULL);//V操作，释放unsorts

			}
		}
		else
		{
			ReleaseSemaphore(M_THREAD, 1, NULL);//V操作，释放线程资源
			NUM_THREAD--;
		}
	}
	DWORD end_time = GetTickCount();
	cout << "排序耗时:" << end_time - start_time << "ms" << endl;
	//排序结束，输出排序结果
	fstream file;
	file.open(OUTPUT_NAME, ios::out);
	for (int i = 0; i < N; i++)
	{
		file << *(pBuffer + i) << endl;
	}
	file.close();
	cout << "程序运行完毕" << endl;
	UnmapViewOfFile((LPCVOID)pBuffer);
}


int main()//主函数
{
	Create_input();//创建输入信息
				   //将输入信息放入共享内存中
	int * pBuffer;
	HANDLE hFile = CreateFile(TEXT(SOURCE_NAME), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, N * sizeof(int), TEXT("QKSORT"));
	pBuffer = (int *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, N * sizeof(int));
	int * input = new int[N];
	fstream file;
	file.open(INPUT_NAME, ios::in);
	for (int i = 0; i < N; i++)
	{
		file >> *(pBuffer + i);
	}
	file.close();
	//开始排序
	Start();
	cin.get();
	return 0;
}
*/