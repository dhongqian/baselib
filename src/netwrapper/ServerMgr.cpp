#include "SocketMgr.h"
#include "ServerMgr.h"
#include "netwrapper/Utility.h"
#include "Client.h"
#include "ClientMgr.h"
#include "netwrapper/IServerCallback.h"

namespace hq {

static void listener_callback(struct evconnlistener*, evutil_socket_t, struct sockaddr*, int socklen, void*);

ServerMgr::ServerMgr(std::shared_ptr<SocketMgr> socket_mgr_ptr) {
    socket_mgr_ptr_ = socket_mgr_ptr;
}

ServerMgr::~ServerMgr() {
}

int ServerMgr::startTcpServer(const HostInfo& host_info, std::shared_ptr<IServerCallback> callback_ptr) {
    auto socket_mgr_ptr = socket_mgr_ptr_.lock();
    assert(nullptr != socket_mgr_ptr);
    struct sockaddr_in sin = net::getSockAddrFromHostInfo(host_info);

    ListenerData* listener_data = new ListenerData();
    if(nullptr == listener_data) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " new listener data failed.");
        return -1;
    }
    //创建、绑定、监听socket
    listener_data->tcp_listener = evconnlistener_new_bind(socket_mgr_ptr->event_base_, listener_callback, (void*)listener_data,
                                                          LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
                                                          (struct ::sockaddr*) & sin,
                                                          sizeof(sin));

    if(nullptr == listener_data->tcp_listener) {
        delete listener_data;
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " create listener socket failed.");
        return 1;
    }

    listener_data->id = (uint64_t)listener_data->tcp_listener;
    listener_data->server_mgr = this;
    //listener_data->event_base = socket_base_;
    listener_data->host_info = host_info;
    listener_data->server_callback = callback_ptr;

    std::lock_guard<std::mutex> mutex_guard(listener_data_map_mutex_);
    listener_data_map_[listener_data->id] = listener_data;

    return 0;
}

int ServerMgr::stopTcpServer(const uint64_t id/*=0*/) {
    if(id != 0) {
        std::lock_guard<std::mutex> mutex_guard(listener_data_map_mutex_);
        auto it = listener_data_map_.find(id);
        if(it == listener_data_map_.end()) {
            return 0;
        }

        ListenerData* listener_data = it->second;
        evconnlistener_free(listener_data->tcp_listener);
        delete listener_data;
        listener_data_map_.erase(it);
        return 0;
    }
    return 0;
}

int ServerMgr::stopAllTcpServer() {
    std::lock_guard<std::mutex> mutex_guard(listener_data_map_mutex_);
    for(auto item : listener_data_map_) {
        ListenerData* listner_data = item.second;
        evconnlistener_free(listner_data->tcp_listener);
        delete listner_data;
    }
    listener_data_map_.clear();
    return 0;
}

void ServerMgr::handleAccept(struct evconnlistener* tcp_listener, evutil_socket_t fd, HostInfo& remote_host_info, void* ptr) {
    auto socket_mgr_ptr = socket_mgr_ptr_.lock();
    assert(nullptr != socket_mgr_ptr);
    auto client_mgr_ptr = socket_mgr_ptr->client_mgr_ptr_;
    uint64_t id = (uint64_t)tcp_listener;
    ListenerData* listener_data = (ListenerData*)ptr;
    assert(id == listener_data->id);

    if(nullptr == client_mgr_ptr) {
        evutil_closesocket(fd);
        return;
    }

    {
        std::lock_guard<std::mutex> mutex_guard(listener_data_map_mutex_);
        auto it = listener_data_map_.find(id);
        if(listener_data_map_.end() == it || nullptr == listener_data->server_callback) {
            evutil_closesocket(fd);
            return;
        }
    }

    IClientPtr client_ptr = client_mgr_ptr->startTcpClient(fd, nullptr);
    if(nullptr == client_ptr) {
        evutil_closesocket(fd);
        return;
    }
    listener_data->server_callback->handleAcceptor(client_ptr);
}

////////////////////////////////////////////////////////////////////
void listener_callback(struct evconnlistener* listener, evutil_socket_t fd, struct ::sockaddr* addr, int len, void* ptr) {
    if(nullptr == ptr || nullptr == addr) return;
    struct sockaddr_in* addr_v4 = (struct sockaddr_in*)addr;
    HostInfo host_info(addr_v4->sin_addr.s_addr, addr_v4->sin_port);
    struct ListenerData* listener_data = (struct ListenerData*)ptr;
    if(nullptr != listener_data->server_mgr) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " accept a new client [" << host_info.getIPString() << ":" << host_info.getPort() << "].");
        listener_data->server_mgr->handleAccept(listener, fd, host_info, ptr);
    }
    else {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " accept [" << host_info.getIPString() << ":" << host_info.getPort() << "] failed.");
    }
}

}
