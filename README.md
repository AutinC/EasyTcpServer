# EasyTcpServer
# 0、简介

## 1. 方式：Socket、全栈、跨平台

### Socket

运用C++和Socket API来构建百万级处理能力的网络通信引擎

### 全栈

不仅包含后端(服务端）的开发知识，还包括前端（客户端)的网络通信知识，并且会在主流引擎工具实际应用。

### 跨平台

项目中的技术知识和设计方案可以应用在Windows/Linux/Android/IOS等主流操作系统上，支持Socket的其他语言也开始借鉴使用。

## 2. 开发工具

### Windows

Windows10+ Visual Studio 20XX

### Linux

Linux(ubuntu 16.04 64位)/GCC/Gedit编辑器
Visual Studio Code

Android

Windows 10 + VS2015 + NDK + JDK + ADT +ANT

### IOS/ Macos

MacOS(MacOS 10.12 64位)+ Xcode 9

### 辅助工具

虚拟机VMware Player、代码管理工具SVN、文本比较工具WinMarge等辅助开发工具

## 3. 学习计划

学会如何搭建不同平台下的C++开发环境。
学会Socket网络通信基础知识。
学会前端(客户端)网络通信，并实际应用到商业工具引擎中.
学会后端(服务端)网络通信，一步一步建立高性能服务器.

## 4. 最终任务

从1个用户连接到10000个用户链接，从每秒1个用户/1个数据包,到1万用户/1百万个数据包,我们探索性能瓶颈,
解决瓶颈,一步一步加入优化点,进行前后对比,深度解析每一个优化解决了那些问题。真正掌握核心知识。 



# 一、建立基础TCP服务端/客户端

![EasyTcpServer/20201126212745789.png at main · AutinC/EasyTcpServer (github.com)](https://github.com/AutinC/EasyTcpServer/blob/main/MySocket/images/20201126212745789.png)

## 1. 建立服务端大致流程

1. 建立一个套接字 **（socket）**
2. 绑定客户端连接的端口 **（bind）**
3. 监听网络端口 **（listen）**
4. 等待接受客户端连接 **（accept）**
5. 接收客户端发送的数据 **（recv）**
6. 向客户端发送数据 **（send）**
7. 关闭套接字 **（closesocket）**

## 2. 建立客户端大致流程

1. 建立一个套接字 **（socket）**
2. 连接服务器 **（connect）**
3. 向客户端发送数据 **（send）**
4. 接收客户端发送的数据 **（recv）**
5. 关闭套接字 **（closesocket）**

## 3. 实现多个客户端与服务器相连

![EasyTcpServer/20201126213341195.png at main · AutinC/EasyTcpServer (github.com)](https://github.com/AutinC/EasyTcpServer/blob/main/MySocket/images/20201126213341195.png)

## 4. 建立可持续处理请求的网络程序

  在进行socket连接后，即服务端进行**accept**操作、客户端进行**connect**操作后，使用循环，在此循环中进行**send/recv**操作传输数据，即可实现持续处理请求。

![EasyTcpServer/20201126214543105.png at main · AutinC/EasyTcpServer (github.com)](https://github.com/AutinC/EasyTcpServer/blob/main/MySocket/images/20201126214543105.png)



# 二、网络数据报文的定义与收发

## 1. 网络数据报文的格式定义

- 报文有两个部分，**包头**和**包体**，是网络消息的基本单元。
- **包头：** 描述本次消息包的大小，描述包体数据的作用。
- **包体：** 其中包含了需要传输的数据。

 根据此数据结构，可以根据包头的内容，来灵活的对包体的数据进行处理。

## 2. 将包头与包体数据分开收发

通过上文对网络数据报文的定义，可以很轻易的想到：

- 发送端进行两次send操作，**第一次send发送包头**，**第二次send发送包体**，即可实现网络数据报文的发送。

- 接收端进行两次recv操作，**第一次recv接收包头**，**第二次recv接收包体**并根据包头的内容进行数据处理，即可实现网络数据报文的接收。

  按以上操作，即可实现网络数据报文的收发。

## 3. 将分开收发报文数据改为一次收发

- 由上文可以知道，可以通过两次send和两次recv进行报文的收发，但是其中操作较为麻烦，需要多次声明DateHeader包头结构体，不仅消耗时间资源，也容易出错。
- 因此，可以尝试将分开收发改为一次收发。大致思路为完善报文的结构体，**使包体继承包头结构体，或者使包体结构体中包含一个包头结构体**。由此完善报文的结构体，只进行一次send操作，即可发送所有报文数据。
- 在进行数据接收时，我们先接收包头大小的数据，随后根据包头的内容，来确定接下来接收数据的大小，即**接收总数据大小 减去 包头数据大小** 的数据。而在接下来使用recv接收剩下数据时，要**使用指针偏移**，跳过结构体包头的接收(因为接收过了)，直接接收到包体数据位置上。



# 三、升级为select网络模型

## 1. 为什么要用select网络模型

通过前面的学习，已经实现了简单的网络报文收发。但是可以很明显的看出其中的缺点，那就是整个程序的运行是**阻塞模式**的：

- 服务端在与一个客户端进行socket连接时，只要连接不中断，那么就无法接收新的客户端的消息；
- 客户端在未输入命令时，是阻塞状态，也无法接收服务端发来的消息。

在之前碰到这个问题时，我的想法是通过多线程来解决程序运行中的阻塞问题，但是在最近的学习中，我了解到可以使用select网络模型来方便快捷的解决小型网络程序运行中的阻塞问题。（查阅I/O多路复用模型相关内容）

## 2. select相关使用总结与心得

在一开始的select使用中，我以为向select函数中传入fd_set地址，select会把待处理事件的socket放在set集合中，但是发现并不是这样。
经过网络上资料的查询以及我个人的测试，可以发现，用户首先需要把一份socket数组传入到此set中，**select函数的作用是移除该set中没有待处理事件的socket**，则剩下的socket都存在待处理事件(未决I/O操作)。这个过程可以说是一种“选择”的过程，select函数“选择”出需要操作的socket，这或许就是select(选择)的意思吧。
在接下来的源码中，对于需要存储所有已连接socket的服务端，我使用动态数组vector进行socket的储存。在进行select筛选前，先把vector中的socket导入到set中，随后set中筛选剩下的即为有待处理事件的socket。
如果服务端自己的socket提示有待处理事件，则说明有新的客户端尝试进行连接，此时进行accept操作即可。
对于客户端的多线程问题，需要注意**使用detach()方法使主线程与新线程分类**，否则可能会出现主线程先结束的情况，导致程序出错。
在线程中，我们可以引入一个bool变量，用来记录客户端是否仍在连接中，当输入exit命令退出客户端时，通过此bool变量使主线程停止，跳出循环。

## 3. 升级为select网络模型的思路

- 服务端升级（**select**）

  之前的思路是：

  ```cpp
  1.建立socket
  2.绑定端口IP
  3.监听端口
  4.与客户端连接
  while(true)
  {
  	5.接收数据
  	6.发送数据
  }
  7.关闭socket
  ```

  导致服务端只能与一个客户端进行连接，随后便进入循环，只能接收这一个客户端的消息，且由于send与recv函数都是阻塞函数，所以程序也是阻塞模式的。

  根据select网络模型，对服务端进行升级。

  ```cpp
  1.建立socket
  2.绑定端口IP
  3.监听端口
  while(true)
  {
  	4.使用select函数获取存在待监听事件的socket
  	5.如果有新的连接则与新的客户端连接
  	6.如果有待监听事件，则对其进行处理(接受与发送)
  }
  7.关闭socket
  ```

- 客户端升级（**select+多线程**）

  之前的思路是：

  ```cpp
  1.建立socket
  2.连接服务器
  while(true)
  {
  	3.发送数据
  	4.接收数据
  }
  5.关闭socket
  ```

  导致客户端在与一个服务端连接后，无法被动的接收服务器端发来的消息。因为send与recv函数都是阻塞函数，程序也为阻塞模式。如果我们想要客户端能接收服务端发来的消息，那么就可以使用select模型。

  根据select网络模型，对客户端进行升级。

  ```cpp
  1.建立socket
  2.连接服务器
  while(true)
  {
  	3.使用select函数获取服务器端是否有待处理事件
  	4.如果有，就处理它(接收/发送)
  }
  5.关闭socket
  ```

  但是，这样的程序结构也有很明显的缺点，因为数据接收函数也为阻塞函数，如果我们想要主动输入一些命令发送给服务端，就会阻塞程序运行。对此，我们可以引入**多线程**解决问题。

  ```cpp
  1.建立socket
  2.连接服务器
  3.建立新线程 用于发送命令
  while(true)
  {
  	4.使用select函数获取服务器端是否有待处理事件
  	5.如果有，就处理它(接收/发送)
  }
  5.关闭socket
  
  新线程：
  while(1)
  {
  	1.键入数据
  	2.发送数据
  }
  ```

  

# 四、跨平台移植到Linux、macOS中

## 1. 为什么要进行跨平台操作

首先，我是想在网络编程学习渐入佳境后，自己尝试做一个网络方面的项目，其中就必须用到服务器。Linux服务器相比Windows服务器更加稳定且高效，所以对于我来说，学会如何编写出可以在Linux系统下运行的网络程序是必不可少的。其次，就目前来说，企业中的高性能网络编程都是基于Linux的，学会跨平台的网络编程技能，可以在未来就业方面等有很大的好处。由此，我决定在网络编程学习的第四小阶段，学习如何进行跨平台的网络编程。

## 2. 关于Win与Linux系统下网络编程的差异

- **在Linux环境下，程序的头文件与定义与Win环境下存在差异**
  - Win环境下的特有头文件 <windows.h> 对应Linux环境下的特有头文件 <unistd.h>
  - Win环境下的网络头文件 <winSock2.h> 对应Linux环境下的特有头文件 <arpa/inet.h>
  - SOCKET为Win环境下的特有数据类型，其原型为unsigned __int64，所以我们在Linux下，需要简单对SOCKET进行定义
  - Linux中同样对INVALID_SOCKET与SOCKET_ERROR也没有定义，所以我们参考Win中的定义，在Linux系统下对其定义。
- 在Linux环境下不需要使用**WSAStartup**与**WSACleanup**搭建网络环境，这是Win环境特有的，所以只需要加上#ifdef _WIN32的判断即可，当检测到系统环境为Win时执行
- Linux环境与Win环境下，网络通信相关结构体 **sockaddr_in**和**sockaddr** 存在差异，最明显的差异为存储IP的结构不太一样
- Linux环境与Win环境下，关闭套接字的函数存在差异，Win下为**closesocket()**，Linux下则简单粗暴为**close()**
- Linux环境与Win环境下，服务器的accept连接函数参数存在差异，Win下的最后一个参数为**int**型地址，Linux下则为**socklen_t**型地址，进行一次强制转换即可
- Linux环境与Win环境下，fd_set结构体中的参数出现了变化，不再有储存socket数量的fd_count变量
  - 需要对源码下select函数的第一个参数进行准确的数据传入。select函数的第一个参数实际为**所有socket的最大值+1**，所以我们新建一个变量，用于储存最大值。在每次对fdread集合进行导入时，找到socket的最大值，随后传入select函数即可。
  - 需要对源码下面关于遍历socket的逻辑进行改变。首先遍历 _clients 数组中的所有socket，随后使用FD_ISSET函数判定其是否存在待处理事件，如果有，即可按逻辑进行处理。

## 3. 移植过程中遇到的一些问题

- **关于IP的问题**

  如果服务端在本机Windows环境下运行，客户端在VM虚拟机Linux环境下运行，则在Windows命令行上输入**ipconfig**命令。下面这一块数据下的IPv4地址即为客户端需要连接的IP。

  ```cpp
  以太网适配器 VMware Network Adapter VMnet8:
  
     连接特定的 DNS 后缀 . . . . . . . :
     本地链接 IPv6 地址. . . . . . . . : 
     IPv4 地址 . . . . . . . . . . . . : 
     子网掩码  . . . . . . . . . . . . : 
     默认网关. . . . . . . . . . . . . :
  ```

  如果服务端在VM虚拟机Linux环境下运行，客户端在本机Windows环境下运行，则在Linux命令行上输入**ifconfig**命令。显示出来的数据中网卡的IP即为客户端需要连接的IP。

- **关于端口的问题**

  如果服务端运行正常，客户端运行正常，本机双开客户端和服务端也运行正常，但本机与虚拟机各开一个却连接不上时，可能是服务端的端口未开放导致的。

  - Windows环境下会主动提示，点击允许即可。

  - Linux环境下相关命令如下：

    ```cpp
    systemctl status firewalld 查看防火墙状态
    systemctl start firewalld 开启防火墙 
    systemctl stop firewalld 关闭防火墙 
    service firewalld start 开启防火墙 
    
    查看对外开放的8888端口状态 yes/no
    firewall-cmd --query-port=8888/tcp
    打开8888端口
    firewall-cmd --add-port=8888/tcp --permanent
    重载端口
    firewall-cmd --reload
    移除指定的8888端口：
    firewall-cmd --permanent --remove-port=8888/tcp
    ```

    

# 五、源码的封装

## 1. 为什么要进行封装操作

C++为面向对象编程语言，我们要以面向对象的思路进行源码的编写。在对主要源码进行封装后，客户端与服务端的代码编写更加清晰明了，逻辑性更强，便于开发维护。且在今后的服务端高并发测试中，便于新建多个连接进行测试。
在本部分中，基于第四部分的源码进行封装，并将记录我对客户端与服务端源码进行封装时的思路与步骤。最终源码为客户端封装类文件TcpClient.hpp与服务端封装类文件TcpServer.hpp，以及客户端源码clien.cpp与服务端源码server.cpp。

## 2. 封装的思路

- **客户端的封装**

  客户端的大致流程：

  ```cpp
  1.建立socket
  2.连接服务器
  3.建立新线程 用于发送命令
  while(true)
  {
  	4.使用select函数获取服务器端是否有待处理事件
  	5.如果有，就处理它(接收/发送)
  }
  6.关闭socket
  
  新线程：
  while(1)
  {
  	1.键入数据
  	2.发送数据
  }
  ```

  需要封装的方法：

  ```cpp
  	//初始化socket
  	int InitSocket();
  	//连接服务器 
  	int Connect(const char *ip,unsigned short port);
  	//关闭socket
  	void CloseSocket();
  	//查询是否有待处理消息 
  	bool OnRun();
  	//判断是否工作中 
  	bool IsRun();
  	//发送数据 
  	int SendData(DataHeader *_head);
  	//接收数据
  	int RecvData(SOCKET _temp_socket);
  	//响应数据
  	virtual void NetMsg(DataHeader *_head);
  ```

  按照此思路，客户端的源码思路为：

  ```cpp
  1.InitSocket();//建立socket
  2.Connect(const char *ip,unsigned short port);//连接服务器 传入IP与端口
  3.建立新线程 用于发送命令
  while(4.IsRun())//检测是否工作中
  {
  	5.OnRun();//查询是否有待处理消息
  }
  6.CloseSocket();//关闭socket
  
  新线程：
  while(1.IsRun())//检测是否工作中
  {
  	2.键入数据
  	3.SendData(DataHeader *_head);
  }
  ```

  其中，**OnRun()** 方法中使用的是select网络结构，在select筛选出待处理事件后，使用**RecvData()** 方法进行包头与包体的接收，随后调用**NetMsg()** 方法，依据包头的报文类型对包体数据进行处理。**NetMsg() 方法为虚方法**，在之后调用此封装类时，可以进行继承重载操作，便于对数据响应的操作进行变更。

  另外，由于已经被封装，所以在调用方法时，可能会出现**步骤错误**的情况。例如还没进行新建套接字就进行**connect连接操作或是关闭套接字操作、传入数据有误**等等，此时就会出现问题。
  我解决此类问题的方法是**多加判定**。例如判定套接字是否已经被建立，或是传入数据是否有误等等，随后根据情况进行处理。

- **服务端的封装**

  服务端的大致流程：

  ```cpp
  1.建立socket
  2.绑定端口IP
  3.监听端口
  while(true)
  {
  	4.使用select函数获取存在待监听事件的socket
  	5.如果有新的连接则与新的客户端连接
  	6.如果有待监听事件，则对其进行处理(接受与发送)
  }
  7.关闭socket
  ```

  需要封装的方法：

  ```cpp
  	//初始化socket 
  	int InitSocket();
  	//绑定IP/端口
  	int Bind(const char* ip,unsigned short port);
  	//监听端口
  	int Listen(int n);
  	//接受连接
  	int Accept();
  	//关闭socket 
  	void CloseSocket();
  	//查询是否有待处理消息 
  	bool OnRun();
  	//判断是否工作中 
  	bool IsRun();
  	//发送数据 
  	int SendData(DataHeader *_head,SOCKET _temp_socket);
  	//接收数据
  	int RecvData(SOCKET _temp_socket);
  	//响应数据
  	void NetMsg(DataHeader *_head,SOCKET _temp_socket);
  ```

  按照此思路，服务端的源码思路为：

  ```cpp
  1.InitSocket();//建立socket
  2.Bind(const char* ip,unsigned short port);//绑定端口IP
  3.Listen(int n);//监听端口
  while(4.IsRun())//是否工作中
  {
  	5.OnRun();//查看是否有待处理消息
  }
  6.CloseSocket();//关闭socket
  ```

  其中，**OnRun()** 方法中使用的是select网络结构。在select筛选出待处理事件后，如果为新连接，则使用**Accept()** 方法进行新客户端连接操作；如果为已连接客户端的待接受事件，则使用**RecvData()** 方法进行包头与包体的接收，随后调用**NetMsg()** 方法，依据包头的报文类型对包体数据进行处理。NetMsg() 方法为虚方法，在之后调用此封装类时，可以进行继承重载操作，便于对数据响应的操作进行变更。



# 六、缓冲区溢出与粘包分包

## 1. 关于缓冲区溢出的原因与解决办法

- **缓冲区溢出的原因**

  之前我们所编写的服务端与客户端的数据量都是很小的，且操作也不频繁，需要键入指令发送报文。

  我们可以尝试在之前客户端代码的循环里，不断发送一种数据包，且把数据包的大小加大到1000字节，会发现很快服务端和客户端就会出现问题——要么是数据接收出现问题，要么是服务端或者客户端程序直接卡掉。这里出现问题的原因就是**socket内核缓冲区溢出**。

  首先，send和recv函数并不是直接通过网卡操作。在使用send函数时，**send函数首先把数据写入到发送缓冲区，随后通过网卡发出；在使用recv函数时，网卡首先把接收到的消息写入接收缓冲区，recv函数再从中copy数据**。注意，上文中的两个缓冲区是存在于内核中的，并不是程序中自定义的缓冲区。

  在之前的源码中，recv的逻辑是先接收包头，随后根据包头接收包体。而当网卡接收数据太多时，我们接收一个包头的时间，网卡可能就新接收了两个完整的数据包，这就导致内核接收缓冲区里的数据量是在不断增加的，最终导致接收缓冲区溢出，造成无法正常发送以及程序阻塞的问题。

  举个例子，缓冲区就像一个浴缸，而我们是一个拿盆子舀水的人。我们之前先接收一个包头就相当于舀出一个包头那么多的水，随后再舀出包体那么多的水。舀了两次仅仅舀出一个报文那么多的水。如果浴缸放水的速度比较大的话，我们很容易就会处理不过来。最终造成浴缸溢出(缓冲区溢出)。

- **缓冲区溢出的解决办法**

  首先不大可能改变浴缸的大小，因为太过麻烦以及治标不治本，只要浴缸放水的时间够长，总会溢出。接着，舀水的速度我们也不好改变，因为一时半会是改不了的。那我们就只能改变**舀水的次数和数量**了。

  如何改变舀水的数量和次数？我们可以**一次舀出足够多的水，随后再从舀出的水中分出想要数量的水**，这样浴缸溢出的可能性就大大减少了。

  从代码层面来看上面的思路，只要我们**程序内新建一个足够大的缓冲区，一次从内核缓冲区上recv足够的数据，就可以避免内核缓冲区溢出**了。

  但是这样会出现新的问题，即**粘包与分包**问题。

## 2. 粘包与分包的原因及解决方法

- **粘包与分包的原因**

  一次接收那么多数据，其中数据的界限是没有限定的，比如上文中是想要一次接收4096个字节。假如缓冲区内有5个1000字节大小的数据包，我们这次接收4096字节，等于说接收的数据中有4.096个数据包，其中就包含了新的问题。

  - **粘包问题：**一次接收中含有多个数据包，这就导致数据包界限不清，粘在了一起。像上文中的4.096个包，接收端是不清楚的，接收端只知道有4096字节的数据，但是它不知道一个包是多大。所以我们可以通**过包头来获取一个数据包的大小**，由此来处理相应大小的数据以解决粘包问题。
  - **分包问题：**一次接收中含有不完整的包。例如上文中的4096个字节，其中包含了4个完整的包，和一个包的前96个字节。对此，我们只能处理前4个完整的数据包。那么问题来了，对于上文中的缓冲区，由于recv函数每次都会覆盖这个缓冲区，这就导致缓冲区内无法存放未处理的消息。对于这个问题，我们可以**新建一个缓冲区，来存放未处理的消息，实现双缓冲，**即可处理分包问题。

  TCP是面向数据流的协议，所以会出现粘包分包问题；UDP是面向消息的协议，每一个数据段都是一条消息，所以不会出现粘包分包问题。

- **粘包与分包的解决办法**

  - **客户端升级思路**

    首先是**新建两个缓冲区，一个用来存放recv到的数据，一个用来存放所有待处理数据**。首先第一个缓冲区recv到数据，随后把第一个缓冲区内的数据copy到第二个缓冲区内，即可实现数据的存放。随后处理数据之类的还是先获取包头，随后根据包头处理包体数据。

    ```cpp
    	//接收数据
    	char 接收缓冲区[4096]
    	char 消息缓冲区[40960];
    	int RecvData(SOCKET temp_socket)//处理数据 
    	{ 
    		//接收客户端发送的数据 
    		int recv_len = recv(temp_socket, 接收缓冲区, 4096, 0);
    		if(recv_len <= 0)
    		{
    			printf("与服务器断开连接,任务结束\n");
    			return -1;
    		} 
    		1.将接收缓冲区的数据拷贝到消息缓冲区 
    		while(2.判断消息缓冲区的数据长度是否大于等于包头长度) 
    		{
    			3.选出包头数据 //解决粘包问题
    			if(4.判断消息缓冲区内数据长度是否大于等于报文长度) //解决少包问题 
    			{
    				5.响应数据 
    				6.将处理过的消息移出消息缓冲区
    			} 
    		} 
    		return 0;	
    	}
    ```

  - **服务端升级思路**

    与客户端整体思路相似，但是需要注意，服务端有多个连接，如果多个连接共用一个缓冲区会存在错误，所以每一个客户端连接都需要有自己的缓冲区。对此，我们可以**新建一个客户端连接类，来存放每一个客户端的socket以及它的缓冲区**。

    ```cpp
    class 客户端连接
    {
    public:
    	1.获取socket() 
    	2.获取缓冲区()
    	3.获取缓冲区长度()
    	4.设置缓冲区长度()
    private:	
    	1.socket
    	2.缓冲区
    }; 
    
    std::vector<客户端连接*> _clients;//储存客户端socket
    char 接收缓冲区[4096];
    
    0.此时前面OnRun函数里的判断过程也需要改变
    //遍历所有socket 查看是否有待处理事件 
    for(int n=0; n<_clients.size(); ++n)
    {
    	if(FD_ISSET(_clients[n]->获取socket(),&fdRead))
    	{
    		if(-1 == RecvData(_clients[n]))//处理请求 客户端退出的话 
    		{
    			std::vector<客户端连接*>::iterator iter = _clients.begin()+n;//找到退出客户端的地址
    			if(iter != _clients.end())//如果是合理值
    			{
    				delete _clients[n];
    				_clients.erase(iter);//移除
    			}
    		}
    	}
    }
    
    int RecvData(客户端连接* client)//处理数据 
    { 
    	//接收客户端发送的数据 
    	int recv_len = recv(client->获取socket(), 接收缓冲区, 4096, 0);
    	if(recv_len <= 0)
    	{
    		printf("与服务器断开连接,任务结束\n");
    		return -1;
    	} 
    	1.将接收缓冲区的数据拷贝到传入对象的消息缓冲区 client->获取缓冲区();
    	while(2.判断消息缓冲区的数据长度是否大于等于包头长度) client->获取缓冲区长度();
    	{
    		3.选出包头数据 //解决粘包问题
    		if(4.判断消息缓冲区内数据长度是否大于等于报文长度) //解决少包问题 
    		{
    			5.响应数据 
    			6.将处理过的消息移出消息缓冲区 client->设置缓冲区长度();
    		} 
    	} 
    	return 0;	
    }
    ```

    

# 七、多线程分离业务处理高负载

## 1. 思路与准备

- **客户端多线程改造**

  ![EasyTcpServer/20201218163531660.png at main · AutinC/EasyTcpServer (github.com)](https://github.com/AutinC/EasyTcpServer/blob/main/MySocket/images/20201218163531660.png)

  客户端多线程分组发送消息，尽可能提升发送频率，模拟大规模客户端并发连接，并发数据消息。之前的方法是在**主线程中建立1000个客户端**，然后进行连接和发送消息。现在**创立四个线程，然后将1000个客户端分散到四个线程中**，每个线程处理250个从而减轻压力，并且提升速度。

- **服务端多线程改造**

  之前的服务端思路为：

  ```cpp
  1.建立socket
  2.绑定端口IP
  3.监听端口
  while(true)
  {
  	4.使用select函数获取存在待监听事件的socket
  	5.如果有新的连接则与新的客户端连接
  	6.如果有待监听事件，则对其进行处理(接受与发送)
  }
  7.关闭socket
  ```

  但是，这样的架构在select处理事件较多时，很容易效率低下。对于这类问题，我们可以引入**生产者与消费者模式**，来处理此类并发问题。
  **主线程为生产者线程，用来处理新客户端加入事件，把新客户端分配至消费者线程中**。**消费者线程便是建立的新线程，专门用来处理客户端发送的报文**。这样就实现了事件处理的分离，从而使服务端处理效率更高。当过多客户端同时涌入时，可以更快的建立连接(因为有专门的线程用来处理这一事件)；而当客户端发送报文频率很快时，多线程处理报文也会大大提高效率。

  ![EasyTcpServer/20201218164714164.png at main · AutinC/EasyTcpServer (github.com)](https://github.com/AutinC/EasyTcpServer/blob/main/MySocket/images/20201218164714164.png)

  所以首先需要新建一个**线程类**，用来封装关于**消费者线程**的内容，从而建立多线程架构。随后，在本次的改进中，**加入计时器用来统计数据以及显示数据**，主要需要统计的数据为：**当前客户端连接数量、数据包的每秒接收数量**。同时**对报文类型进行了分离**，把报文类型放在单独的头文件里，这样既方便更改也方便引用。

  ![EasyTcpServer/20201218165300642.png at main · AutinC/EasyTcpServer (github.com)](https://github.com/AutinC/EasyTcpServer/blob/main/MySocket/images/20201218165300642.png)

  **缓冲区作用：**

  ​	生产者把数据放入缓冲区，而消费者从缓冲区取出数据。 生产者和消费者只依赖缓冲区，而不互相	依赖 ，支持并发和异步 。

## 2. 基于chrono库的高精度计时器

C++11中新引入了**std::chrono库**，由此可以较为容易的实现一个计时器。同时，**休眠操作**也可以通过这个库来实现，从而使代码有良好的跨平台性，避免使用Windows/Linux的系统休眠函数，同时引入该计时器可以**实现对每秒收包、连接等数据的计数显示**。

- **简易的计时器类**（CELLTimestamp.hpp)

  ```cpp
  #ifndef CELLTimestamp_hpp_
  #define CELLTimestamp_hpp_
   
  #pragma once
  #include<chrono>
  using namespace std::chrono;
   
  class CELLTimestamp
  {
  public:
  	CELLTimestamp()
  	{
  		update();
  	}
  	~CELLTimestamp()
  	{
   
  	}
   
  	void update()
  	{
  		_begin = high_resolution_clock::now();
  	}
   
  	//获取当前秒
  	double getElapsedSecond()
  	{
  		return this->getElapsedTimeInMicroSec() * 0.000001;
  	}
  	//获取毫秒
  	double getElapsedTimeInMilliSec()
  	{
  		return this->getElapsedTimeInMicroSec() * 0.001;
  	}
  	//获取微秒
  	long long getElapsedTimeInMicroSec()
  	{
  		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
  	}
  protected:
  	time_point<high_resolution_clock> _begin;
  };
   
  #endif
  ```

- **基于chrono库的休眠**

  ```cpp
  	std::chrono::milliseconds t(1);//休眠一毫秒 
  	std::this_thread::sleep_for(t);
  ```

## 3. 多线程的代码实现

- **新建子线程类**（CELLSever.hpp)

  其中包含的基础方法以及相关变量如下：

  ```cpp
  //线程类
  class CellServer
  {
  public:
  	//构造 
  	CellServer(SOCKET sock = INVALID_SOCKET);
  	//析构
  	~CellServer();
  	//关闭socket 
  	void CloseSocket();
  	//判断是否工作中 
  	bool IsRun();
  	//查询是否有待处理消息 
  	bool OnRun();
  	//接收数据
  	int RecvData(ClientSocket *t_client);
  	//响应数据
  	void NetMsg(DataHeader *head,SOCKET temp_socket);
  	//增加客户端 
  	void addClient(ClientSocket* client);
  	//启动线程
  	void Start(); 
  	//获取该线程内客户端数量
  	int GetClientCount()
  	
  private:
  	//缓冲区相关 
  	char *_Recv_buf;//接收缓冲区 
  	//socket相关 
  	SOCKET _sock; 
  	//正式客户队列 
  	std::vector<ClientSocket*> _clients;//储存客户端
  	//客户缓冲区队列
  	std::vector<ClientSocket*> _clientsBuf; 
  	std::mutex _mutex;//锁
  	//线程 
  	std::thread* _pThread;
  
  public:
  	std::atomic_int _recvCount;//接收包的数量 
  };
  ```

  处理思路如下：

  ```cpp
  线程外：
  Start() 首先调用该方法启动线程
  
  新客户端加入：
  GetClientCount() 首先主线程使用这个方法获取各个线程内客户端数量
  //这个添加客户端的方法内涉及到临界区，需要上锁
  addClient(ClientSocket* client) 主线程找到客户端数量最少的线程，使用该线程添加客户端至缓冲队列
  
  线程内：
  OnRun()//运行线程
  {
  	while(IsRun())//判断是否工作中
  	{
  		1.将缓冲队列中的客户数据加入正式队列
  		2.正式客户队列为空的话，continue本次循环
  		3.select选择出待处理事件，错误的话就关闭所有连接CloseSocket()
  		4.对待处理事件进行接收RecvData()，接收包的数量加一，随后处理NetMsg()
  	}
  }
  ```

- **服务端主线程类的更改**（EasyTcpServer.hpp)

  由于处理事件都改为在子线程中，所以主线程中不需要处理报文消息，所以类中接收消息和处理消息的方法都可以删除了。同时我们加入`Start`方法用来启动子线程，加入**time4msg()方法用来显示子线程中的客户端数量、每秒收包数等数据**。

  ```cpp
  class EasyTcpServer
  {
  public:
  	//构造 
  	TcpServer();
  	//析构 
  	~TcpServer();
  	//初始化socket 返回1为正常 
  	int InitSocket();
  	//绑定IP/端口
  	int Bind(const char* ip,unsigned short port);
  	//监听端口
  	int Listen(int n);
  	//接受连接
  	int Accept();
  	//添加客户端至服务端  
  	void AddClientToServer(ClientSocket* pClient);
  	//线程启动 
  	void Start();
  	//关闭socket 
  	void CloseSocket();
  	//判断是否工作中 
  	bool IsRun();
  	//查询是否有待处理消息 
  	bool OnRun();
  	//显示各线程数据信息 
  	void time4msg();
  	
  private:
  	//socket相关 
  	SOCKET _sock; 
  	std::vector<ClientSocket*> _clients;//储存客户端
  	std::vector<CellServer*> _cellServers;//子线程们 
  	//计时器
  	CELLTimestamp _time; 
  };
  ```

  处理思路如下：

  ```cpp
  调用EasyTcpServer封装类建立服务端的流程：
  1.InitSocket() 建立一个socket
  2.Bind(const char* ip,unsigned short port) 绑定端口和IP
  3.Listen(int n) 监听
  4.Start() 线程启动
  while(5.IsRun()) 主线程循环 
  {
  	6.OnRun() 开始select选择处理事件
  }
  7.CloseSocket() 关闭socket
  
  主线程内：
  OnRun()
  {
  	time4msg()显示数据 
  	select选择出新客户端加入事件
  	如果有新客户端加入，调用Accept()接受连接
  	Accept()连接成功后，调用AddClientToServer(ClientSocket* pClient)分配客户端到子线程中
  }
  
  AddClientToServer()内：
  首先调用子线程的GetClientCount()方法获取各条子线程中的客户端数量
  随后调用子线程的addClient(ClientSocket* client)方法，把新客户端添加至客户端最少的线程中
  
  time4msg()内：
  首先GetSecond()获取计时器的计时
  如果大于一秒，就统计客户端的情况：子线程内_recvCount为收包数，主线程内_clients.size()获取客户端数量
  显示后UpDate()重置计时器，并且重置收包数，从而达到统计每秒收包数的作用
  ```

- **引入接口类，实现子线程向主线程通信**（INetEvent.hpp）

  子线程对象是在主线程Start()方法被创建的，随后被加入容器_cellServers储存。这就导致主线程中可以调用子线程类中的方法与成员变量，但是**子线程中却无法调用主线程的方法与成员变量**。从而导致**当子线程中有客户端退出时，主线程无法了解**。对于这种情况，我们可以**创建一个接口，让主线程类继承这个接口，子线程即可通过这个接口调用主线程中的特定方法**。

  ```cpp
  class INetEvent
  {
  public:
  	//有客户端退出 
  	virtual void OnLeave(ClientSocket* pClient) = 0;
  private:	
  };
  ```

  ```cpp
  1.首先是主线程类继承该接口：
  class EasyTcpServer : public INetEvent
  
  2.随后实现接口中的虚方法：
  //客户端退出
  void OnLeave(ClientSocket* pClient)
  {
  	//找到退出的客户端 
  	for(int n=0; n<_clients.size(); n++)
  	{
  		if(_clients[n] == pClient)
  		{
  			auto iter = _clients.begin() + n;
  			if(iter != _clients.end())
  			{
  				_clients.erase(iter);//移除 
  			}
  		}
  	} 
  } 
  即可实现调用该方法，移除客户端容器中指定客户端
  
  3.随后在子线程类中添加私有成员变量： 
  private:
  	INetEvent* _pNetEvent; 
  创建接口对象
  
  4.子线程内创建方法，让接口对象指向主线程类
  void setEventObj(INetEvent* event)
  {
  	_pNetEvent = event; 
  }
  event传进去主线程即可，接口对象即指向主线程
  
  5.主线程创建、启动子线程类时，调用该方法，传入自身this
  子线程对象->setEventObj(this);
  
  6.随后即可通过子线程调用主线程的OnLeave()方法删除客户端
  _pNetEvent->OnLeave(要删除的客户端);
  ```



# 八、对select网络模型进行优化

## 1. 思路与流程

- 获取一到三个fd_set集合，获取一个timeval

- select函数对fd_set集合进行选择筛选

- FD_ISSET函数依据fd_set集合遍历查找待处理事件

  ```cpp
  WINSOCK_API_LINKAGE int WSAAPI select(
  		int nfds,//是指待监听集合里的范围 即待监听数量最大值+1
  		fd_set *readfds,//待监听的可读文件集合 
  		fd_set *writefds,//待监听的可写文件集合  
  		fd_set *exceptfds,//待监听的异常文件集合  
  		const PTIMEVAL timeout);//超时设置 传入NULL为阻塞模式 传入timeval结构体为非阻塞模式
  
  返回值为满足条件的待监听socket数量和，如果出错返回-1，如果超时返回0。
  ```

## 2. 对fd_set的优化

之前，我们每进行一次select操作，都要使用循环把所有的已连接socket放到fd_set集合中，随后进行选择操作。但是当连接数很大、select操作频繁时，不断的新建fd_set并用循环放入socket，很明显会大大增大系统的消耗。
由于fd_set集合中一定存放的是当前所有的socket，由此，我们可以**建立两个fd_set集合与一个bool变量**。**bool变量用来表示socket的组成是否发生了变化**，当有客户端加入或断开时，该变量为true，否则为false。
我们使用一个fd_set集合储存"老的"socket集合，当socket集合没有发生变化时，我们另一个**新fd_set集合直接使用memcpy函数复制老集合中的内容**，从而避免从头循环放入。当socket集合发生变化时，新集合直接循环从头录入，随后老集合使用memcpy函数把新集合内的内容复制过去，方便下一次使用。
这样，我们即可大大减少关于fd_set集合初始化的消耗。

## 3. 对select函数的优化

当前代码只对read可读集合进行操作，并没有write可写集合与except异常集合的操作，所以select目前第三、第四个参数都传了空。这样可以增加一点select的效率。因为select函数被封装了，目前来看应该没法做进一步的优化，可能以后会有吧。

```cpp
select(_sock+1,&fdRead,0,0,&s_t); 
```

## 4. 对FD_ISSET的优化

这是select架构里最消耗资源的部分，当socket连接数很大时，显而易见的是这种**O(N^2)**的查询方法会极大的消耗资源。对此我们可以引入**map**加快查找操作。

```cpp
std::map<socket,Client> _clients;
```

socket当键，客户端对象当值。根据select处理后的fd_set集合内的socket进行查找。效率提高为**O(logN)**。



# 九、消息接收与发送分离

## 1. 思路与流程

在之前第七部分的多线程分离业务处理高负载中，将业务处理相关内容从主线程中分离，大大提高了服务端的效率。随着服务端的进一步完善，把各个模块进行分离，从而使代码的结构更加便于修改与完善，同时也可以提高代码的运行效率。

大致思路如下：**处理事件线程 >> 发送线程缓冲区 >> 发送线程**。

![EasyTcpServer/20210221123454584.png at main · AutinC/EasyTcpServer (github.com)](https://github.com/AutinC/EasyTcpServer/blob/main/MySocket/images/20210221123454584.png)

新建业务子线程时创建一条发送子线程。其中由**主线程创建子线程对象**，子线程对象中包含两条子线程，**一条为业务子线程，一条为发送子线程**。这样两条线程可以在一个对象中进行相关操作，便于实现。同时，**主线程只需声明一个对象即可，封装性良好且低耦合**。
在业务线程需要发送消息时，首先创建一个消息发送对象，其中包含发送的目标和内容。随后把该对象加入缓冲区。在发送线程中，首先把缓冲区内的对象提取到正式发送队列中，随后把正式发送队列中的待发送事件挨个进行处理即可。

## 2. 代码实现

- **发送线程类**(CELLTask.hpp)

  新建两个类，**一个是发送任务基类，一个是发送线程类**。
  发送任务基类中，含有一个**虚方法DoTask()用来执行发送任务**。该方法在服务端源码中被重载。
  发送线程类中，含有一条发送线程，同时有一个Start()方法用来启动该线程。含有两条链表，**一条为缓冲区链表，一条为正式消息链表**，同时有一个**addTask()方法用来把消息任务加入缓冲区**。线程执行**OnRun()方法把缓冲区中的任务加入正式队列并执行该任务的DoTask()方法**。

  ```cpp
  #ifndef _CELL_Task_hpp_
  #define _CELL_Task_hpp_
  
  #include<thread>
  #include<mutex>
  #include<list>
  
  //任务基类
  class CellTask
  {
  public:
  	//执行任务 
  	virtual void DoTask() = 0; 
  };
  
  //发送线程类
  class CellTaskServer
  {
  public:
  	CellTaskServer()
  	{
  		
  	}
  	virtual ~CellTaskServer()
  	{
  		
  	}
  	
  	//添加任务 
  	void addTask(CellTask* ptask)
  	{
  		std::lock_guard<std::mutex>lock(_mutex);
  		_tasksBuf.push_back(ptask);
  	} 
  	//启动服务
  	void Start()
  	{
  		//线程
  		std::thread t(std::mem_fn(&CellTaskServer::OnRun),this); 
  		t.detach();
  	}
  	
  protected:
  	//工作函数 
  	void OnRun()
  	{
  		while(1)
  		{
  			//将缓冲区内数据加入 
  			if(!_tasksBuf.empty())//不为空 
  			{
  				std::lock_guard<std::mutex>lock(_mutex);
  				for(auto pTask : _tasksBuf)
  				{
  					_tasks.push_back(pTask);	
  				} 
  				_tasksBuf.clear();
  			}
  			//如果无任务
  			if(_tasks.empty())
  			{
  				//休息一毫秒 
  				std::chrono::milliseconds t(1);
  				std::this_thread::sleep_for(t);
  				continue;
  			} 
  			//处理任务
  			for(auto pTask:_tasks)
  			{
  				pTask->DoTask();
  				delete pTask;	
  			} 
  			//清空任务 
  			_tasks.clear();
  		}
  	}
  	
  private:
  	//任务数据 
  	std::list<CellTask*>_tasks;
  	//任务数据缓冲区 
  	std::list<CellTask*>_tasksBuf;
  	//锁 锁数据缓冲区 
  	std::mutex _mutex; 
  };
  
  #endif
  ```

  - 其中的缓冲区加入操作涉及到临界操作，所以加个自解锁。
  - 因为缓冲区以及正式队列涉及到频繁进出，所以用的是链表`list`。
  - 当没有发送任务时，会进行一毫秒的休息，防止消耗太多的内存。

- **主文件重写DoTask**(CELLSendMsgTask)

  在导入上述CELLTask.hpp头文件后，需要重载DoTask方法，从而实现把主文件内的相关类型数据进行发送。
  可以创建一个新类，使它继承CELLTask任务基类。其中导入主文件内的相关类型。例如下面的例子中就导入了ClientSocket客户端类与DataHeader报文结构体。同时重写了DoTask方法，使其调用ClientSocket客户端类的SendData方法发送报文。

  ```cpp
  //网络消息发送任务
  class CellSendMsgTask : public CellTask
  {
  public:
  	CellSendMsgTask(ClientSocket* pClient,DataHeader* pHead)
  	{
  		_pClient = pClient;
  		_pHeader = pHead;
  	}
  	
  	//执行任务
  	virtual void DoTask()
  	{
  		_pClient->SendData(_pHeader);
  		delete _pHeader;
  	}  
  	
  private:
  	ClientSocket* _pClient;
  	DataHeader* _pHeader;
  	
  };
  ```

  而想要发送报文时，只需要新建上述`CELLSendMsgTask`对象，并调用`CELLTaskServer`任务线程类的`addTask`方法添加至发送队列即可。

  ```cpp
  //向 pClient 客户端发送 pHead 报文
  void AddSendTask(ClientSocket* pClient,DataHeader* pHead)
  {
  	CellSendMsgTask* ptask = new CellSendMsgTask(pClient,pHead);
  	_taskServer.addTask(ptask);
  }
  ```



# 十、内存池设计与实现

## 1. 内存池设计思路

为什么要使用内存池？我是这样理解的：不断的使用new/malloc从堆中申请内存，会在内存中留下一些碎片。例如我们申请三份8个字节大小的内存A、B、C，由于内存地址是连续的，则ABC的地址值每个相差8(正常情况)。此时我们delete/free掉B内存，A与C内存之间此时就有了8个字节的空白。假如我们今后申请的内存都比8个字节大，则A与C之间这块内存就会一直为空白，这就是内存碎片。
过多的内存碎片会影响程序的内存分配效率，为了降低内存碎片的影响，我们可以引入内存池来尝试解决它。

在程序启动时(或是其他合适的时机)，预先申请足够的、大小相同的内存，把这些内存放在一个容器内。在需要申请内存时，直接从容器中取出一块内存使用；而释放内存时，把这块内存放回容器中即可。这个容器就被称为内存池。而这样操作也可以大大减少内存碎片出现的可能性，提高内存申请/释放的效率。

![EasyTcpServer/20210228213013636.png at main · AutinC/EasyTcpServer (github.com)](https://github.com/AutinC/EasyTcpServer/blob/main/MySocket/images/20210228213013636.png)

需要新建三个类：

- 首先是底层的内存块类，其中包含了该内存块的信息**：内存块编号、引用情况、所属内存池、下一块的位置**等。
- 其次是内存池类，它**对成组的内存块进行管理**，可以实现把内存块从内存池中取出以及把内存块放回内存池。
- 最后是内存管理工具类，其中包含一个或多个内存池，所以它要**根据用户申请的内存大小找到合适的内存池**，调用内存池类的方法申请/释放内存。

需要重载的函数：

- 对`new/delete`进行重载，使其直接调用内存管理工具类申请/释放内存。

上面的工作完成后，虽然仍是以`new/delete`来申请/释放内存，但是已经是通过内存池来实现的了。

## 2. 底层内存块类MemoryBlock的设计与实现

![EasyTcpServer/20210228215549586.png at main · AutinC/EasyTcpServer (github.com)](https://github.com/AutinC/EasyTcpServer/blob/main/MySocket/images/20210228215549586.png)

首先，在内存池中每一块内存是由一个内存头以及其可用内存组成的，其中**内存头**里储存了这块内存的相关信息，**可用内存**即为数据域，类似链表中节点的结构。而一块块内存之间正是一种类似链表的结构，即通过内存头中的一个指针进行连接。内存头中包含的信息大概如下：

- **内存块编号**
- **引用情况**
- **所属内存池**
- **下一块位置**
- **是否在内存池内**

通过上面的思路新建内存块类`MemoryBlock`：

```cpp
由于内存头中要标记所属内存池，所以我们先预声明内存池类，在之后再进行实现。
建立完成后，内存池内一块内存的大小为：sizeof(MemoryBlock) + 可用内存的大小

//预声明内存池类
class MemoryAlloc;
//内存块类
class MemoryBlock
{
public:
	//内存块编号
	int _nID;
	//引用情况
	int _nRef;
	//所属内存池
	MemoryAlloc* _pAlloc;
	//下一块位置
	MemoryBlock* _pNext;
	//是否在内存池内
	bool _bPool;

private:
    
};
```

## 3. 中间件内存池类MemoryAlloc的设计与实现

![EasyTcpServer/20210228222838762.png at main · AutinC/EasyTcpServer (github.com)](https://github.com/AutinC/EasyTcpServer/blob/main/MySocket/images/20210228222838762.png)

由图可知，整个内存池的管理基本为**链表结构**，内存池对象一直指向头部内存单元。在**申请内存时移除头部单元**，类似链表头结点的移除；在**释放内存时，类似链表的头插法，把回收回来的内存单元放在内存池链表的头部**。

- **MemoryAlloc类内成员：**
  - **成员方法**
    - 成员变量初始化 —— 对内存单元可用内存大小以及内存单元数量进行设定
    - 初始化 —— 依据内存单元的大小与数量，对内存池内的内存进行malloc申请，完善每一个内存单元的信息
    - 申请内存 —— 从内存池链表中取出一块可用内存
    - 释放内存 —— 将一块内存放回内存池链表中
  - **成员变量**
    - 内存池地址 —— 指向内存池内的总内存
    - 头部内存单元 —— 指向头部内存单元
    - 内存块大小 —— 内存单元的可用内存大小
    - 内存块数量 —— 内存单元的数量

通过上面的思路新建内存块类`MemoryAlloc`：

```cpp
//导入内存块头文件
#include"MemoryBlock.h"

class MemoryAlloc
{
public:
	MemoryAlloc();
	virtual ~MemoryAlloc();
	//设置初始化
	void setInit(size_t nSize,size_t nBlockSize);//传入的为内存块可用内存大小和内存块数量
	//初始化
	void initMemory();
	//申请内存
	void* allocMem(size_t nSize);//传入的为申请可用内存的大小
	//释放内存
	void freeMem(void* p);

protected:
	//内存池地址
	char* _pBuf;
	//头部内存单元
	MemoryBlock* _pHeader;
	//内存块大小
	size_t _nSize;
	//内存块数量
	size_t _nBlockSize;
	//多线程锁
	std::mutex _mutex;//锁上申请内存方法和释放内存方法即可实现多线程操作
};
```

## 4. 顶层内存管理工具类MemoryMgr的实际与实现

![EasyTcpServer/20210228230825518.png at main · AutinC/EasyTcpServer (github.com)](https://github.com/AutinC/EasyTcpServer/blob/main/MySocket/images/20210228230825518.png)

内存管理工具类用的是**单例模式**，从而能简易的对内存池进行管理。在这次的实现里，使用的是**饿汉式**单例对象。其次，为了更简单的判断出申请内存时所需要调用的内存池，**建立了一个数组映射内存池**。在工具类构造函数内，首先是对内存池进行初始化，随后便是将其映射到数组上。

```cpp
映射：
假如申请一个64字节内存池，申请一个128字节内存池
新建一个指针数组test，使下标0~64指向64字节内存池，下标65~128指向128字节内存池
则我们通过 test[要申请的内存大小] 即可确定合适的内存池
```

在随后的申请过程中，我们首先判断申请内存大小是否超过内存池最大可用内存，若没超过，则通过映射数组指向的内存池进行内存申请；**若超过了，则直接使用malloc申请**，记得多申请一个内存头大小的内存。随后完善内存头内的资料。
在随后的释放过程中，我们**通过内存头判断这块内存是否使属于内存池的内存**，如果是，则通过其所属内存池进行内存回收；若不是，则直接进行free释放。

- **MemoryMgr类内成员：**
  - **成员方法**
    - 饿汉式单例模式 —— 调用返回单例对象
    - 申请内存 —— 调用获取一块内存
    - 释放内存 —— 调用释放一块内存
    - 内存初始化 —— 将内存池映射到数组上
  - 成员变量
    - 映射数组 —— 映射内存池
    - 内存池1
    - 内存池2
    - ......

通过上面的思路新建内存管理工具类`MemoryMgr`：

```cpp
//内存池最大申请
#define MAX_MEMORY_SIZE 128

//导入内存池模板类
#include"MemoryAlloc.h"

class MemoryMgr
{
public:
	//饿汉式单例模式
	static MemoryMgr* Instance();
	//申请内存
	void* allocMem(size_t nSize);
	//释放内存
	void freeMem(void* p);
	
private:
	MemoryMgr();
	virtual ~MemoryMgr();
	//内存映射初始化
	void init_szAlloc(int begin,int end,MemoryAlloc* pMem);

private:
	//映射数组
	MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1];
	//64字节内存池
	MemoryAlloc _mem64;
	//128字节内存池
	MemoryAlloc _mem128;
	//内存池...
};
```

## 5. 重载new/delete函数

```cpp
void* operator new(size_t size);
void operator delete(void* p);
void* operator new[](size_t size);
void operator delete[](void* p);
void* mem_alloc(size_t size);//malloc
void mem_free(void* p);//free
```

## 6. 小结

- 在申请与释放内存时，返回给用户和用户传进来的都是可用内存的地址，并不是内存头的地址。我们需要**对地址进行偏移**，从而返回/接收正确的地址。具体为可用内存地址向前偏移一个内存头大小即为内存头地址；内存头地址向后偏移一个内存头大小即为可用内存地址。
- 内存池初始化时，**申请总地址大小为：(可用地址大小+内存头大小) * 内存单元数量**
  内存池外申请的内存，不会在内存池析构函数内被释放，需要手动释放。（不过一般触发析构函数的时候，也不用手动释放了）



# 十一、对象池的设计与实现

## 1. 对象池的设计思路

- **什么是对象池**

  对象池是一种**空间换时间**的技术，对象被预先创建并初始化后放入对象池中，对象提供者就能利用已有的对象来处理请求，并在不需要时归还给池子而非直接销毁它减少对象频繁创建所占用的内存空间和初始化时间

- **对象池需要具备的四个基本函数**

  重载new/delete函数、创建和销毁对象。

- **对象池的优点**

  - 减少频繁创建和销毁对象带来的成本，实现对象的缓存和复用
  - 提高了获取对象的响应速度，对实时性要求较高的程序有很大帮助

- **对象池的缺点**

  - 很难设定对象池的大小，如果太小则不起作用，过大又会占用内存资源过高
  - 并发环境中, 多个线程可能同时需要获取池中对象, 进而需要在堆数据结构上进行同步或者
    因为锁竞争而产生阻塞, 这种开销要比创建销毁对象的开销高数百倍;
  - 由于池中对象的数量有限, 势必成为一个可伸缩性瓶颈;
  - 所谓的脏对象就是指的是当对象被放回对象池后，还保留着刚刚被客户端调用时生成的数据。
    脏对象可能带来两个问题：
    - 脏对象持有上次使用的引用，导致内存泄漏等问题；
    - 脏对象如果下一次使用时没有做清理，可能影响程序的处理数据。

- **对象池和内存池的差异**。

  - 内存池主要是管理大量的内存分配和释放操作，将频繁的内存分配和释放操作合并为少量的内存池的初始化和释放操作，提高了内存分配效率。它可以避免频繁地进行内存分配，减少了内存碎片的产生，提高了程序的运行效率。内存池适用于那些需要经常分配内存的场合，比如说网络服务器、数据库等。
  - 对象池则是一种特殊的内存池，主要用于管理大量的小对象的分配和释放操作。对象池的优点在于，它可以大幅度减少小对象的内存分配和释放操作，降低了内存碎片的产生，提高了程序的运行效率。在需要频繁地创建和销毁小对象的场合下，对象池的效果更为明显。

各个类的参数种类和数量不确定，该怎么用一个对象池类来创建所有类的对象池呢？答案是使用C++11的**可变参数模板**。

## 2. **对象池类的设计与实现**(CELLObjectPool)

对象池内部的实现和内存池基本一样，都是先申请一大块内存空间，然后再把大内存分成许多小内存，每个小内存对应一个对象，这些小内存以链表的形式进行管理。

![EasyTcpServer/20201231092424374.png at main · AutinC/EasyTcpServer (github.com)](https://github.com/AutinC/EasyTcpServer/blob/main/MySocket/images/20201231092424374.png)

CELLObjectPool类为对象池的具体实现，包含一个内部类NodeHeader，用于存储对象块的信息，如**下一块位置、内存块编号、引用次数和是否在内存池中**等。该类还包含两个主要的方法：allocObjMemory()和freeObjMemory()，分别用于申请对象内存和释放对象内存，内部使用了**互斥锁**来保证线程安全。

- **NodeHeader类：**存储对象块的信息。
- **allocObjMemory()**：申请对象内存。
- **freeObjMemory()**：释放对象内存。
- **initPool()**：初始化对象池。
- **_pHeader**：指向内存块的头节点。
- **_pBuf：**对象池内存缓存区地址。
- **_mutex：**互斥锁，保证线程安全。

## **3. 对象池管理类的设计与实现(ObejectPoolBase)**

内存池的管理主要是对用户提供接口，参数的传递通过可变参的模板类来实现，对new和delete操作进行封装是为了更方便的初始化对象，让使用时更加灵活；要创建不同类型的对象池，只需要继承ObjectPoolBase这个类就可以，当发现该类型对应的对象池没有初始化的时候，就会创建该对象池并且进行初始化操作，之后就可以正常分配对象。

![EasyTcpServer/20201231092545415.png at main · AutinC/EasyTcpServer (github.com)](https://github.com/AutinC/EasyTcpServer/blob/main/MySocket/images/20201231092545415.png)

ObjectPoolBase类为对象池的接口类，提供了重载new和delete操作符的方法，同时也提供了创建和销毁对象的方法。该类还包含一个**静态成员变量objectPool**，用于保存CELLObjectPool对象的实例，以及一个typedef类型ClassTypePool，用于表示CELLObjectPool类的实例。

- **operator new()：**重载new操作符，用于申请对象内存。
- **operator delete()：**重载delete操作符，用于释放对象内存。
- **createObject()：**创建对象的方法。
- **destroyObject()：**销毁对象的方法。
- **objectPool()：**静态CELLObjectPool对象，用于保存CELLObjectPool对象的实例。

## 4. 智能指针与对象池

对象池和内存池可以一起使用，在一起使用的情况下，如果要在对象池中使用智能指针，应该特别注意new操作，因为智能指针使用的是全局的new，内存池使用的是类里面的new，在申请内存时会有差别。

如果想使用智能指针的同时用到对象池，使用shared_ptr<T> obj(new T())的形式构造，这样对shared_ptr的obj走了内存池的构造，而对象T走了对象池的构造，这样可以把指针对象和用户对象分开申请内存。



# 十二、心跳检测与定时收发数据

做以下修改：

- 更改服务端中，客户端对象储存的方式，由**vector改为map**。
- 改变任务队列中任务储存方式，由**任务基类改为匿名函数**。
- 加入**心跳检测**机制，及时剔除未响应客户端。
- 加入**定时发送消息**检测机制，及时发送缓冲区内的内容。

## 1. 更改客户端存储方式

之前服务端程序储存客户端对象ClientSocket的方式是std::**vector**<ClientSocket>，在select筛选后的fd_set中使用**FD_ISSET函数获取需接收报文的客户端**。
但是FD_ISSET函数是使用**for循环**进行暴力检索，消耗较大，我们可以改为使用**std::map::find**进行检索。这样就需要把储存客户端对象的方式改为std::map。因为我们需要通过socket进行查找，所以我把**std::map的键设为SOCKET，值设为客户端对象ClientSocket的指针**，这样我们需要改为std::map<SOCKET,ClientSocket*>。
在改变储存数据结构后，若想获取客户端socket，则调用iter->first；若想获取客户端对象指针，则调用iter->second；获取已连接客户端数量则还是_clients.size()。
在更换数据结构后，通过fdRead.fd_count进行循环，由于linux下fd_set内容与windows下不一致，所以本次要分环境进行检索，代码如下：

```cpp
#ifdef _WIN32
		for (int n = 0; n < (int)fdRead.fd_count; n++)
		{
			auto iter = _clients.find(fdRead.fd_array[n]);
			if (iter != _clients.end())
			{
				if (-1 == RecvData(iter->second))
				{
					if (_pNetEvent)//主线程中删除客户端 
					{
						_pNetEvent->OnNetLeave(iter->second);
					}
					closesocket(iter->first);
					delete iter->second;
					_clients.erase(iter);//移除
					_client_change = true;//客户端退出 需要通知系统重新录入fdset集合 
				}
			}
		}
#else
		std::vector<ClientSocket*> ClientSocket_temp;
		for(auto iter = _clients.begin(); iter != _clients.end(); ++iter)
		{
			if (FD_ISSET(iter->first, &fdRead))
			{
				if (-1 == RecvData(iter->second))//处理请求 客户端退出的话 
				{
					if (_pNetEvent)//主线程中删除客户端 
					{
						_pNetEvent->OnNetLeave(iter->second);
					}
					ClientSocket_temp.push_back(iter->second);
					_clients.erase(iter);//移除
					_client_change = true;//客户端退出 需要通知系统重新录入fdset集合 
				}
			}
		}
		for (auto client : ClientSocket_temp)
		{
			closesocket(client->GetSockfd());
			_clients.erase(client->GetSockfd());
			delete client;
		}
#endif // _WIN32
```

## 2. 更改任务队列存储方式

之前通过声明了一个抽象任务基类，通过重写基类的DoTask()方法来规定如何执行任务。但是这样利用多态可以执行重写后的任务。但是对于每一个新的任务类型，都需要定义一个新类重写一次DoTask()方法，有点麻烦。所以使用C++11中新引入的**匿名函数**，来更改任务队列的储存方式，定义一个匿名函数类型，使任务内容可以更加灵活。

```cpp
//定义
typedef std::function<void()> CellTask;
//任务数据 
std::list<CellTask>_tasks;

//处理任务
for (auto pTask : _tasks)
{
	pTask();	
}

//使用lambda式添加匿名函数
_tasks.push_back([pClient,pHead]() 
{
	pClient->SendData(pHead);
	delete pHead;
});
```

## 3. 加入心跳检测机制

心跳检测的前提是存在一个计时器（见第七部分第2节），通过调用 getNowMillSec 方法，返回当前时间戳。这样通过一个变量来储存上一次获取的时间戳，从而可以计算两次获取时间戳之间的时间差，从而实现计时功能。

在客户端类中定义一个心跳计时变量，并且声明两个相关方法，实现**对心跳计时变量的归零与检测**操作。当心跳计时器超过规定的客户端死亡时间后，`CheckHeart`方法会返回true告知该客户端已死亡。

```cpp
//客户端死亡时间 20000毫秒
#define CLIENT_HREAT_TIME 20000
//心跳计时器
time_t _dtHeart;
//计时变量归零
void ClientSocket::ResetDtHeart()
{
	_dtHeart = 0;
}
//dt为时间差 传入两次检测之间的时间差，检测心跳计时器是否超过心跳检测的阈值
bool ClientSocket::CheckHeart(time_t dt)
{
	_dtHeart += dt;
	if (_dtHeart >= CLIENT_HREAT_TIME)
	{
		printf("CheakHeart dead:%d,time=%lld\n",_sockfd,_dtHeart);
		return true;
	}
	return false;
}
```

接着需要在合适的函数中进行客户端的心跳检测。在子线程的 OnRun 方法中，即对客户端进行 select 操作的方法中，加入 CheckTime 方法，之后对客户端相关的检测操作均在此方法中进行。在 CheckTime 中，我们首先要获取两次 Checktime 之间的时间差，随后**遍历所有客户端对象，挨个使用 CheckHeart 方法进行检测是否超时，若发现超时，则主动断开与该客户端之间的连接**。

```cpp
void CellServer::CheckTime()
{
	//获取时间差
	time_t nowTime = HBtimer::getNowMillSec();
	time_t dt = nowTime - _oldTime;
	_oldTime = nowTime;
	//遍历所有客户端对象
	for (auto iter = _clients.begin(); iter != _clients.end();)
	{
		//检测心跳是否超时
		if (iter->second->CheckHeart(dt) == true)
		{
			if (_pNetEvent)//主线程中删除客户端 
			{
				_pNetEvent->OnNetLeave(iter->second);
			}
			closesocket(iter->second->GetSockfd());
			delete iter->second;
			_clients.erase(iter++);//移除
			_client_change = true;//客户端退出 需要通知系统重新录入fdset集合 
			continue;
		}
		iter++;
	}
}
```

接着是心跳信号，可以在**每次收到客户端报文时都对心跳计时变量归零**，也可以声明单独的心跳报文，当接收到此报文时，重置心跳计时变量。目前采用的方式是前者。

## 4. 加入定时发送缓存消息机制

之前仅进行了客户端消息**定量发送**功能，即当客户端对象发送缓冲区满后，进行消息的发送。这样当消息发送效率不够高时，很容易造成消息反馈的延迟，于是本次也实现了定时发送的功能。
上面实现心跳检测时，已经新建了 CellServer::CheckTime 方法，这个定时发送检测也可以放在这个方法里。思路和心跳检测大同小异，也是在客户端类中定义一个发送计时变量，并且声明两个相关方法，实现**对发送计时变量的归零与检测**操作。
当发现需要发送消息时，需要一个方法把客户端对象发送缓冲区内的内容全部发送，并且清空缓冲区(指针归零)，随后重置计时变量。该方法为 ClientSocket::SendAll。

```cpp
//客户端定时发送时间 200毫秒
#define CLIENT_AUTOMATIC_SEND_TIME 200
//定时发送计时器
time_t _dtSend;
//重置
void ClientSocket::ResetDtSend()
{
	_dtSend = 0;
}
//判断
bool ClientSocket::CheckSend(time_t dt)
{
	_dtSend += dt;
	if (_dtSend >= CLIENT_AUTOMATIC_SEND_TIME)
	{
		//printf("AutomaticSend:%d,time=%lld\n", _sockfd, _dtSend);
		return true;
	}
	return false;
}
//发送缓冲区内全部消息
int ClientSocket::SendAll()
{
	int ret = SOCKET_ERROR;
	if (_Len_Send_buf > 0 && SOCKET_ERROR != _sockfd)
	{
		//发送 
		ret = send(_sockfd, (const char*)_Msg_Send_buf, _Len_Send_buf, 0);
		//发送后缓冲区归零 
		_Len_Send_buf = 0;
		//重置发送计时器
		ResetDtSend();
		//发送错误 
		if (SOCKET_ERROR == ret)
		{
			printf("error 发送失败");
		}
	}
	return ret;
}
//检测
void CellServer::CheckTime()
{
	//获取时间差
	time_t nowTime = HBtimer::getNowMillSec();
	time_t dt = nowTime - _oldTime;
	_oldTime = nowTime;
	//遍历所有客户端对象
	for (auto iter = _clients.begin(); iter != _clients.end();)
	{
		//检测是否到定时发送消息
		if (iter->second->CheckSend(dt) == true)
		{
			iter->second->SendAll();
			iter->second->ResetDtSend();
		}
		iter++;
	}
}
```



# 十三、线程退出安全优化

做以下修改：

- 封装线程类，使其可以控制线程及时关闭
- 使得程序可以按合适的顺序正常退出，避免因退出顺序问题引发崩溃

## 1. 封装线程类

为何要及时使得线程退出？因为**接收、发送线程与主线程是分离的**，当在主线程中析构线程类时，线程中所使用的变量遂被释放，但此时接收、发送线程可能还未从上次循环中结束，仍然在调用已被释放的变量，此时就会出现崩溃等问题。所以需要自己封装一个线程类，来实现对线程退出的控制，使得可以获得线程已经正常退出的信号，从而再安全的释放各种变量。
如何实现？首先新建一个bool类型的信号量，**在主线程发出关闭线程信号后，使用while(1)进行阻塞**。**当线程退出时，更改bool信号量的状态**，当while(1)中检测到信号量发生变化后，则跳出循环解除阻塞，正常向下运行释放变量。伪代码如下：

```cpp
主线程内：
{
	_state = false;//线程是否运行
	while(1)
	{
		if(_semaphore == true)//查看信号量状态
			break;
	}
	释放变量等;
}

子线程内:
{
	while(_state)
	{
		工作;
	}
	printf("线程已退出\n");
	_semaphore = true;
}
```

如上，好处是可以确保线程可以按顺序退出，使得释放变量等操作不会出错。但是坏处是这个while(1)循环会占用大量的系统资源，影响程序效率。以及可能出现信号量未能正确变化，从而陷入死循环。首先是**占用资源太多的问题**，我们可以引用C++11中的**condition_variable条件变量**。

- C++11中的`condition_variable`是一个同步原语，它允许线程在特定条件下等待。它通常与`std::unique_lock`（独占锁）一起使用，以实现线程之间的同步。

  `condition_variable`的主要目的是等待某些事件的发生，这些事件通常由其他线程触发。在等待期间，线程会被阻塞，直到另一个线程通知条件变量，表示该事件已经发生。

  - `condition_variable`的基本用法是：
    - 创建一个`std::condition_variable`对象；
    - 创建一个`std::unique_lock`对象，用于对共享数据进行加锁；
    - 在等待事件的线程中，调用`wait`方法，使线程进入等待状态；
    - 在其他线程中，当某个事件发生时，调用`notify_one`或`notify_all`方法，通知等待的线程；
    - 当线程被通知后，它将再次获取锁并检查条件，如果条件不满足，则返回步骤3，否则继续执行后续操作。

伪代码如下：

```cpp
#include<condition_variable>//条件变量

std::condition_variable _cv;

主线程内：
{
	std::unique_lock<std::mutex> lock(_mutex);//需上锁
	_state = false;//线程是否运行
	_cv.wait(lock);//阻塞等待
	释放变量等;
}

子线程内:
{
	while(_state)
	{
		工作;
	}
	printf("线程已退出\n");
	_cv.notify_one();//唤醒
}
```

如上，阻塞资源消耗太大的问题已经得到了改善，接下来将着重于**如何避免死循环状态**。首先新建信号量类`CellSemaphore`，对信号量操作进行封装，使得线程类内可以直接调用信号量相关操作。结构如下：

```cpp
class CellSemaphore
{
public:
	CellSemaphore();
	~CellSemaphore();
	//开始阻塞
	void wait();
	//唤醒
	void wakeup();
private:
	//等待数
	int _wait = 0;
	//唤醒数
	int _wakeup = 0;
	//条件变量
	std::condition_variable _cv;
	//锁
	std::mutex _mutex;
};
```

如上，CellSemaphore 封装了 wait 和 wakeup 方法，通过调用这两个方法，即可实现阻塞与唤醒。而成员变量方面，声明了一个等待计数器 wait 和一个唤醒计数器 wakeup。每当成功调用 wakeup 方法，都会使等待计数器和唤醒计数器加一；而每当成功调用 wait 方法，都会使等待计数器和唤醒计数器减一。所以正常情况下，一组操作后，两个计数器的值都为0。而由此也可以判断不同的情况，比如当调用wait方法时，唤醒计数器的值大于0，则说明之前已经进行了唤醒操作，则直接跳过阻塞即可。而当调用wakeup方法时，若等待计数器数值不正常，则也直接跳过唤醒操作。相关代码如下：

```cpp
//开始阻塞
void CellSemaphore::wait()
{
	std::unique_lock<std::mutex> lock(_mutex);
	if (--_wait < 0)
	{
		//阻塞开始 等待唤醒
		_cv.wait(lock, [this]()->bool 
		{
			return _wakeup > 0;
		});
		--_wakeup;
	}
}
//唤醒
void CellSemaphore::wakeup()
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (++_wait <= 0)
	{
		++_wakeup;
		_cv.notify_one();
	}
}
```

由此，信号量相关的封装完成了，接下来将进行线程类相关的封装。首先，线程类中需要有一个 CellSemaphore 信号量对象以便于我们对线程的掌握。其次，线程的基础函数：**启动Start()、关闭Close()、退出Exit()、是否运行isRun()**需要存在。接着，是最重要的线程工作函数**OnWork()**。在工作函数OnWork()中，执行三个匿名函数：**_onCreate、_onRun、 _onDestory**，这三个匿名函数分别执行**线程创建时的操作、线程运行时的操作、线程销毁时的操作**。最后还需要一个锁和一个bool变量来保证数据的正常更改以及线程运行状态的判定。线程类 CellThread 结构如下：

```cpp
class CellThread
{
private:
	typedef std::function<void(CellThread*)> EventCall;
public:
	//启动线程
	void Start(EventCall onCreate = nullptr, EventCall onRun = nullptr, EventCall onDestory = nullptr);
	//关闭线程
	void Close();
	//工作中退出
	void Exit();
	//是否运行中
	bool isRun();
protected:
	//工作函数
	void OnWork();
private:
	//三个事件 匿名函数
	EventCall _onCreate;
	EventCall _onRun;
	EventCall _onDestory;
	//改变数据时 需要加锁
	std::mutex _mutex;
	//控制线程的终止与退出
	CellSemaphore _semaphore;
	//线程是否启动
	bool _state = false;
};
```

在启动线程时，需要传入三个匿名函数(默认为空)；当关闭线程时，需要调用`wait()`方法；而当退出线程时，由于一般都是出现错误时调用该方法，所以不需要阻塞，直接停止线程即可。

```cpp
//启动线程
void CellThread::Start(EventCall onCreate, EventCall onRun, EventCall onDestory)
{
	//上锁
	std::lock_guard<std::mutex> lock(_mutex);
	if (!_state)
	{
		//事件赋值
		if (onCreate)
			_onCreate = onCreate;
		if (onRun)
			_onRun = onRun;
		if (onDestory)
			_onDestory = onDestory;
		//线程启动
		_state = true;
		std::thread t(std::mem_fn(&CellThread::OnWork), this);
		t.detach();
	}
}
//关闭线程
void CellThread::Close()
{
	//上锁
	std::lock_guard<std::mutex> lock(_mutex);
	if (_state)
	{
		_state = false;
		_semaphore.wait();
	}
}
//退出线程
void CellThread::Exit()
{
	//上锁
	std::lock_guard<std::mutex> lock(_mutex);
	if (_state)
	{
		_state = false;
		//这里的目的是退出线程 没必要阻塞等信号量
	}
}
//线程是否运行
bool CellThread::isRun()
{
	return _state;
}
```

OnWork 方法内按顺序依次执行三个匿名函数，当销毁阶段函数执行后，调用信号量类的唤醒操作，来告知线程已安全退出。由此，封装线程类相关操作已经完成,可以通过相关方法来更精准的操作线程。

```cpp
void CellThread::OnWork()
{
	//开始事件
	if (_onCreate)
		_onCreate(this);
	//运行
	if (_onRun)
		_onRun(this);
	//销毁
	if (_onDestory)
		_onDestory(this);
	_semaphore.wakeup();
}
```

## 2. 退出顺序

在上文完成线程类的封装后，对源码中的线程相关进行更换。如下，即为线程的创建与关闭操作。

```cpp
//启动接收线程
	_thread.Start(
		//onCreate
			nullptr,
		//onRun
		[this](CellThread*)
		{
			OnRun(&_thread);//工作函数OnRun
		},
		//onDestory
			nullptr);
//关闭接收线程
	_thread.Close();
```

对程序退出的顺序进行规范，大制思路如下：

- 在TcpServer(主线程)宣布程序退出时，首先**对接收线程类进行析构**，进入接收线程的析构函数；
- 在接收线程的析构函数中，首先**对接收线程类中配套的发送线程类进行析构**，进入发送线程的析构函数；
- 在发送线程的析构函数中，**调用发送线程的Close()操作**，在线程关闭后，释放发送线程内的相关变量；
- 在发送线程的析构结束后，**调用接收线程的Close()操作**，在线程关闭后，释放接收线程内的相关变量，随后挨个**释放储存的客户端连接对象**；
- 在释放客户端连接对象时，会进入其析构函数，进行释放相关变量以及关闭socket连接的操作；
- **此时接收线程的析构函数完毕，若还有未释放的接收线程，则重复上述操作**；
- 当子线程全部析构后，回到第一步主线程的退出函数中，此时执行关闭主机socket、清除环境、释放变量等操作。
