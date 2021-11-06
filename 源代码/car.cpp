#include "my_ipc.cpp"
#include "tunnel.cpp"
#include <vector>
#include <sys/types.h>
#include <sys/time.h>
#include <ctime>
#include <cstring>
using namespace std;
struct car_op
{
	string op;//r或者w
	string content;//r的长度或者w的字符串
	int duration;//占用的时间
	int mid;//操作邮箱的id
};
class car
{
public:
	int carid;//汽车编号
	tunnel *tn;//隧道
	long int enter;//进入隧道时间
	
	vector<car_op> operation;//该辆车要进行的操作
	
	string car_mem;//车辆的内存
	
	car(int carid,tunnel &tn);
	void coming_to_tunnel();//到达隧道
	void read_from_maibox(int mid,int length,int lasttime);//读邮箱
	void write_to_mailbox(string str,int mid,int lasttime);//写邮箱
	void leaving_from_tunnel();//离开隧道
	void exec_car();
	int *read_p;
};

car::car(int id,tunnel &tn)
{
	this->carid=id;
	this->tn=&tn;
	this->car_mem="";
	this->read_p=new int[tn.mail_sum];
}

void car::coming_to_tunnel()
{
		printf("[Car %d]: The car is coming.\n",this->carid);
		Wait(tn->tun_car_semid,0);		
		printf("[Car %d]: The car is entering the tunnel.\n",this->carid);
		tn->car_in+=1;
		struct timeval tv;
		gettimeofday(&tv, NULL);
		this->enter= tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void car::leaving_from_tunnel()
{
	//车辆离开隧道
	Signal(tn->tun_car_semid,0);
	printf("[Car %d]: The car is leaving the tunnel.\n",this->carid);
	//打印车的内存
	if(car_mem.size())
		cout<<"[Car "<<carid<<"]: The memory of this car "<<carid<<" is: "<<car_mem<<"."<<endl;
	else
		cout<<"[Car "<<carid<<"]: The memory of this car "<<carid<<" is: NULL."<<endl;
	tn->car_in--;
}

//读邮箱
void car::read_from_maibox(int mid,int length,int lasttime)
{
	//获取信号量
	Wait(tn->mail_semid[mid],2);//z
	//Wait(tn->z[mid]);
	Wait(tn->mail_semid[mid],4);//rsem
	//Wait(tn->rsem[mid]);
	Wait(tn->mail_semid[mid],0);//x
	//Wait(tn->x[mid]);
	//获取readcount
	int* read=(int*)get_shm(tn->readcount);
	read[mid]++;

	if(read[mid]==1)
		Wait(tn->mail_semid[mid],3);//wsem
		//Wait(tn->wsem[mid]);
	Signal(tn->mail_semid[mid],0);//x
	//Signal(tn->x[mid]);
	Signal(tn->mail_semid[mid],4);//rsem
	//Signal(tn->rsem[mid]);
	Signal(tn->mail_semid[mid],2);//z
	//Signal(tn->z[mid]);
	usleep(lasttime*1000);
	char* message=(char*)get_shm(tn->mail_shmid[mid]);
	string temp=message;
	string pre_read;
	
	//判断这次是否会读到末尾
	if(this->read_p[mid]+length>temp.size())
	{
		car_mem+=temp.substr(this->read_p[mid]);
		pre_read=temp.substr(this->read_p[mid]);
		cout<<"[Car "<<carid<<"]: The car reached the end of the mailbox["<<mid<<"], can't read more!"<<endl;
	}
	else
	{
		car_mem+=temp.substr(this->read_p[mid],length);
		pre_read=temp.substr(this->read_p[mid],length);
	}
	//断开与共享内存区的连接
	this->read_p[mid]=strlen(message);
	disconnect_shm(message);
	
	//释放对该邮箱的锁
	Wait(tn->mail_semid[mid],0);//x
	//Wait(tn->x[mid]);
	read[mid]--;
	if(read[mid]==0)
		Signal(tn->mail_semid[mid],3);//wsem
		//Signal(tn->wsem[mid]);
    Signal(tn->mail_semid[mid],0);
	//Signal(tn->x[mid]);
	//断开连接
	disconnect_shm(read);
	cout<<"[Car "<<carid<<"]: The car is succeeding in reading \""<<pre_read<<"\" from mailbox["<<mid<<"]"<<endl;
}

//写邮箱
void car::write_to_mailbox(string context,int mid,int lasttime)
{
	//获取锁
	Wait(tn->mail_semid[mid],1);//y
	//Wait(tn->y[mid]);

	//得到共享内存区
	int* write=(int*)get_shm(tn->writecount);
	write[mid]++;
	if(write[mid]==1)
		Wait(tn->mail_semid[mid],4);
		//Wait(tn->rsem[mid]);
	Signal(tn->mail_semid[mid],1);
	//Signal(tn->y[mid]);
	Wait(tn->mail_semid[mid],3);
	//Wait(tn->wsem[mid]);

	//根据时间占用邮箱
	usleep(lasttime*1000);
	char* message=(char*)get_shm(tn->mail_shmid[mid]);
	strcat(message,context.data());
	
	//断开与共享内存区的连接	
	disconnect_shm(message);
	
	//释放对该邮箱的锁
	Signal(tn->mail_semid[mid],3);
	//Signal(tn->wsem[mid]);	
	Wait(tn->mail_semid[mid],1);
	//Wait(tn->y[mid]);
	write[mid]--;
	if(write[mid]==0)
		Signal(tn->mail_semid[mid],4);
		//Signal(tn->rsem[mid]);
	Signal(tn->mail_semid[mid],1);
	//Signal(tn->y[mid]);
	disconnect_shm(write);
	cout<<"[Car "<<carid<<"]: The car is succeeding in writing \""<<context<<"\" to mailbox["<<mid<<"]"<<endl;
}

//车辆行驶的具体操作
void car::exec_car()
{
	//进入隧道
	coming_to_tunnel();
	int i;
	for(i=0;i<operation.size();i++)
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		long int now= tv.tv_sec * 1000 + tv.tv_usec / 1000;
		if((now-enter)>=tn->travel_time)
			break;
		if(operation[i].op=="w")//是写操作
		{
			//如果是第一次进入隧道
			if(i==0)
			{
				int duration=(operation[i].duration-(now-enter));
				write_to_mailbox(operation[i].content,operation[i].mid,duration);
			}
			else
				write_to_mailbox(operation[i].content,operation[i].mid,operation[i].duration);
		}
		else//读操作
		{
			//如果是第一次进入隧道
			int length=stoi(operation[i].content);
			if(i==0)
			{
				int duration=(operation[i].duration-(now-enter));
				read_from_maibox(operation[i].mid,length,duration);
			}
			else
				read_from_maibox(operation[i].mid,length,operation[i].duration);
		}
	}
	//如果操作完成但是时间没到，继续走
	struct timeval tv;
	gettimeofday(&tv, NULL);
	long int now= tv.tv_sec * 1000 + tv.tv_usec / 1000;
	if(now-enter<tn->travel_time)
		usleep((tn->travel_time-(now-enter))*1000);
	//时间到了，离开隧道
	leaving_from_tunnel();
	//还没有完成就出了隧道
	for(;i<operation.size();i++)
	{
		if(operation[i].op=="w")
			cout<<"[Car "<<carid<<"]: The car is outside the tunnel writing."<<endl;
		else
			cout<<"[Car "<<carid<<"]: The car is outside the tunnel reading."<<endl;
	}
}
