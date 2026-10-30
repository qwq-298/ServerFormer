#include  <nlohmann/json.hpp>

#include <iostream>
#include <vector>
#include <map>

std::string func1(){
    nlohmann::json js;
    js["msg_type"] =2;
    js["from"] ="sde";
    js["to"] ="neu";
    //std::cout<<js<<std::endl;
    return js.dump();
}
std::string func3(){
    nlohmann::json js;
    std::vector<int> vec{1,2,3};
    js["array"] = vec;
    std::map<int,int> mymap{{1,1},{2,2},{3,3}};
    js["map"] = mymap;
    std::string send = js.dump();
    //std::cout<<send<<std::endl;
    return send;
}
using json = nlohmann::json;
int main(){
    json js = json::parse(func3());
    std::cout<<js<<std::endl;
}