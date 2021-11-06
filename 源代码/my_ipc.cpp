/*
* ipc控制，封装成类
*sembuf定义
*	struct sembuf
*	{  
*	unsigned short sem_num; //信号在信号集中的索引，0代表第一个信号，1代表第二个信号  
*	short sem_op; //操作类型  
*	short sem_flg; //操作标志  
*	};  
*/

#pragma once
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <iostream>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdlib.h>
using namespace std;

//创建共享内存区
int create_shm(int mykey,int size)
{
	int shmid=shmget(mykey,size,IPC_CREAT|0666);
	if(shmid==-1)
	{
		printf("shmget error!");
		exit(1);
	}
	return shmid;
}

//返回制定的共享内存区
//retshm指向该块内存区
void* get_shm(int shmid)
{
	void* retshm=shmat(shmid,(void*)0,0);
	if(shmid==-1)
	{
		printf("shmat error!\n");
		exit(1);
	}
	return retshm;
}

//断开共享内存区
void disconnect_shm(void *shmaddr)
{
	if(shmdt(shmaddr)==-1)
	{
		printf("shmdt error!\n");
		exit(1);
	}
}

//删除共享内存区
void delete_shm(int shmid)
{
	if(shmctl(shmid,IPC_RMID,NULL)==-1)
	{
		printf("shmctl error!\n");
		exit(1);
	}
}

union semun
{
	int val;	//信号灯的初值
	struct semid_ds *buf;
	unsigned short *array;
};

//建立信号量
int creat_sem(int mykey,int semval,int semnum)
{
	//创建信号量
	int semid=semget(mykey,semnum,IPC_CREAT|0666);
	if(semid==-1)
	{
		printf("semget error!");
		exit(1);
	}
	//初始化信号量
	union semun sem_union;
	sem_union.val=semval;
	for(int i=0;i<semnum;i++)
	{
		if(semctl(semid,i,SETVAL,sem_union)==-1)
		{
			printf("semctl error!");
			exit(1);
		}	
	}
	return semid;
}

//得到信号量
int get_sem(int mykey,int semnum)
{
	int semid=semget(mykey,semnum-1,IPC_CREAT);
	if(semid==-1)
	{
		printf("semget error!");
		exit(1);
	}
	return semid;
}

//等待信号量
void Wait(int sid,int sem_num)
{
   struct sembuf op;
   op.sem_num=sem_num;
   op.sem_op=-1;
   op.sem_flg=0;
   if(semop(sid, &op, 1)==-1)
   {
	   printf("semop error!\n");
	   exit(1);
   }
}

//通过信号量
void Signal(int sid,int sem_num)
{
   struct sembuf op;
   op.sem_num=sem_num;
   op.sem_op=1;
   op.sem_flg=0;
   if(semop(sid, &op, 1)==-1)
   {
	   printf("semop error!\n");
	   exit(1);
   }
}

//删除信号量
void delete_sem(int semid)
{
	if(semctl(semid, 7,IPC_RMID)==-1)
		{
			printf("semctl error!\n");
			exit(1);
		}	
}
