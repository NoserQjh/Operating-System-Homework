#include<iostream>
#include<fstream>
#include<queue>
#include"windows.h"
using namespace std;

#define INPUT_NAME "input.txt" //输入文件名
#define OUTPUT_NAME "output.txt" //输出文件名
#define N 3 //柜台数
#define T 100 //时钟周期

int NUM = 1;//号码
int NUM_CUSTOM = 0;//总顾客数
int NUM_DONE = 0;//已完成的顾客数
int time = 0;//当前时刻数

HANDLE M_GET_NUM = CreateSemaphore(NULL, 1, 1, NULL);//顾客取号信号量
HANDLE M_CALL_NUM = CreateSemaphore(NULL, 1, 1, NULL);//柜台叫号信号量

class customer//顾客类
{
public:
	customer(int customer_num, int enter_time, int duration_time);
	friend void Get_num(customer *this_customer);
	int Get_serve(int server_num, int start_num);
	bool Entered();
	void End_serve();
	bool got_num;
	void Output(fstream *file);
private:
	HANDLE M_CUSTOM = CreateSemaphore(NULL, 1, 1, NULL);//顾客信号量
	int customer_num;
	int enter_time;
	int duration_time;
	int num;
	int start_time;
	int server_num;
	int end_time;
};

customer::customer(int customer_num, int enter_time, int duration_time)//顾客初始化
{
	this->customer_num = customer_num;
	this->enter_time = enter_time;
	this->duration_time = duration_time;
	this->got_num = false;
	WaitForSingleObject(this->M_CUSTOM, INFINITE);//P操作，抬起以保护用户将要使用的数据
}

queue <customer> customers_queue;
customer ** customers;
queue <customer*> waitlist;
void customer::Output(fstream *file)//输出当前顾客信息
{
	*file << enter_time << '\t';
	*file << start_time << '\t';
	*file << end_time << '\t';
	*file << server_num << '\n';
}

bool customer::Entered()//确认是否到进入时间
{
	return (enter_time == time);
}

int customer::Get_serve(int server_num, int start_time)//开始接受服务时记录柜台号及开始时间
{
	this->server_num = server_num;
	this->start_time = start_time;
	//cout << "顾客 " << customer_num << " 在 " << start_time << " 时刻开始被 "<< server_num <<" 号柜台服务" << endl;
	return duration_time;
}

void customer::End_serve()//结束服务时记录结束时间
{
	this->end_time = time;
	//cout << "顾客 " << customer_num << " 在 " << end_time << " 时刻结束了服务" << endl;
	NUM_DONE++;
	ReleaseSemaphore(this->M_CUSTOM, 1, NULL);//V操作，用户相关数据使用结束，同步结束
}

void Get_num(customer *this_customer)//取号过程
{
	Sleep(T / 2);
	if (WaitForSingleObject(M_GET_NUM, INFINITE) == WAIT_OBJECT_0)//P操作
	{
		this_customer->num = NUM;//记录当前号码
		NUM++;//号码加一
		waitlist.push(this_customer);//开始排队
		ReleaseSemaphore(M_GET_NUM, 1, NULL);//V操作
		//cout << "顾客 " << this_customer->customer_num << " 在 " << time << " 时刻取到了号码 " << this_customer->num << endl;
	}
}



class server//柜台类
{
public:
	server(int server_num);
	friend void Call_num(server *this_server);
	void serve(int duration);
private:
	customer * custom;
	int server_num;
};

server::server(int server_num)//柜台初始化
{
	this->server_num = server_num;
}

void server::serve(int duration)//柜台服务
{
	Sleep(duration * T);//模拟服务过程
	custom->End_serve();//结束服务
	Call_num(this);//继续叫号
}

void Call_num(server *this_server)//叫号过程
{
	while (NUM_DONE < NUM_CUSTOM)
	{
		if (WaitForSingleObject(M_CALL_NUM, INFINITE) == WAIT_OBJECT_0)//P操作
		{
			if (waitlist.size() != 0)//有顾客在等待则开始叫号，否则忙等待
			{
				this_server->custom = waitlist.front();//提出找到顾客中优先级最高的
				waitlist.pop();//将该顾客从等待队列中移除
				ReleaseSemaphore(M_CALL_NUM, 1, NULL);//V操作
				int duration;
				duration = this_server->custom->Get_serve(this_server->server_num, time);//该顾客服务柜台为当前柜台
				this_server->serve(duration);//开始服务
			}
			else
				ReleaseSemaphore(M_CALL_NUM, 1, NULL);//V操作
		}
	}
}

void clock()//模拟时钟函数
{
	while (true)
	{
		Sleep(T);
		time++;
	}
}

void start()
{
	HANDLE *custom_thread = new HANDLE[NUM_CUSTOM];
	DWORD *custom_threadID = new DWORD[NUM_CUSTOM];
	while (NUM_DONE < NUM_CUSTOM)
	{
		int i;
		for (i = 0; i < NUM_CUSTOM; i++)
		{
			if ((*(customers + i))->Entered())
			{
				if (!(*(customers + i))->got_num)//若时间到且该顾客未取号则加入取号线程
				{
						
					*(custom_thread + i) = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Get_num, *(customers + i), 0, custom_threadID + i);
					(*(customers + i))->got_num = true;
				}
			}
		}
	}
	fstream file;
	file.open(OUTPUT_NAME,ios::out);//所有顾客服务结束后开始输出
	if (!file)
	{
		cout << "打开输出文件失败" << endl;
	}
	else
		cout << "正在写入输出文件信息..." << endl;
	for (int i = 0; i < NUM_CUSTOM; i++)
	{
		(*(customers + i))->Output(&file);
	}
	file.close();
}

int main()
{
	fstream file;
	
	file.open(INPUT_NAME);//读入输入文件信息
	if(!file)
	{
		cout << "打开输入文件失败" << endl;
	}
	cout << "正在读取输入文件信息..." << endl;
	while (!file.eof())
	{
		int customer_num;
		int enter_time;
		int duration_time;

		file >> customer_num >> enter_time >> duration_time;
		customers_queue.push(customer(customer_num, enter_time, duration_time));
		NUM_CUSTOM++;
	}
	cout << "共读入" << customers_queue.size() << "个顾客的相关信息" << endl;
	customers = new customer*[NUM_CUSTOM];
	for (int i = 0; i < NUM_CUSTOM; i++)
	{
		*(customers + i) = &customers_queue.front();
		customers_queue.pop();
	}

	HANDLE clock_thread;
	DWORD clock_threadID;

	clock_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)clock, NULL, 0, &clock_threadID);//开始时钟函数

	HANDLE server_thread[N];
	DWORD server_threadID[N];
	server **servers;
	servers = new server*[N];
	for (int i = 0; i < N; i++)//令所有柜台开始服务
	{
		*(servers + i) = new server(i);//初始化柜台
		server * thisserver = *(servers + i);
		server_thread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Call_num, *(servers + i), 0, &server_threadID[i]);
	}

	start();//开始进入顾客

	system("pause");
	return 0;
}


