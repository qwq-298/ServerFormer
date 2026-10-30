#ifndef AZMXKS
#define AZMXKS


#include <server/group.h>
#include <vector>
#include <string>

namespace chat{

class GroupModel{

    public:
    bool createGroup(Group &group);
    void addGroup(int userid,int groupid,std::string role);
    std::vector<Group> queryGroups(int userid);
    std::vector<int> queryGroupUsers(int userid,int groupid);
  
};

}
#endif