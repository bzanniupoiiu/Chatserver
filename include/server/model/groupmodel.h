#pragma once

#include "group.h"

class GroupModel
{


public:
    //创建群组
    bool createGroup(Group &group);
    //加入群组
    void addGroup(int userid,int groupid,string role);
    //查询用户所在群组的信息
    vector<Group> queryGroups(int userid);
    //根据指定的groupid查询群组用户的id列表
    vector<int> queryGroupUsers(int userid,int groupid);

private:

};



