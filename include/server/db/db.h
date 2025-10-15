#pragma once
#include <mysql/mysql.h>
#include <string>
using namespace std;


class MySQL
{

public:
    MySQL();
   
    ~MySQL();
   
    bool connect();
   
    bool update(string sql);
   
    MYSQL_RES* query(string sql);

    MYSQL *getConnection();
    
  


private:
    /* data */
    MYSQL * _conn;
};


