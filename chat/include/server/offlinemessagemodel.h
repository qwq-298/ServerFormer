#ifndef SHJW
#define SHJW

#include <string>
#include <vector>
namespace chat{

    class OfflineMessage{
        public:
        void insert(int userid, std::string msg);

        void remove(int userid);

        std::vector<std::string> query(int userid);
    };
}

#endif