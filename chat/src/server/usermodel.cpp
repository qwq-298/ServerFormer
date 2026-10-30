#include <server/usermodel.h>
#include <server/db/db.h>
#include <logger/log.h>
#include <iostream>
namespace chat
{
    bool UserModel::insert(User &user){
        char sql[1024] = {'0'};
        sprintf(sql,
        "insert into user(name,password,state) values('%s','%s','%s')",
        user.getName().c_str(),user.getpassword().c_str(),user.getstate().c_str()
        );
        MySQL mysql;
        if(mysql.connect()){
            LOG_INFO<<"connected"<<std::endl;
            if(mysql.update(sql)){
                LOG_INFO<<"updated"<<std::endl;
                user.setId(mysql_insert_id(mysql.getConnection()));
                return true;
            }
        }
        return false;
    }

    User UserModel::query(int id){
        char sql[1024] = {'0'};
        sprintf(sql,
        "select * from user where id = '%d'",id
        );
        MySQL mysql;
        User user;
        if(mysql.connect()){
            MYSQL_RES *res = mysql.query(sql);
            if(res!=nullptr){
                LOG_INFO << "getres";
                MYSQL_ROW row = mysql_fetch_row(res);
                if(row!=nullptr){
                    user.setId(atoi(row[0]));
                    user.setName(row[1]);
                    user.setpassword(row[2]);
                    user.setstate(row[3]);
                    mysql_free_result(res);
                    return user;
                }
                mysql_free_result(res);
            }
        }
        return user;
    }

    bool UserModel::updatestate(User &user){
        char sql[1024] = {'0'};
        sprintf(sql,
        "update user set state = '%s' where id = %d",
        user.getstate().c_str(),user.getId()
        );
        MySQL mysql;
        if(mysql.connect()){
            if(mysql.update(sql)){
                return true;
            }
        }
        return false;
    }

    void UserModel::reset(){
        char sql[1024] = {'0'};
        sprintf(sql,
        "update user set state = 'offline' where state = 'online'"
        );
        MySQL mysql;
        if(mysql.connect()){
            mysql.update(sql);
        }
    }
} 
