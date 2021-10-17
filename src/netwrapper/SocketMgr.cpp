#include "SocketMgr.h"
#include "Client.h"
#include <cassert>
#include "ServerCallback.h"
#include "netwrapper/Utility.h"
#include "ClientMgr.h"
#include "ServerMgr.h"
#include "TimerMgr.h"

namespace hq {

static void timer_callback(int sock, short event, void* arg);

//////////////////////////////////////////////////////////////////////////
SocketMgr::SocketMgr(): event_base_(nullptr), socket_thread_(nullptr), notify_socket_(-1), client_mgr_ptr_(nullptr), timer_mgr_ptr_(nullptr) {
#ifdef WIN32
    WSADATA wsaData;
    /// WSAStartup可以调用多次，与之对应的WSACleanup也要补充调用多次。只有最后一次的WSACleanup才会执行清理操作
    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(ret != 0) {
        printf("WSAStartup failed: %d\n", ret);
    }

    evthread_use_windows_threads();
#else
    evthread_use_pthreads();
#endif
}

SocketMgr::~SocketMgr() {
    stop();

#ifdef WIN32
    WSACleanup();
#endif
}

int SocketMgr::start(const int pool_size/*=0*/) {
    if(nullptr != event_base_) return 0;
    event_base_ = event_base_new();
    if(nullptr == event_base_) return -1;

    LOG4CPLUS_DEBUG(log4cplus::Logger::getInstance("netwrapper"), "begin.");
#ifdef SOCKET_MGR_TEST
    { ///< for test
        int i;
        const char** methods = event_get_supported_methods();
        LOG4CPLUS_DEBUG(log4cplus::Logger::getInstance("netwrapper"), " starting libevent " << event_get_version());
        printf("Starting Libevent %s. Available methods are:\n", event_get_version());
        for(i = 0; methods[i] != NULL; ++i)
        {
            printf("\t%s\n", methods[i]);
            LOG4CPLUS_DEBUG(log4cplus::Logger::getInstance("netwrapper"), "\t " << methods[i]);
        }
    }
#endif

    prepareNotifySocket();
    socket_thread_ = new std::thread([=]() {
        net::setThreadName("netwrapper");
        event_base_dispatch(event_base_);
    });
    if(nullptr == socket_thread_) {
        event_base_free(event_base_);
        event_base_ = nullptr;
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " start failed.");
        return -1;
    }

    LOG4CPLUS_DEBUG(log4cplus::Logger::getInstance("netwrapper"), " start success.");

    client_mgr_ptr_ = new ClientMgr(this->shared_from_this());
    assert(nullptr != client_mgr_ptr_);
    server_mgr_ptr_ = new ServerMgr(this->shared_from_this());
    assert(nullptr != server_mgr_ptr_);
    timer_mgr_ptr_ = new TimerMgr(this->shared_from_this());
    assert(nullptr != timer_mgr_ptr_);

    return 0;
}

int SocketMgr::stop() {
    if(nullptr == event_base_) return 0;

    stopTcpServer(0);

    for(int i = 0; i < TIMER_INFO_COUNT; ++i) {
        stopTimer(i);
    }

    if(nullptr != client_mgr_ptr_) {
        client_mgr_ptr_->stopAllClient();
        delete client_mgr_ptr_;
        client_mgr_ptr_ = nullptr;
    }
    if(nullptr != server_mgr_ptr_) {
        server_mgr_ptr_->stopAllTcpServer();
        delete server_mgr_ptr_;
        server_mgr_ptr_ = nullptr;
    }
    if(nullptr != timer_mgr_ptr_) {
        timer_mgr_ptr_->stopAllTimer();
        delete timer_mgr_ptr_;
        timer_mgr_ptr_ = nullptr;
    }

    event_base_loopbreak(event_base_);
    sendNotify();
    socket_thread_->join();

    if(nullptr != event_base_) {
        event_base_free(event_base_);
        event_base_ = nullptr;
    }
    delete socket_thread_;
    socket_thread_ = nullptr;

    std::lock_guard<std::mutex> lock_guard(notify_socket_mutex_);
    if(notify_socket_ > 0) {
        evutil_closesocket(notify_socket_);
        notify_socket_ = -1;
    }

    LOG4CPLUS_DEBUG(log4cplus::Logger::getInstance("netwrapper"), " stop.");

    return 0;
}

int SocketMgr::prepareNotifySocket() {
    /// 没有特殊需求，可只绑定一个fd至event
    std::lock_guard<std::mutex> mutex_guard(notify_socket_mutex_);
    evutil_socket_t msg_socket_pair[2];
    if(evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, msg_socket_pair) == -1) {
        return -1;
    }

    IClientCallbackPtr client_callback_ptr;
    if(-1 == evutil_make_socket_nonblocking(msg_socket_pair[0])) {
        goto error_flag;
    }
    if(-1 == evutil_make_socket_nonblocking(msg_socket_pair[1])) {
        goto error_flag;
    }

    notify_client_ptr_ = client_mgr_ptr_->startTcpClient(msg_socket_pair[1], nullptr);
    client_callback_ptr = std::dynamic_pointer_cast<IClientCallback>(this->shared_from_this());
    notify_client_ptr_->setCallback(client_callback_ptr);
    notify_socket_ = msg_socket_pair[0];

    return 0;

error_flag:
    evutil_closesocket(msg_socket_pair[0]);
    evutil_closesocket(msg_socket_pair[1]);
    return -1;
}

int SocketMgr::stopClient(const uint64_t client_id) {
    if(nullptr != client_mgr_ptr_) {
        client_mgr_ptr_->stopClient(client_id);
    }
    return -1;
}

IClientPtr SocketMgr::startUdpClient(std::shared_ptr<IClientCallback> callback_ptr) {
    if(nullptr == event_base_ || nullptr == client_mgr_ptr_) return nullptr;
    return client_mgr_ptr_->startUdpClient(callback_ptr);
}

int SocketMgr::stopUdpClient(std::shared_ptr<IClient> client_ptr) {
    if (nullptr == client_ptr || nullptr == client_mgr_ptr_) return -1;

    Client* client = dynamic_cast<Client*>(client_ptr.get());
    if(nullptr != client) {
        return client_mgr_ptr_->stopClient(client->getClientID());
    }
    return 0;
}

IClientPtr SocketMgr::startUdpServer(const HostInfo& host_info, std::shared_ptr<IClientCallback> callback_ptr) {
    if(nullptr == event_base_ || nullptr == client_mgr_ptr_) return nullptr;
    if (!host_info.isValid()) return nullptr;
    return client_mgr_ptr_->startUdpServer(host_info, callback_ptr);
}

int SocketMgr::stopUdpServer(std::shared_ptr<IClient> client_ptr) {
    if (nullptr == client_ptr || nullptr == client_mgr_ptr_) return -1;
    Client* client = dynamic_cast<Client*>(client_ptr.get());
    if(nullptr != client) {
        return client_mgr_ptr_->stopClient(client->getClientID());
    }
    return 0;
}

IClientPtr SocketMgr::startTcpClient(const HostInfo& host_info, std::shared_ptr<IClientCallback> callback_ptr) {
    if(nullptr == event_base_ || nullptr == client_mgr_ptr_) return nullptr;
    return client_mgr_ptr_->startTcpClient(host_info, callback_ptr);
}

int SocketMgr::stopTcpClient(std::shared_ptr<IClient> client_ptr) {
    if (nullptr == client_ptr || nullptr == client_mgr_ptr_) return 0;
    Client* client = dynamic_cast<Client*>(client_ptr.get());
    if(nullptr != client) {
        return client_mgr_ptr_->stopClient(client->getClientID());
    }
    return 0;
}

int SocketMgr::startTcpServer(const HostInfo& host_info, std::shared_ptr<IServerCallback> callback_ptr) {
    if(nullptr == callback_ptr) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " error！no server callback.");
        return -1;
    }
    if(nullptr != server_mgr_ptr_) {
        return server_mgr_ptr_->startTcpServer(host_info, callback_ptr);
    }

    return 0;
}

int SocketMgr::stopTcpServer(const uint64_t id/*=0*/) {
    if(nullptr != server_mgr_ptr_) {
        return server_mgr_ptr_->stopTcpServer(id);
    }

    return 0;
}

int SocketMgr::startTimer(const int seconds, const int useconds, ITimerCallbackPtr callback_ptr) {
    if(nullptr != timer_mgr_ptr_) {
        return timer_mgr_ptr_->startTimer(seconds, useconds, callback_ptr);
    }
    return 1;
}

int SocketMgr::stopTimer(const int timer_id) {
    if(nullptr != timer_mgr_ptr_) {
        return timer_mgr_ptr_->stopTimer(timer_id);
    }
    return 1;
}

int SocketMgr::send(const uint64_t client_id, std::shared_ptr<BufferUtility> buffer_ptr) {
    if(nullptr != client_mgr_ptr_) {
        return client_mgr_ptr_->send(client_id, buffer_ptr);
    }
    return -1;
}

int SocketMgr::send(const uint64_t client_id, HostInfo& host_info, std::shared_ptr<BufferUtility> buffer_ptr) {
    if(nullptr != client_mgr_ptr_) {
        return client_mgr_ptr_->send(client_id, host_info, buffer_ptr);
    }
    return -1;
}

int SocketMgr::sendNotify() {
    char c = 'k';
    if(notify_socket_ > 0) {
        int ret = ::send(notify_socket_, (const char*)&c, 1, 0);
        if(ret < 0) {
            std::lock_guard<std::mutex> lock_guard(notify_socket_mutex_);
            if(notify_socket_ > 0) {
                evutil_closesocket(notify_socket_);
                notify_socket_ = -1;
            }
        }
    }
    return 0;
}

void SocketMgr::handleRead(const HostInfo& remote_host_info, BufferUtilityPtr read_buffer_ptr, std::size_t size) {

}

void SocketMgr::handleSend(const HostInfo& remote_host_info, std::size_t size) {

}

void SocketMgr::handleError(const int errror_code) {
    notify_client_ptr_.reset();
}

ISocketMgrPtr createSocketMgr() {
    return std::shared_ptr<ISocketMgr>(new SocketMgr());
}

}
