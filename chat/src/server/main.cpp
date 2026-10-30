#include <server/chatserver.h>
#include <iostream>
#include <server/chatservice.h>
#include <signal.h>

void resetHandler(int){
    chat::ChatService::getInstance().reset();
    exit(0);
}

int main(){

    signal(SIGINT,resetHandler);

    muduo::net::EventLoop eventloop;
    muduo::net::InetAddress serverAddress("127.0.0.1", 8888);
    chat::ChatServer myserver(&eventloop, serverAddress, "chatserver");
    myserver.start();
    eventloop.loop();
}
