
#pragma once

//user表的数据操作类

#include<string>
#include "user.h"

using namespace std;

class UserModel
{

public:

    //User表的增加方法
    bool insert(User &user );

    //根据用户号码查询用户信息
    User query(int id);

    bool updateState(User user);

    void resetState();

    

private:
    /* data */


};




