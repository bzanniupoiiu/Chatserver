//聊天服务器业务类
#pragma once
#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>

#include <mutex>

using namespace std;
using namespace muduo::net;
using namespace muduo;

#include "usermodel.h"
#include "offlinemessagemodel.h"
#include "friendmodel.h"
#include "groupmodel.h"
#include "redis.h"


#include "json.hpp"
using json = nlohmann::json;


using MsgHandler = std::function<void(const TcpConnectionPtr &conn,json &js,Timestamp)>;


class ChatService
{
public:
    //获取单例对象的接口函数
    static ChatService *instance();

    //处理登录业务
    void login(const TcpConnectionPtr &conn,json &js,Timestamp);
    //处理注册业务
    void reg(const TcpConnectionPtr &conn,json &js,Timestamp);

    //获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    
    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr&conn);
    
    //一对一聊天服务业务：
    void oneChat(const TcpConnectionPtr &conn,json &js,Timestamp);

    //服务器异常退出，在线状态重置业务
    void reset();

    //添加好友业务
    void addFriend(const TcpConnectionPtr &conn,json &js,Timestamp);

    //创建群聊业务
    void createGroup(const TcpConnectionPtr &conn,json &js,Timestamp);

    //加入群聊业务
    void addGroup(const TcpConnectionPtr &conn,json &js,Timestamp);
    

    void GroupChat(const TcpConnectionPtr &conn,json &js,Timestamp);

    void logout(const TcpConnectionPtr &conn,json &js,Timestamp);

    void handleRedisSubscribeMessage(int userid, string msg);


    



private:
    /* data */
    ChatService();

    //存储消息id和其对应的业务处理办法
    unordered_map<int,MsgHandler> _msgHandlerMap;

    //用于存储在线用户的通信连接
    unordered_map<int,TcpConnectionPtr> _userConnMap;

    //定义互斥锁，保证_userConnMap的线程安全
    mutex _connMutex_for_usermap;

    //数据操作类对象
    UserModel _usermodel;

    //离线消息操作类对象
    OfflineMsgModel _offlinemessageModel;

    //添加好友操作类对象
    friendModel _friendModel;

    //群组相关操作类对象
    GroupModel _groupmodel;

    //服务器通信
    Redis _redis;

};



