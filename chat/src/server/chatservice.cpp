#include <server/chatservice.h>
#include <server/public.h>
#include <server/user.h>
#include <server/usermodel.h>
#include <server/offlinemessagemodel.h>
#include <server/friendmodel.h>

namespace chat{
    
    ChatService& ChatService::getInstance(){
        static ChatService chatservice;
        return chatservice;
    }

    ChatService::ChatService(){
        _userptr = std::make_shared<UserModel>();
        _offlinemessageptr = std::make_shared<OfflineMessage>();
        _friendptr = std::make_shared<FriendModel>();
        _Groupptr = std::make_shared<GroupModel>();

        msgHandlermap.insert({
            LOGIN_MSG,
            [this](const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timestamp){
                login(conn,js,timestamp);
            }
            }
        );
        msgHandlermap.insert({
            REGISTRE_MSG,
            [this](const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timestamp){
                regis(conn,js,timestamp);
            }
            }
        );
        msgHandlermap.insert({
            ONE_TO_ONE_CHAT_MSG,
            [this](const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timestamp){
                oneChat(conn,js,timestamp);
            }
            }
        );
        msgHandlermap.insert({
            ADD_FRIEND_MSG,
            [this](const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timestamp){
                addfriend(conn,js,timestamp);
            }
        });
        msgHandlermap.insert({
            CREATE_GROUP_MSG,
            [this](const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timestamp){
                createGroup(conn,js,timestamp);
            }
        });
        msgHandlermap.insert({
            ADD_GROUP_MSG,
            [this](const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timestamp){
                addGroup(conn,js,timestamp);
            }
        });
        msgHandlermap.insert({
            GROUP_CHAT_MSG,
            [this](const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timestamp){
                GroupChat(conn,js,timestamp);
            }
        });
    }

    void ChatService::login(const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timestamp){
        std::cout<<"loginto"<<conn->localAddress().toIpPort()<<" "<<js.dump()<<"at"<<timestamp.toString()<<std::endl;
        int id = js["id"];
        std::string pwd = js["password"];
        User user = _userptr->query(id);
        if(user.getId()!=-1 && user.getpassword()==pwd){
            if(user.getstate() == "online"){
            nlohmann::json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            conn->send(response.dump());//重复登录
            }else{
            LOG_INFO << "登录成功" ;
            {
            std::lock_guard<std::mutex> lock(sharedmutex); 
            userConnectionmap.insert({id,conn});
            }
            user.setstate("online");
            _userptr->updatestate(user);
            nlohmann::json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] =user.getName();
            //是否又离线消息
            std::vector<std::string> vec = _offlinemessageptr->query(id);
            if(vec.size()!=0){
                response["offlineMsg"] = vec;
            }
            _offlinemessageptr->remove(id);
            //好友消息
            std::vector<User> uservec = _friendptr->query(id);
            if(!uservec.empty()){
                std::vector<std::string> vec;
                for(std::size_t i=0;i<uservec.size();i++){
                    nlohmann::json js;
                    js["id"] = uservec[i].getId();
                    js["name"] = uservec[i].getName();
                    js["state"]= uservec[i].getstate();
                    vec.push_back(js.dump());
                }
                response["friends"] = vec;
            }

            std::vector<Group> groupvec= _Groupptr->queryGroups(id);
            if(!groupvec.empty()){
                std::vector<std::string> vec;
                for(std::size_t i=0;i<groupvec.size();i++){
                    nlohmann::json js;
                    js["id"] = groupvec[i].getId();
                    js["groupname"] = groupvec[i].getName();
                    js["groupdesc"] = groupvec[i].getDesc();
                    std::vector<GroupUser> vvv = groupvec[i].getUsers();
                    
                    std::vector<std::string> vec1;
                    for(auto groupuser : vvv){
                        nlohmann::json js;
                        js["id"] = groupuser.getId();
                        js["name"] = groupuser.getName();
                        js["state"]= groupuser.getstate();
                        js["role"] = groupuser.getRole();
                        vec1.push_back(js.dump());
                    }

                    js["users"] = vec1;

                    vec.push_back(js.dump());
                }
                response["groups"] = vec;
            }

            conn->send(response.dump());
            }
        } else{
            LOG_INFO << "登录失败" ;
            nlohmann::json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 1;
            conn->send(response.dump());
        }
    }

    void ChatService::regis(const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timestamp){
        std::cout<<"registo"<<conn->localAddress().toIpPort()<<" "<<js.dump()<<"at"<<timestamp.toString()<<std::endl;
        std::string name = js["name"];
        std::string pwd = js["password"];
        User user;
        user.setName(name);
        user.setpassword(pwd);
        bool state = _userptr->insert(user);
        if(state){
            nlohmann::json response;
            response["msgid"] = REGISTER_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            conn->send(response.dump());
        }else{
            nlohmann::json response;
            response["msgid"] = REGISTER_MSG_ACK;
            response["errno"] = 1;
            conn->send(response.dump());
        }
    }

    MsgHandler ChatService::getMsgHandler(int msgid){
        auto it = msgHandlermap.find(msgid);
        if(it==msgHandlermap.end()){
            return [=](const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timestamp){
                std::cout<<"ERROR"<<conn->localAddress().toIpPort()<<" "<<js.dump()<<"at"<<timestamp.toString()<<std::endl;
                LOG_ERROR << "wrong msgid :"<< msgid << std::endl;
            };
        }
        return it->second;
    }

    void ChatService::clientCloseException(const muduo::net::TcpConnectionPtr& conn){
        User user;
         {
         std::lock_guard<std::mutex> lock(sharedmutex);
         for(auto it = userConnectionmap.begin();it!=userConnectionmap.end();it++){
            if(it->second==conn)
            {
               user.setId(it->first);
               userConnectionmap.erase(it);
               break;
            }
         }
         }
         if(user.getId()!=-1){
         user.setstate("offline");
         _userptr->updatestate(user);
         }else{
            LOG_INFO<< "no user log";
         }
    }

    void ChatService::oneChat(const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timestamp){
         int toid = js["to"].get<int>();
         //int id = js["id"].get<int>();
         std::cout<<"onechat"<<conn->localAddress().toIpPort()<<" "<<js.dump()<<"at"<<timestamp.toString()<<std::endl;
         {
            std::lock_guard<std::mutex> lock(sharedmutex);
            auto it = userConnectionmap.find(toid);
            if(it!=userConnectionmap.end()){
                it->second->send(js.dump());
                // conn->send(response) 是否会影响性能
                return;
            }
         }
         _offlinemessageptr->insert(toid,js.dump());
    }
    
    void ChatService::reset(){
        _userptr->reset();
    }

    void ChatService::addfriend(const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timestamp){
        std::cout<<"addfriend"<<conn->localAddress().toIpPort()<<" "<<js.dump()<<"at"<<timestamp.toString()<<std::endl;
        int userid = js["id"].get<int>();
        int friendid = js["friendid"].get<int>();

        _friendptr ->insert(userid,friendid);
    }

    void ChatService::createGroup(const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timestamp){
        std::cout<<"creategroup"<<conn->localAddress().toIpPort()<<" "<<js.dump()<<"at"<<timestamp.toString()<<std::endl;
        int userid = js["id"].get<int>();
        std::string name = js["groupname"];
        std::string desc = js["groupdesc"];

        Group group(-1,name,desc);

        if(_Groupptr->createGroup(group)){
            _Groupptr->addGroup(userid,group.getId(),"creator");
        }

    }

    void ChatService::addGroup(const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timstamp){
        std::cout<<"addgroup"<<conn->localAddress().toIpPort()<<" "<<js.dump()<<"at"<<timstamp.toString()<<std::endl;
        int userid = js["id"].get<int>();
        int groupid = js["groupid"].get<int>();
        _Groupptr->addGroup(userid,groupid,"normal");
    }

    void ChatService::GroupChat(const muduo::net::TcpConnectionPtr&conn,nlohmann::json &js,muduo::Timestamp timstamp){
        std::cout<<"GroupChat"<<conn->localAddress().toIpPort()<<" "<<js.dump()<<"at"<<timstamp.toString()<<std::endl;
        int userid = js["id"].get<int>();
        int groupid = js["groupid"].get<int>();
        std::vector<int> vecid = _Groupptr->queryGroupUsers(userid,groupid);
        std::lock_guard<std::mutex> lock(sharedmutex);
        for(auto id : vecid){
            {
                auto it = userConnectionmap.find(id);
                if(it!=userConnectionmap.end()){
                    it->second->send(js.dump());
                }
                else{
                    _offlinemessageptr->insert(id,js.dump());
                }
            }
        }
    }
}