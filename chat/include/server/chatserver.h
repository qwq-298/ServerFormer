#ifndef JKIHUWE
#define JKIHUWE

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

namespace chat{
    class ChatServer{
    public:

    ChatServer(
        muduo::net::EventLoop* loop,
        const muduo::net::InetAddress& listenAddr,
        const std::string& nameArg
    );//:myTcpServer(loop,listenAddr,nameArg),
    //eventloop(loop){}
    
    void start();

    private:

    void onConnection(const muduo::net::TcpConnectionPtr& );

    void onMessage(const muduo::net::TcpConnectionPtr& ,
                            muduo::net::Buffer*, 
                            muduo::Timestamp );

    muduo::net::TcpServer myTcpServer;
    muduo::net::EventLoop* eventloop;
    };
}

#endif