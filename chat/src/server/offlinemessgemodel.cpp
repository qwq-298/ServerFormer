#include <server/offlinemessagemodel.h>
#include <server/db/db.h>
#include <logger/log.h>
namespace chat{
    void OfflineMessage::insert(int userid,std::string msg){
        char sql[1024] = {'0'};
        sprintf(sql,
        "insert into offlinemessage(userid,message) values('%d','%s')",
        userid,msg.c_str()
        );
        MySQL mysql;
        if(mysql.connect()){
            LOG_INFO<<"connected"<<std::endl;
            if(mysql.update(sql)){
                return;
            }
        }
    }
    
    void OfflineMessage::remove(int userid){
        char sql[1024] = {'0'};
        sprintf(sql,"delete from offlinemessage where userid = '%d'",userid);
        MySQL mysql;
        if(mysql.connect()){
            mysql.update(sql);
        }
    }
 
    std::vector<std::string> OfflineMessage::query(int userid){
        char sql[1024] = {'0'};
        sprintf(sql,"select message from offlinemessage where userid = '%d'",userid);
        std::vector<std::string> vec;
        MySQL mysql;
        if(mysql.connect()){
            MYSQL_RES* res = mysql.query(sql);
            if(res!=nullptr){
                MYSQL_ROW row;
                while((row = mysql_fetch_row(res))!=nullptr){
                    vec.push_back(row[0]);
                }
                return vec;
            }
            mysql_free_result(res);
        } 
        return vec;
    }


}