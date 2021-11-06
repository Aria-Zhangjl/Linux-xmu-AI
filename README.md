# Linux-xmu-AI
xmu AI18级Linux大作业
该项目要求使用Unix高级IPC机制（包括信号量和共享内存）来实现进程同步。

实验要求实现一个多进程系统来模拟一个单向隧道内交通流量控制。出于安全原因，隧道内一次不能超过N辆车。入口处的交通灯控制车辆的进入，入口和出口处的车辆检测器则可以检测交通流量。当隧道已满时，到达的汽车将不允许进入隧道，直到隧道中有车辆离开。

隧道内的每辆车都可以访问和修改一个用以模拟隧道邮箱系统的共享内存段（可以看成是一个数组，访问操作操作包括：r和w），这样，隧道内的车辆就在进隧道后保持其手机通讯（隧道将阻塞手机信号）。隧道外的汽车则不需要访问该共享内存段。

每辆车都由一个Linux进程来模拟。当汽车在隧道内，它们的操作是允许并发的，并且必须与Unix高级IPC机制进行同步。如果一个给定邮箱尚未因写操作而锁定，则对它的读操作可以并行，但对同一邮箱的写操作则同一时刻只允许一个。
## 输入格式
程序的输入包括隧道行程时间，每个存储段的大小以及车辆到达的顺序表，格式如下：
```
total_number_of_cars
maximum_number_of_cars_in_tunnel
tunnel_travel_time
total_number_of_mailboxes
memory_segment_size            /* size of each mailbox */

car_no_1
<operation> <length>|<message> <duration> <mailbox-no>
...
...
end.

car_no_2
<operation> <length>|<message> <duration> <mailbox-no>
...
...
end.

...
...

car_no_LAST
<operation> <length>|<message> <duration> <mailbox-no>
...
...
end.
```

如果一个操作是写（w），则后面会跟随一个字符串，如下：
```
 w 'hello' 20 5        /* write 'hello' into mailbox 5 with a
                      duration of 20 milliseconds */
```
如果一个操作是写（r），则对应邮箱里的目标信息将被读取并保存在车辆（也就是进程）的内存中以备过后打印，如下：
```
 r 20 10 5           /* read 20 characters from mailbox 5 for 10
                     milliseconds */
```
每辆车针对每个邮箱都维护一个读指针，以防止其重复读取信息。如果没有更多字符需要读取，应打印出“邮箱末尾”消息。但是，每个邮箱只有一个写指针。

如果汽车在隧道外，则写入和读取操作都应打印如下消息：
```
car X is outside the tunnel writing (reading),
```
其中X表示车辆号。

除了上述的消息，每辆车的手机内存内容也应该打印出来。
## 样例输入
```
3
3
5
5
200
car_no_1
r 4 3 1
r 4 3 2
r 4 3 3
r 4 3 4
end.
car_no_2
r 4 3 1
r 4 3 2
r 4 3 3
r 4 3 4
end.
car_no_3
w aaaaaaa 1 1
w bbbbbbb 1 2
w ccccccc 1 3
r 4 3 1
r 4 3 2
r 4 3 3
r 4 3 4
end.
```
