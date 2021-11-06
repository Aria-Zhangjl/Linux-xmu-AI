#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/types.h>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "car.cpp"
#include "tunnel.cpp"
using namespace std;

int main(int argc, char ** argv)
{
	 int total_number_of_cars, max_number_of_car_in_tunnel, tunnel_travel_time,total_number_of_mailboxes,memory_segment_size;
	ifstream fin(argv[1]);
	if(fin.is_open()==0)
	{
		cout<<"No such file!"<<endl;
		return 0;
	}
    fin >> total_number_of_cars >> max_number_of_car_in_tunnel >> tunnel_travel_time >>total_number_of_mailboxes>>memory_segment_size;
	string temp;
	getline(fin,temp);//读入第一行
	tunnel T(max_number_of_car_in_tunnel,total_number_of_mailboxes,memory_segment_size,tunnel_travel_time);
	pid_t mypid;
	int carid=0;
	int i=0;
	vector<car> Car;
	for(i=0;i<total_number_of_cars;i++)
	{
		car newcar(i,T);
		getline(fin,temp);
		
		getline(fin,temp);
		
		//读入每辆车的操作
		while(true)
		{
			getline(fin,temp);
			//cout<<temp<<endl;
			if(temp=="end.")
				break;
			istringstream in(temp);
			string t;
			vector<string> v;
			while (in >> t) 
			{
				v.push_back(t);
			}
			car_op pre;
			pre.op=v[0];
			pre.content=v[1];
			pre.duration=stoi(v[2]);
			pre.mid=stoi(v[3]);
			newcar.operation.push_back(pre);
		}
		Car.push_back(newcar);
	}
	fin.close();
	
	for (i = 0; i < total_number_of_cars; i++)
	{
		mypid = fork();
		if (mypid == 0)
		{
			Car[i].exec_car();
			cout<<"[Car "<<i<<"]: The car finished its operation."<<endl;
			break;
		}
		else
		{
			sleep(0.05);
		}
	}
	if(mypid!=0)
	{
		int childfinish=0;
		while(childfinish<total_number_of_cars)
		{
			 if(wait(NULL)!=-1)
            {
                childfinish++;
            }
		}
		cout<<endl<<"All of the cars are leaving the tunnel."<<endl;
		//退出前主进程打印出邮箱的内容
		T.print_memory();
	}
	return 0;
}
