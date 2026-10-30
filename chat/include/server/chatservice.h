#ifndef CHAT_SERVICE_H
#define CHAT_SERVICE_H

#include <unordered_map>
#include <functional>
#include <muduo/net/TcpConnection.h>
#include <nlohmann/json.hpp>
#include <server/usermodel.h>
#include <server/offlinemessagemodel.h>
#include <server/friendmodel.h>
#include <server/groupmodel.h>

#include <mutex>

#include <memory>

#include <logger/log.h>


namespace chat{
using MsgHandler = std::function<void(
    const muduo::net::TcpConnectionPtr& conn,
    nlohmann::json& js,
    muduo::Timestamp timestamp
)>;

using UserModelPtr = std::shared_ptr<UserModel>;
using OfflineMsgModelPtr = std::shared_ptr<OfflineMessage>;
using FriendModelPtr = std::shared_ptr<FriendModel>;
using GroupModelPtr = std::shared_ptr<GroupModel>;

class ChatService
{
public:
    void login(const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timstamp);

    void regis(const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timstamp);

    void oneChat(const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timstamp);

    static ChatService& getInstance();//{
        //static ChatService chatservice;
        //return chatservice;
    //} 
    MsgHandler getMsgHandler(int msgid);

    void reset();

    void addfriend(const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timstamp);

    void clientCloseException(const muduo::net::TcpConnectionPtr& conn);

    void createGroup(const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timstamp);

    void addGroup(const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timstamp);

    void GroupChat(const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timstamp);
    
private:

    ChatService();

    ChatService& operator () (const ChatService&) = delete;
    ChatService(const ChatService&) =delete;
    ChatService(ChatService&&) =delete;
    
    std::unordered_map<int,MsgHandler> msgHandlermap;

    std::unordered_map<int,muduo::net::TcpConnectionPtr> userConnectionmap;

    std::mutex sharedmutex;

    UserModelPtr _userptr;

    OfflineMsgModelPtr _offlinemessageptr;
    
    FriendModelPtr _friendptr;

    GroupModelPtr _Groupptr;
};

}
#endif