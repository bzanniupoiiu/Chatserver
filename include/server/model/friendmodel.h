#pragma once
#include "user.h"
#include <vector>
using namespace std;


class friendModel
{
public:
    void insert(int userid,int friendid);

    vector<User> query(int userid);


    


private:
    /* data */
};

