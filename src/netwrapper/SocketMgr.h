/*
 * @Author: your name
 * @Date: 2021-05-26 19:21:03
 * @LastEditTime: 2021-05-28 19:11:13
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \baselib\src\netwrapper\SocketMgr.h
 */
#pragma once
#ifndef __HQ_NET_WRAPPER_SOCKET_MGR_H__

#include <memory>
#include <thread>
extern "C" {
#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/thread.h>

#ifndef WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#else
#include <Windows.h>
#include <WinSock2.h>
#pragma comment(lib,"Ws2_32.lib ")
#include <Iphlpapi.h>
#pragma  comment(lib, "Iphlpapi.lib")
#include <io.h>
#endif
}
#include <mutex>
#include "netwrapper/ISocketMgr.h"
#include "baselib/BufferUtility.h"

namespace hq {

class SocketMgr;
struct SocketData {
    uint64_t     id;
    evutil_socket_t fd;
    event* normal_event;
    bufferevent* buffer_event;
    SocketMgr* socket_mgr;
    HostInfo     host_info;
    std::weak_ptr<IClient> client_ptr;
    BufferUtilityPtr  read_buff_ptr;

    SocketData() {
        memset(this, 0, offsetof(SocketData, host_info));
        read_buff_ptr = std::make_shared<BufferUtility>();
    }
};

struct ListenerData {
    uint64_t     id;
    struct evconnlistener* tcp_listener;
    SocketMgr* socket_mgr;
    HostInfo     host_info;
    std::shared_ptr<IServerCallback>  server_callback;
};

struct TimerEventInfo {
    int timer_id;
    timeval tv;
    event* timer_event;
    SocketMgr* socket_msg;
    ITimerCallbackPtr timer_callback;

    TimerEventInfo(): timer_id(-1), timer_event(nullptr), socket_msg(nullptr) {
        tv.tv_sec = 0;
        tv.tv_usec = 0;
    }
};

class IClient;
class baselib_declspec SocketMgr: public ISocketMgr{
public:
    SocketMgr();
    virtual ~SocketMgr();

#define TIMER_INFO_COUNT   (30)

    virtual int start(const int pool_size = 0);
    virtual int stop();

    virtual IClientPtr startUdpClient(std::shared_ptr<IClientCallback> callback_ptr);
    virtual int stopUdpClient(std::shared_ptr<IClient> client_ptr);
    virtual IClientPtr startUdpServer(const HostInfo& host_info, std::shared_ptr<IClientCallback> callback_ptr);
    virtual int stopUdpServer(std::shared_ptr<IClient> client_ptr);
    virtual IClientPtr startTcpClient(const HostInfo& host_info, std::shared_ptr<IClientCallback> callback_ptr);
    virtual int stopTcpClient(std::shared_ptr<IClient> client_ptr);
    virtual int startTcpServer(const HostInfo& host_info, std::shared_ptr<IServerCallback> callback_ptr);
    virtual int stopTcpServer(const uint64_t id=0);
    virtual int stopClient(const uint64_t client_id);

public:
    void handleError(SocketData* socket_data, const int error_code);
    void handleAccept(struct evconnlistener* tcp_listener, evutil_socket_t fd, HostInfo &remote_host_info, void* ptr);

    virtual int startTimer(const int seconds, const int useconds, ITimerCallbackPtr callback_ptr);
    virtual int stopTimer(const int timer_id);

protected:
    evutil_socket_t createUdpSocket();
    evutil_socket_t createUdpBindSocket(const HostInfo &host_info);
    int add2socketDataMap(SocketData* socket_data);
    int closeSocketData(SocketData* &socket_data);
    int prepareMsgSocket();
    SocketData *createSocketData(IClientPtr client_ptr, const evutil_socket_t fd, const HostInfo &host_info, bufferevent* bev);

protected:
    struct event_base* event_base_;
    std::thread* socket_thread_;
    evutil_socket_t msg_socket_pair_[2];
    bufferevent* msg_socket_event_pair_[2];
    std::mutex socket_data_map_mutex_;
    std::map<uint64_t, SocketData*>    socket_data_map_;
    std::mutex listener_data_map_mutex_;
    std::map<uint64_t, ListenerData*>  listener_data_map_;
    TimerEventInfo* timer_info_arr_[TIMER_INFO_COUNT];
    std::mutex             timer_arr_mutex_;
};
}

#endif ///< __HQ_NET_WRAPPER_SOCKET_MGR_H__
