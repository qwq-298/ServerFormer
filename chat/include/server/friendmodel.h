#ifndef FRINED_H
#define FRINED_H

#include <vector>
#include <server/user.h>

namespace chat{

    class FriendModel{
        public:

        void insert(int userid,int friendid);
        
        std::vector<User> query(int userid);
                  
    };
}
#endif