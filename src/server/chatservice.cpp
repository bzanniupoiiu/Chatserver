#include "chatservice.h"
#include "public.h"

#include <string>
#include <muduo/base/Logging.h>
#include <vector>
using namespace muduo;
using namespace std;


ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}

ChatService::ChatService()
{
    //其实是注册
    _msgHandlerMap.insert({LOGIN_MSG,std::bind(&ChatService::login,this,_1,_2,_3)});
    _msgHandlerMap.insert({REG_MSG,std::bind(&ChatService::reg,this,_1,_2,_3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG,std::bind(&ChatService::oneChat,this,_1,_2,_3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG,std::bind(&ChatService::addFriend,this,_1,_2,_3)});

    _msgHandlerMap.insert({CREATE_GROUP_MSG,std::bind(&ChatService::createGroup,this,_1,_2,_3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG,std::bind(&ChatService::addGroup,this,_1,_2,_3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG,std::bind(&ChatService::GroupChat,this,_1,_2,_3)});
    _msgHandlerMap.insert({LOGINOUT_MSG,std::bind(&ChatService::logout,this,_1,_2,_3)});

    if(_redis.connect())
    {
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage,this,_1,_2));
    }

}

MsgHandler ChatService::getHandler(int msgid)
{
    auto it = _msgHandlerMap.find(msgid);
    if(it == _msgHandlerMap.end())
    {
        return [=](const TcpConnectionPtr &conn,json &js,Timestamp)
        {
            LOG_ERROR<<"msgid:"<<msgid<<" can not find handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }

}


//处理登录业务 id,pwd
void ChatService::login(const TcpConnectionPtr &conn,json &js,Timestamp)
{
    // LOG_INFO<<" do login service!!!";
    int id = js["id"].get<int>();
    string pwd = js["password"];
    User user = _usermodel.query(id);

    if (user.getId() == id && user.getPwd() == pwd)  
    {
        if(user.getState()=="online")
        {
             //登录失败
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["erron"] = 2;
            response["errmsg"] = "该账号已经登录，请勿重复登录";

            conn->send(response.dump());
            //该用户已经登录，不允许重复登录
        }
        else
        {

            //登录成功，记录用户连接信息
            {
                lock_guard<mutex> lock(_connMutex_for_usermap);//加锁
                _userConnMap.insert({id,conn});
            }//解锁

            //登录成功，订阅对应的消息通道
            _redis.subscribe(id);
            
            //登录成功，更新用户状态信息offline->online;
            user.setState("online");
            _usermodel.updateState(user);
            
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["erron"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            //查询该用户是否有离线消息
            vector<string> vec = _offlinemessageModel.query(id);
            if(!vec.empty())
            {
                response["offlinemsg"] = vec;
                _offlinemessageModel.remove(id);
            }
            //查询该用户的好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if(!userVec.empty()) 
            {
                vector<string> vec2;
                for(User &user:userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            conn->send(response.dump());
        }
       
    }
    else
    {
        //登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["erron"] = 1;
        response["errmsg"] = "用户名或者密码错误";

        conn->send(response.dump());
    }

}
//处理注册业务
void ChatService::reg(const TcpConnectionPtr &conn,json &js,Timestamp)
{
    // LOG_INFO<<" do reg service!!!";

    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool flag = _usermodel.insert(user);
    if(flag)
    {
        //注册成功;
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["id"] = user.getId();
        response["erron"] = 0;

        conn->send(response.dump());
    }
    else
    {
        //注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["erron"] = 1;

        conn->send(response.dump());
    }

}



//客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr&conn)
{
    User user;
    {
        lock_guard<mutex> lock_(_connMutex_for_usermap);
        for(auto it = _userConnMap.begin();it!=_userConnMap.end();it++)
        {
            if(it->second==conn)
            {
                user.setId(it->first);
                //从map表删除用户的连接信息
                _userConnMap.erase(it);
                break;
            }
        }
    }

    // 用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(user.getId()); 
   
    if(user.getId()!=-1)
    {
        //更新用户在线信息
        user.setState("offline");
        _usermodel.updateState(user);
    }
 
    
}


void ChatService::oneChat(const TcpConnectionPtr &conn,json &js,Timestamp)
{
    int toid = js["toid"].get<int>();
    {
        lock_guard<mutex> lock_(_connMutex_for_usermap);
        auto it = _userConnMap.find(toid);
        if(it!=_userConnMap.end())
        {
            it->second->send(js.dump());
            // toid在线，
            return;
        }
    }

     // 查询toid是否在线 
    User user = _usermodel.query(toid);
    if (user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
        // LOG_INFO<<"发布订阅成功";
        return;
    }


    // toid不在线,存储离线消息
    //这个消息存到mysql表里了
    _offlinemessageModel.insert(toid,js.dump()); 

}

void ChatService::reset()
{
    //online->offline
    _usermodel.resetState();

}

void ChatService::addFriend(const TcpConnectionPtr &conn,json &js,Timestamp)
{
    //当前用户id
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    _friendModel.insert(userid,friendid);
    _friendModel.insert(friendid,userid);

    
}



//创建群聊业务
void ChatService::createGroup(const TcpConnectionPtr &conn,json &js,Timestamp)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    Group group(-1,name,desc);
    if(_groupmodel.createGroup(group))
    {
        _groupmodel.addGroup(userid,group.getId(),"creator");
    }

}

//加入群聊业务
void ChatService::addGroup(const TcpConnectionPtr &conn,json &js,Timestamp)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupmodel.addGroup(userid,groupid,"normal");
}


void ChatService::GroupChat(const TcpConnectionPtr &conn,json &js,Timestamp)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    vector<int> useridVec = _groupmodel.queryGroupUsers(userid,groupid);

    lock_guard<mutex> lock(_connMutex_for_usermap);
    for(int id:useridVec)
    {
        auto it = _userConnMap.find(id);
        if(it!=_userConnMap.end())
        {
            it->second->send(js.dump());
        }
        else
        {
            
             
            User user = _usermodel.query(id);
            if (user.getState() == "online")
            {
                _redis.publish(id, js.dump());
                return;
            }
            else
            {
                _offlinemessageModel.insert(id,js.dump());
            }

        }
    }
}


void ChatService::logout(const TcpConnectionPtr &conn,json &js,Timestamp)
{
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex_for_usermap);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }

    // // 用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(userid); 

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    _usermodel.updateState(user);  
}


// 从redis消息队列中获取订阅的消息(redis有消息就触发)
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex_for_usermap);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    _offlinemessageModel.insert(userid, msg);
}