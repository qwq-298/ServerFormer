#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <atomic>
#include <cstring>
#include <cstdlib>
#include <limits>
#include <mutex>

#include <unistd.h>
#include <sys/socket.h>
#include <muduo/net/Socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <server/group.h>
#include <server/user.h>
#include <server/public.h>


chat::User g_currentUser;
std::vector<chat::User> g_currentUserFriendList;
std::vector<chat::Group> g_currentUserGroupList;
std::atomic<bool> g_isMainMenuRunning{false};
std::mutex g_coutMutex;

void showCurrentUserData();
void ReadTaskHandler(int clientfd);
std::string getCurrentTime();
bool sendJson(int clientfd, const nlohmann::json& js);
void mainMenu(int clientfd);
void printHelp();
void addFriend(int clientfd);
void oneChat(int clientfd);
void createGroup(int clientfd);
void addGroup(int clientfd);
void groupChat(int clientfd);
void logout(int clientfd);

//main线程发送 子线程接收
int main(int argc,char ** argv){
    if(argc<3){
        std::cerr<< "invalid command" <<std::endl;
        exit(-1);
    }

    char* ip=argv[1];
    uint16_t port = atoi(argv[2]);

    int clientfd = socket(AF_INET,SOCK_STREAM,0);
    
    sockaddr_in server;
    memset(&server,0,sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if(connect(clientfd,(sockaddr *)&server,sizeof(sockaddr_in))==-1){
        close(clientfd);
        exit(-1);
    }

    while(1){
        std::cout<<"========================================="<<std::endl;
        std::cout<<"1.login"<<std::endl;
        std::cout<<"2.register"<<std::endl;
        std::cout<<"3.quit"<<std::endl;
        std::cout<<"========================================="<<std::endl;
        std::cout<<"choice:";
        int choice = 0;
        if(!(std::cin >> choice)){
            if(std::cin.eof()){
                std::cout << "stdin closed, exiting" << std::endl;
                break;
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        switch(choice)
        {
            case 1:
            {
                int id =0 ;
                char pwd[50] ={0};
                std::cout << "userid:";
                std::cin >> id;
                std::cin.get();
                std::cout<<"userpassword:";
                std::cin.getline(pwd,50);
                nlohmann::json js;
                js["msgid"] = LOGIN_MSG;
                js["id"] = id;
                js["password"] = pwd;
                
                int len = send(clientfd,js.dump().c_str(),strlen(js.dump().c_str())+1,0);
                if(len!=-1){
                    char buffer[1024] ={0};
                    len = recv(clientfd,buffer,1024,0);
                    if(len!=-1){
                    nlohmann::json response = nlohmann::json::parse(buffer);
                    if(response["errno"].get<int>()!=0){
                        std::cerr<<"login fail"<<std::endl;
                        break;
                    }
                    g_currentUser.setId(response["id"].get<int>());
                    g_currentUser.setName(response["name"]);

                    g_currentUserFriendList.clear();
                    g_currentUserGroupList.clear();
                    if(response.contains("friends")){
                        std::vector<std::string> vec = response["friends"];
                        for(std::string &str : vec){
                            nlohmann::json js = nlohmann::json::parse(str);
                            chat::User user;
                            user.setId(js["id"].get<int>());
                            user.setName(js["name"]);
                            user.setstate(js["state"]);
                            g_currentUserFriendList.push_back(user);
                        }
                    }
                    if(response.contains("groups")){
                        std::vector<std::string> vec = response["groups"];
                        for(std::string &str : vec){
                            nlohmann::json js = nlohmann::json::parse(str);
                            chat::Group group;
                            group.setId(js["id"].get<int>());
                            group.setName(js["groupname"]);
                            group.setDesc(js["groupdesc"]);
                            
                            std::vector<std::string> vec2 = js["users"];

                            for(std::string& userstr: vec2){
                                chat::GroupUser user;
                                nlohmann::json js = nlohmann::json::parse(userstr);
                                user.setId(js["id"].get<int>());
                                user.setName(js["name"]);
                                user.setstate(js["state"]);
                                user.setRole(js["role"]);
                                group.getUsers().push_back(user);
                            }
                            
                            g_currentUserGroupList.push_back(group);
                        }
                    }
                    showCurrentUserData();

                    if(response.contains("offlineMsg")){
                        std::vector<std::string> vec = response["offlineMsg"];
                        for(std::string &str : vec){
                            nlohmann::json js = nlohmann::json::parse(str);
                            if(js.contains("groupid")){
                                std::cout<< js["time"] << " [group:" << js["groupid"] << "] [" << js["id"] << "] " << js["name"] << " said: " << js["msg"] << std::endl;
                            }else{
                                std::cout<< js["time"] << " [ " <<js["id"] <<" ] " << js["name"] << " said: " <<js["msg"] << std::endl;
                            }
                        }
                    }
                    
                    std::thread readtask(ReadTaskHandler,clientfd);
                    readtask.detach();

                    mainMenu(clientfd);

                    }
                }
            }
            break;
            case 2:
            {
                /* char name[50] = {'0'};
                char pwd[50] = {'0'};
                std::cout<<"usermame:";
                std::cin.getline(name,50);
                std::cout<<"password:";
                std::cin.getline(pwd,50); */
                std::string name;
                std::string pwd;
                std::cout<<"usermame:";
                std::cin>>name;
                std::cout<<"password:";
                std::cin>>pwd;
                if(pwd.empty()){
                    std::cout<<"password empty"<<std::endl;
                    break;
                }
                nlohmann::json js;
                js["msgid"] = REGISTRE_MSG;
                js["name"] = name;
                js["password"] = pwd;

                int len = send(clientfd,js.dump().c_str(),strlen(js.dump().c_str())+1,0);
                if(len!=-1){
                    char buffer[1024] ={0};
                    len = recv(clientfd,buffer,1024,0);
                    if(len!=-1){
                    nlohmann::json response = nlohmann::json::parse(buffer);
                    if(response["errno"].get<int>()!=0){
                    std::cerr<<"register fail"<<std::endl;
                    }else{
                        std::cout<<"register success"<<std::endl;
                    }
                    }
                }
            }
            break;
            case 3:
            {
                close(clientfd);
                exit(0);
            }
            break;
        }
    }

    close(clientfd);
    return 0;
}

void showCurrentUserData(){
    std::cout<<"+++++++++++++++login in+++++++++++++++++"<<std::endl;
    std::cout<<"current user id: "<< g_currentUser.getId() << " name: " <<g_currentUser.getName() <<std::endl;
    std::cout<<"+++++++++++++++friend list++++++++++++++"<<std::endl;
    if(!g_currentUserFriendList.empty()){
        for(chat::User &user : g_currentUserFriendList){
            std::cout<< user.getId() <<" "<<user.getName() << " "<< user.getstate() <<std::endl;
        }
    }
    std::cout<<"+++++++++++++++group list+++++++++++++++"<<std::endl;
    if(!g_currentUserGroupList.empty()){
        for(chat::Group &group : g_currentUserGroupList){
            std::cout<< group.getId() <<" "<<group.getName()<<" "<<group.getDesc()<< std::endl;
            for(chat::GroupUser &groupuser : group.getUsers()){
                std::cout<< groupuser.getId() <<" "<<groupuser.getName()<<" "<<groupuser.getstate() << " "<<groupuser.getRole() <<std::endl;
            }
        }
    }
    std::cout<<"+++++++++++++++++++++++++++++++++++++++++"<<std::endl;
}

bool sendJson(int clientfd, const nlohmann::json& js){
    std::string data = js.dump();
    int len = send(clientfd, data.c_str(), data.size() + 1, 0);
    if(len == -1){
        std::cerr << "send fail" << std::endl;
        return false;
    }
    return true;
}

std::string getCurrentTime(){
    std::time_t t = std::time(nullptr);
    std::tm tm{};
    localtime_r(&t, &tm);
    char buf[64] = {0};
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buf);
}

void ReadTaskHandler(int clientfd){
    while(true){
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        if(len == -1 || len == 0){
            std::lock_guard<std::mutex> lock(g_coutMutex);
            std::cout << "connection closed by server" << std::endl;
            close(clientfd);
            exit(0);
        }

        nlohmann::json js;
        try{
            js = nlohmann::json::parse(buffer);
            int msgid = js["msgid"].get<int>();
            std::lock_guard<std::mutex> lock(g_coutMutex);
            if(msgid == ONE_TO_ONE_CHAT_MSG){
                std::cout << js["time"].get<std::string>() << " [" << js["id"] << "] "
                          << js["name"].get<std::string>() << " said: "
                          << js["msg"].get<std::string>() << std::endl;
            }else if(msgid == GROUP_CHAT_MSG){
                std::cout << js["time"].get<std::string>() << " [group:" << js["groupid"] << "] "
                          << "[" << js["id"] << "] " << js["name"].get<std::string>()
                          << " said: " << js["msg"].get<std::string>() << std::endl;
            }else{
                std::cout << "recv msgid=" << msgid << " raw: " << buffer << std::endl;
            }
        }catch(const std::exception& e){
            std::lock_guard<std::mutex> lock(g_coutMutex);
            std::cerr << "recv parse error: " << e.what() << ", raw: " << buffer << std::endl;
        }
    }
}

void printHelp(){
    std::cout << "============ command list ============" << std::endl;
    std::cout << "0: help" << std::endl;
    std::cout << "1: chat (one-to-one)" << std::endl;
    std::cout << "2: add friend" << std::endl;
    std::cout << "3: create group" << std::endl;
    std::cout << "4: add user to group" << std::endl;
    std::cout << "5: group chat" << std::endl;
    std::cout << "6: logout" << std::endl;
    std::cout << "======================================" << std::endl;
}

void addFriend(int clientfd){
    int friendid = 0;
    std::cout << "input friend id:";
    std::cin >> friendid;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    nlohmann::json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    sendJson(clientfd, js);
    std::cout << "add friend request sent" << std::endl;
}

void oneChat(int clientfd){
    int toid = 0;
    std::cout << "input friend id:";
    std::cin >> toid;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string msg;
    std::cout << "input message:";
    std::getline(std::cin, msg);

    nlohmann::json js;
    js["msgid"] = ONE_TO_ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["to"] = toid;
    js["msg"] = msg;
    js["time"] = getCurrentTime();
    sendJson(clientfd, js);
}

void createGroup(int clientfd){
    std::string groupname, groupdesc;
    std::cout << "input group name:";
    std::getline(std::cin, groupname);
    std::cout << "input group desc:";
    std::getline(std::cin, groupdesc);

    nlohmann::json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    sendJson(clientfd, js);
    std::cout << "create group request sent" << std::endl;
}

void addGroup(int clientfd){
    int groupid = 0;
    int userid = 0;
    std::cout << "input group id:";
    std::cin >> groupid;
    std::cout << "input user id to add:";
    std::cin >> userid;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    nlohmann::json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = userid;
    js["groupid"] = groupid;
    sendJson(clientfd, js);
    std::cout << "add group member request sent" << std::endl;
}

void groupChat(int clientfd){
    int groupid = 0;
    std::cout << "input group id:";
    std::cin >> groupid;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string msg;
    std::cout << "input message:";
    std::getline(std::cin, msg);

    nlohmann::json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = msg;
    js["time"] = getCurrentTime();
    sendJson(clientfd, js);
}

void logout(int clientfd){
    g_isMainMenuRunning = false;
    close(clientfd);
    exit(0);
}

void mainMenu(int clientfd){
    g_isMainMenuRunning = true;
    printHelp();

    while(g_isMainMenuRunning){
        std::cout << "choice:";
        int command = 0;
        if(!(std::cin >> command)){
            if(std::cin.eof()){
                std::cout << "stdin closed" << std::endl;
                break;
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "invalid input" << std::endl;
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch(command){
            case 0:
                printHelp();
                break;
            case 1:
                oneChat(clientfd);
                break;
            case 2:
                addFriend(clientfd);
                break;
            case 3:
                createGroup(clientfd);
                break;
            case 4:
                addGroup(clientfd);
                break;
            case 5:
                groupChat(clientfd);
                break;
            case 6:
                logout(clientfd);
                break;
            default:
                std::cout << "invalid command, input 0 for help" << std::endl;
                break;
        }
    }

}