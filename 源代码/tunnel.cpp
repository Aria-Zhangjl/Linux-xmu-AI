#pragma once
#include "my_ipc.cpp"
#include <sys/types.h>

#define tunnel_key 0x199705
#define mail_key_shm 0x199706
#define mail_key_sem 0x200706
#define mail_read 0x180706
#define mail_write 0x180705

using namespace std;
class tunnel
{
public:
	int max_car_in;//隧道的最大容量
	int car_in;//隧道里现有的汽车数
	int tun_car_semid;//控制车辆进入的信号量
	int mail_sum;//邮箱个数
	int mail_size;//每个邮箱的大小
	int travel_time;//通过隧道的时间
	
	int *mail_shmid;//各个邮箱的内存编号
	int *mail_semid;//各个邮箱的信号量写者优先的控制
	/*各个邮箱当前正要读/写的个数*/
	int readcount;
	int writecount;
	
	tunnel(int pid,int max_car,int mail_sum,int mail_size);
	
	void creat_tunnel_sem();//创建控制隧道流量的信号量
	void creat_mail_shared();//创建邮箱的共享内存
	void creat_mail_sem();//创建各个邮箱的信号量
	void print_memory();//退出时打印出内存的信息
	void del_tunnel_sem();//删除掉隧道流量控制的信号量
	void del_mail_shared();//删除掉邮箱的共享内存
	void del_mail_sem();//删除掉各个邮箱的信号量
};

tunnel::tunnel(int max_car,int mail_sum,int mail_size,int travel_time)
{

	this->max_car_in=max_car;
	this->car_in=0;
	this->mail_size=mail_size;
	this->mail_sum=mail_sum;
	this->travel_time=travel_time;
	this->mail_shmid=new int[mail_sum];
	this->mail_semid=new int[mail_sum];
	
	
	creat_tunnel_sem();
	creat_mail_shared();
	creat_mail_sem();
}

//创建控制隧道流量的信号量
void tunnel::creat_tunnel_sem()
{
	this->tun_car_semid=creat_sem(tunnel_key,this->max_car_in,1);
}

//创建邮箱的共享内存
void tunnel::creat_mail_shared()
{
	int i;
	for(i=0;i<mail_sum;i++)
	{
		this->mail_shmid[i]=create_shm(mail_key_shm+i,this->mail_size);
	}
	//创建读者写者的数量共享内存
	this->readcount=create_shm(mail_read,sizeof(int)*mail_sum);
	this->writecount=create_shm(mail_write,sizeof(int)*mail_sum);
	//初始值设置为0
	int* read=(int*)shmat(readcount,NULL,0);
	int* write=(int*)shmat(writecount,NULL,0);	
	memset(read,0,sizeof(int)*mail_sum);
	memset(write,0,sizeof(int)*mail_sum);
	//断开连接
	disconnect_shm(read);
	disconnect_shm(write);
}

//创建各个邮箱的信号量集，一共有5个
void tunnel::creat_mail_sem()
{
	int i=0;
	for(i=0;i<mail_sum;i++)
	{
		this->mail_semid[i]=creat_sem(mail_key_sem+i,1,5);
	}
}

//打印各个邮箱的信息
void tunnel::print_memory()
{
	cout<<"========================================"<<endl;
	for(int i=0;i<mail_sum;i++)
	{
		char* message=(char*)get_shm(this->mail_shmid[i]);
		if(strlen(message))
			cout<<"Mailbox["<<i<<"]'s content is:"<<message<<endl;
		else
			cout<<"Mailbox["<<i<<"]'s content is:NULL"<<endl;
	}
	del_tunnel_sem();
	del_mail_shared();
	del_mail_sem();
}

//删除掉隧道流量控制的信号量
void tunnel::del_tunnel_sem()
{
	delete_sem(tun_car_semid);
}

//删除掉邮箱的共享内存
void tunnel::del_mail_shared()
{
	int i;
	for(int i=0;i<mail_sum;i++)
	{
		delete_shm(mail_shmid[i]);
	}
	   delete_shm(readcount);
	   delete_shm(writecount);	
}

//删除掉各个邮箱的信号量
void tunnel::del_mail_sem()
{
	int i;
	for(i=0;i<mail_sum;i++)
	{
		delete_sem(mail_semid[i]);
	}
}
