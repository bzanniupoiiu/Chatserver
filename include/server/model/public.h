#pragma once
//server和clint的公共文件

enum EnMsgType
{
    LOGIN_MSG =1,//收到这个，会调用void login(const TcpConnectionPtr &conn,json &js,Timestamp);
    LOGIN_MSG_ACK,//登录响应消息
    REG_MSG,  //收到这个，会调用void reg(const TcpConnectionPtr &conn,json &js,Timestamp);
    REG_MSG_ACK, //服务器响应消息
    ONE_CHAT_MSG,//聊天消息 
    /*
        msgid 
        id
        from
        to
        msg_text:
    */
    ADD_FRIEND_MSG,//6

    CREATE_GROUP_MSG,//创建群组消息
    ADD_GROUP_MSG,//加入群组消息
    GROUP_CHAT_MSG,//群聊天消息

    LOGINOUT_MSG,//注销

    
};

