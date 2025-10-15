#include "chatserver.h"
#include "chatservice.h"

#include <string>
#include <json.hpp>
#include <functional>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop,
            const InetAddress& listenAddr,
            const string& nameArg):_server(loop,listenAddr,nameArg),_loop(loop)
{
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,_1));
    _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));
    _server.setThreadNum(4);
}
void ChatServer::start()
{
    _server.start();

}



void ChatServer::onConnection(const TcpConnectionPtr& conn)
{
    //如果客户端断开连接
    if(!conn->connected())
    {
        //用户异常退出
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

//上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr& conn,
                            Buffer *buffer,
                            Timestamp time)
{
    string buf = buffer->retrieveAllAsString();

    //数据的反序列化
    json js = json::parse(buf);
    //目的就是完全解耦网络模块的代码和业务模块的代码

    //通过js["msgid"]获取-》业务handler-》
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    //回调消息绑定好的事件处理器，来执行相应的业务
    msgHandler(conn,js,time);

    
}
    

