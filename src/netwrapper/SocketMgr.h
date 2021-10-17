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
#include "netwrapper/IClientCallback.h"

namespace hq {

class IClient;
class ClientMgr;
class ServerMgr;
class TimerMgr;
class baselib_declspec SocketMgr: public ISocketMgr, public IClientCallback, public std::enable_shared_from_this<SocketMgr> {
public:
    SocketMgr();
    virtual ~SocketMgr();

#define TIMER_INFO_COUNT   (30)

    friend class ClientMgr;
    friend class ServerMgr;
    friend class TimerMgr;

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
    virtual int startTimer(const int seconds, const int useconds, ITimerCallbackPtr callback_ptr);
    virtual int stopTimer(const int timer_id);

public:
    int send(const uint64_t client_id, std::shared_ptr<BufferUtility> buffer_ptr);
    int send(const uint64_t client_id, HostInfo& host_info, std::shared_ptr<BufferUtility> buffer_ptr);

public:
    virtual void handleRead(const HostInfo& remote_host_info, BufferUtilityPtr read_buffer_ptr, std::size_t size) override;
    virtual void handleSend(const HostInfo& remote_host_info, std::size_t size) override;
    virtual void handleError(const int errror_code) override;

protected:
    int prepareNotifySocket();
    int sendNotify();

protected:
    struct event_base* event_base_;
    std::thread* socket_thread_;
    std::mutex        notify_socket_mutex_;
    evutil_socket_t notify_socket_;
    ClientMgr* client_mgr_ptr_;
    ServerMgr* server_mgr_ptr_;
    TimerMgr* timer_mgr_ptr_;
    IClientPtr              notify_client_ptr_;
};

typedef std::shared_ptr<SocketMgr>  SocketMgrPtr;

}

#endif ///< __HQ_NET_WRAPPER_SOCKET_MGR_H__
