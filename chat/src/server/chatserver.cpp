#include <server/chatserver.h>
#include <server/chatservice.h>
#include <iostream>
#include <logger/log.h>
#include <nlohmann/json.hpp>

namespace chat{

    ChatServer::ChatServer(
        muduo::net::EventLoop* loop,
        const muduo::net::InetAddress& listenAddr,
        const std::string& nameArg
    ):myTcpServer(loop,listenAddr,nameArg),
    eventloop(loop){
        myTcpServer.setConnectionCallback(
            [this](const muduo::net::TcpConnectionPtr& conn){
                 onConnection(conn);
            }
        );

        myTcpServer.setMessageCallback(
            [this](
                const muduo::net::TcpConnectionPtr& conn,
                            muduo::net::Buffer* buf,
                            muduo::Timestamp timestamp
            ){
                 onMessage(conn,buf,timestamp);   
            }
        );

        myTcpServer.setThreadNum(4);
    }
    
    void ChatServer::start(){
        myTcpServer.start();
    }

    void ChatServer::onConnection(const muduo::net::TcpConnectionPtr& conn){
         if(!conn->connected()){
            LOG_INFO << "disconnected!";
            ChatService::getInstance().clientCloseException(conn);
         }
    };

    void ChatServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
                            muduo::net::Buffer* buf,
                            muduo::Timestamp timestamp)
    {
          std::string buffer = buf->retrieveAllAsString();

          try{
              nlohmann::json js = nlohmann::json::parse(buffer);

              //回调 业务处理函数
              auto msgHandler = ChatService::getInstance().getMsgHandler(/*js["msgid"].get<int>()*/static_cast<int>(js["msgid"]));

              msgHandler(conn,js,timestamp);

              std::cout<<"Recv data:"<<buffer <<" time:" << timestamp.toString()<<std::endl;
              //conn->send(buffer);
          }catch(const std::exception& e){
              LOG_ERROR << "onMessage exception: " << e.what() << std::endl;
          }
    };

}