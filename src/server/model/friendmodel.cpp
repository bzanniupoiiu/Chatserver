#include "friendmodel.h"

#include "db.h"


void friendModel::insert(int userid,int friendid)
{
    char sql[1024] = {0};
    sprintf(sql,"insert into Friend values(%d,%d)",userid,friendid);
    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
       
    }
}

vector<User> friendModel::query(int userid)
{
     //1组装sql语句
    char sql[1024] = {0};
    //friend和user的联合查询
    sprintf(sql,"select a.id,a.name,a.state from User a inner join Friend b on b.friendid = a.id where b.userid = %d",userid);
    
    vector<User> vec;
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res!=nullptr)
        {
            //查询成功
            MYSQL_ROW row ;

            while((row = mysql_fetch_row(res))!=nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            
            mysql_free_result(res);//手动释放资源，防止内存泄漏
        }
    
    }
    return vec;
    
}


    
