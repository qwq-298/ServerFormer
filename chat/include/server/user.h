#ifndef USER_H
#define USER_H

#include <string>

namespace chat{
    class User{
        public:
        User(int _id=-1,std::string _name="",std::string _password="",std::string _state="offline")
        :id(_id),name(_name),password(_password),state(_state){}

        void setId(int _id){
            id=_id;
        }
        void setName(std::string _name){
            name=_name;
        }
        void setpassword(std::string _password){
            password=_password;
        }
        void setstate(std::string _state){
            state=_state;
        }
        int getId(){
            return id;
        }
        std::string getName(){
            return name;
        }
        std::string getpassword(){
            return password;
        }
        std::string getstate(){
            return state;
        }
        private:
        int id;
        std::string name;
        std::string password;
        std::string state;
    };
}

#endif