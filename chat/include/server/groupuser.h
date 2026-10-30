#ifndef TYUIO
#define TYUIO

#include <server/user.h>

namespace chat{

class GroupUser : public User{
    public:

    void setRole(std::string role){
        this->role = role;
    }

    std::string getRole(){
        return role;
    }
    private:
    std::string role;
};

}



#endif