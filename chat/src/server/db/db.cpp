#include <server/db/db.h>

#include <string>
#include <logger/log.h>

namespace chat{
    MySQL::MySQL(){
        conn = mysql_init(nullptr);
        //mysql_init(conn);
    }
    MySQL::~MySQL(){
        if(conn!=nullptr){
            mysql_close(conn);
        }
    }
    bool MySQL::connect(){
        MYSQL *p = mysql_real_connect(
            conn,server.c_str(),user.c_str(),password.c_str(),dbname.c_str(),3306,nullptr,0
        );
        if(p == nullptr){
            LOG_ERROR << "MySQL connect failed: " << mysql_error(conn) << std::endl;
            return false;
        }
        mysql_query(conn,"set names gbk");
        return true;
    }
    bool MySQL::update(std::string sql){
        if(mysql_query(conn,sql.c_str())){
            LOG_ERROR << __FILE__ << ":" << __LINE__ << ":" 
            << sql << std::endl;
            return false;
        }
        return true;
    }
    MYSQL_RES* MySQL::query(std::string sql){
        if(mysql_query(conn,sql.c_str())){
            LOG_ERROR << __FILE__ << ":" << __LINE__ << ":" 
            << sql << std::endl;
            return nullptr;
        }
        return mysql_use_result(conn);
    }

    MYSQL* MySQL::getConnection(){
        return conn;
    }
}