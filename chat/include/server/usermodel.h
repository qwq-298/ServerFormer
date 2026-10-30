#ifndef USERMOODEL_H
#define USERMOODEL_H

#include <server/user.h>
namespace chat{
class UserModel{
    public:
    bool insert(User &user);
    User query(int id);
    bool updatestate(User &user);
    void reset();
};
}
#endif