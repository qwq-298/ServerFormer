//#include <TcpServer.h>
//#include <EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
class ChatServer
{
    public:
    ChatServer(muduo::net::EventLoop* loop,
    const muduo::net::InetAddress &listenAddr,
    const std::string& name)
    :_tcpserver(loop,listenAddr,name),_loop(loop)
    {
        //_tcpserver.setConnectionCallback(std::bind(&ChatServer::onConnection,this,std::placeholders::_1));
        _tcpserver.setConnectionCallback([this](const muduo::net::TcpConnectionPtr conn){onConnection(conn);});

        _tcpserver.setMessageCallback([this](const muduo::net::TcpConnectionPtr conn, 
            muduo::net::Buffer *buffer,
            muduo::Timestamp time){
                onMessage(conn,buffer,time);
            });
        
        _tcpserver.setThreadNum(4);
    }
    
    void start(){
        _tcpserver.start();
    }


    private:
    //处理用户连接与断开
    void onConnection(const muduo::net::TcpConnectionPtr& connetion){
         if(connetion->connected()){
            std::cout<<connetion->peerAddress().toIpPort()<< "->" <<connetion->localAddress().toIpPort() << "connected" <<std::endl;
         }else {
            std::cout<<connetion->peerAddress().toIpPort()<< "->" <<connetion->localAddress().toIpPort() << "offline" << std::endl;
            connetion->shutdown();
         }
    }
    //处理用户读写
    void onMessage(const muduo::net::TcpConnectionPtr& connection,
    muduo::net::Buffer *buf,
    muduo::Timestamp time)
    {
        std::string message = buf->retrieveAllAsString();
        std::cout<<"Recv data:"<<message<<" time:"<<time.toString()<<std::endl;
        connection->send(message);
    }
    muduo::net::TcpServer _tcpserver;
    muduo::net::EventLoop* _loop;
};

int main(){
    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr("127.0.0.1",8888);
    ChatServer server(&loop,addr,"ChatServer");
    server.start();
    loop.loop();
}