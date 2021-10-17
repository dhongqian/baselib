#include "SocketMgr.h"
#include "ClientMgr.h"
#include "netwrapper/Utility.h"
#include "Client.h"

namespace hq {

static void read_callback(struct bufferevent* bev, void* arg);
static void write_callback(struct bufferevent* bev, void* arg);
static void tcp_event_callback(struct bufferevent* bev, short events, void* arg);
static void udp_event_callback(evutil_socket_t fd, short events, void* param);


///////////////////////////////////////////////////////////////////////////
///////////   SocketData   ///////////////////////////////////////////////

void SocketData::reset() {
    id = 0;
    if(fd > 0) {
        evutil_closesocket(fd);
    }
    if(nullptr != normal_event) {
        event_free(normal_event);
    }
    if(nullptr != buffer_event) {
        bufferevent_free(buffer_event);
    }
    memset(this, 0, offsetof(SocketData, host_info));
    client_ptr.reset();
}

//////////////////////////////////////////////////////////////////////////

ClientMgr::ClientMgr(std::shared_ptr<SocketMgr> socket_mgr_ptr) {
    socket_mgr_ptr_ = socket_mgr_ptr;
}

ClientMgr::~ClientMgr() {
}

IClientPtr ClientMgr::startUdpClient(std::shared_ptr<IClientCallback> callback_ptr) {
    auto socket_mgr_ptr = socket_mgr_ptr_.lock();
    assert(nullptr != socket_mgr_ptr);
    int ret = 0;
    IClientPtr client_ptr;
    SocketData* socket_data = nullptr;
    evutil_socket_t fd = net::createNoBlockingSocket(SOCK_DGRAM);
    if(fd < 0) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " create socket failed.");
        goto error_failed_4;
    }

    socket_data = new(std::nothrow) SocketData();
    if(nullptr == socket_data) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " new socket data failed.");
        goto error_failed_3;
    }
    socket_data->id = (uint64_t)socket_data;
    socket_data->client_mgr = this;
    socket_data->fd = fd;

    client_ptr = std::make_shared<Client>(socket_mgr_ptr, socket_data->id, fd);
    if(nullptr == client_ptr) {
        goto error_failed_2;
    }
    socket_data->client_ptr = client_ptr;

    socket_data->normal_event = event_new(socket_mgr_ptr->event_base_, fd,
                                          EV_READ | EV_PERSIST,
                                          udp_event_callback, socket_data);
    if(nullptr == socket_data->normal_event) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " create event failed.");
        goto error_failed_1;
    }
    event_add(socket_data->normal_event, NULL);

    ret = add2socketDataMap(socket_data);
    if(ret < 0) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " add to socket data map failed.");
        goto error_failed_1;
    }

    client_ptr->setCallback(callback_ptr);
    socket_mgr_ptr->sendNotify();
    return client_ptr;

error_failed_1:
    event_free(socket_data->normal_event);
error_failed_2:
    delete socket_data;
error_failed_3:
    evutil_closesocket(fd);
error_failed_4:
    return nullptr;
}

IClientPtr ClientMgr::startUdpServer(const HostInfo& host_info, std::shared_ptr<IClientCallback> callback_ptr) {
    auto socket_mgr_ptr = socket_mgr_ptr_.lock();
    assert(nullptr != socket_mgr_ptr);
    evutil_socket_t fd = net::createBindNoBlockingSocket(host_info, SOCK_DGRAM);
    if(fd < 0) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " create bind socket failed.");
        return nullptr;
    }

    SocketData* socket_data = new(std::nothrow) SocketData();
    if(nullptr == socket_data) {
        evutil_closesocket(fd);
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " new socket data failed.");
        return nullptr;
    }
    socket_data->id = (uint64_t)socket_data;
    socket_data->client_mgr = this;
    socket_data->host_info = host_info;
    socket_data->fd = fd;

    auto client_ptr = std::make_shared<Client>(socket_mgr_ptr, socket_data->id, fd);
    if(nullptr == client_ptr) {
        evutil_closesocket(fd);
        delete socket_data;
        return nullptr;
    }
    socket_data->client_ptr = client_ptr;

    int ret = 0;
    socket_data->normal_event = event_new(socket_mgr_ptr->event_base_, fd,
                                          EV_READ | EV_PERSIST,
                                          udp_event_callback, socket_data);
    if(nullptr == socket_data->normal_event) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " create event failed.");
        goto start_failed;
    }
    ret = event_add(socket_data->normal_event, NULL);
    ret = add2socketDataMap(socket_data);
    if(ret < 0) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " add to socket data map failed.");
        goto start_failed;
    }

    client_ptr->setCallback(callback_ptr);
    socket_mgr_ptr->sendNotify();
    return client_ptr;

start_failed:
    event_free(socket_data->normal_event);
    delete socket_data;
    evutil_closesocket(fd);
    return nullptr;
}

IClientPtr ClientMgr::startTcpClient(const HostInfo& host_info, std::shared_ptr<IClientCallback> callback_ptr) {
    auto socket_mgr_ptr = socket_mgr_ptr_.lock();
    assert(nullptr != socket_mgr_ptr);
    struct sockaddr_in sin = net::getSockAddrFromHostInfo(host_info);
    struct bufferevent* bev = bufferevent_socket_new(socket_mgr_ptr->event_base_, -1, BEV_OPT_CLOSE_ON_FREE);
    if(bev == NULL) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " buffer event new failed.");
        return nullptr;
    }

    //连接服务端
    int flag = bufferevent_socket_connect(bev, (struct ::sockaddr*) & sin, sizeof(sin));
    if(-1 == flag) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " buffer event connect failed.");
        bufferevent_free(bev);
        return nullptr;
    }
    evutil_socket_t fd = bufferevent_getfd(bev);
    int ret = bufferevent_enable(bev, EV_READ | EV_WRITE);
    if(ret < 0) {
        bufferevent_free(bev);
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " buffer event enable failed.");
        return nullptr;
    }

    SocketData* socket_data = new(std::nothrow) SocketData();
    if(nullptr == socket_data) {
        bufferevent_free(bev);
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " new socket data failed.");
        return nullptr;
    }
    socket_data->id = (uint64_t)socket_data;
    socket_data->client_mgr = this;
    socket_data->host_info = host_info;
    socket_data->buffer_event = bev;

    auto client_ptr = std::make_shared<Client>(socket_mgr_ptr, socket_data->id, fd);
    if(nullptr == client_ptr) {
        bufferevent_free(bev);
        delete socket_data;
        return nullptr;
    }
    socket_data->client_ptr = client_ptr;

    //绑定读事件回调函数、写事件回调函数、错误事件回调函数
    bufferevent_setcb(socket_data->buffer_event, read_callback, write_callback, tcp_event_callback, socket_data);

    //bufferevent_enable(socket_data->buffer_event, EV_WRITE);
    //bufferevent_enable(socket_data->buffer_event, EV_READ);
    ret = add2socketDataMap(socket_data);
    if(ret < 0) {
        bufferevent_free(bev);
        delete socket_data;
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " add to socket data map failed.");
        return nullptr;
    }

    client_ptr->setCallback(callback_ptr);
    socket_mgr_ptr->sendNotify();
    return client_ptr;
}

IClientPtr ClientMgr::startTcpClient(evutil_socket_t fd, std::shared_ptr<IClientCallback> callback_ptr) {
    int ret = 0;
    IClientPtr client_ptr;
    auto socket_mgr_ptr = socket_mgr_ptr_.lock();
    assert(nullptr != socket_mgr_ptr);
    SocketData* socket_data = new(std::nothrow) SocketData();
    if(nullptr == socket_data) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " new socket data failed.");
        return nullptr;
    }
    socket_data->id = (uint64_t)socket_data;
    socket_data->client_mgr = this;
    socket_data->fd = fd;

    client_ptr = std::make_shared<Client>(socket_mgr_ptr, socket_data->id, fd);
    if(nullptr == client_ptr) {
        delete socket_data;
        socket_data = nullptr;
        return nullptr;
    }
    socket_data->client_ptr = client_ptr;

    //构造一个bufferevent
    socket_data->buffer_event = bufferevent_socket_new(socket_mgr_ptr->event_base_, fd, BEV_OPT_CLOSE_ON_FREE);
    if(!socket_data->buffer_event) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " constructing bufferevent failed.");
        delete socket_data;
        socket_data = nullptr;
        return nullptr;
    }

    //绑定读事件回调函数、写事件回调函数、错误事件回调函数
    bufferevent_setcb(socket_data->buffer_event, read_callback, write_callback, tcp_event_callback, socket_data);

    bufferevent_enable(socket_data->buffer_event, EV_WRITE);
    bufferevent_enable(socket_data->buffer_event, EV_READ);
    ret = add2socketDataMap(socket_data);
    if(ret < 0) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " add to socket data map failed.");
        bufferevent_free(socket_data->buffer_event);
        delete socket_data;
        socket_data = nullptr;
        return nullptr;
    }
    client_ptr->setCallback(callback_ptr);
    socket_mgr_ptr->sendNotify();

    return client_ptr;
}

int ClientMgr::stopAllClient() {
    std::lock_guard<std::mutex> mutex_guard(socket_data_map_mutex_);
    for(auto &item : socket_data_map_) {
        closeClientByClientID(item.second->id);
    }
    socket_data_map_.clear();
    return 0;
}

int ClientMgr::stopClient(const uint64_t client_id) {
    return closeClientByClientID(client_id);
}

int ClientMgr::add2socketDataMap(SocketData* socket_data) {
    if(nullptr == socket_data || 0 == socket_data->id) return -1;

    std::lock_guard<std::mutex> mutex_guard(socket_data_map_mutex_);
    auto it = socket_data_map_.find(socket_data->id);
    if(socket_data_map_.end() == it) {
        socket_data_map_[socket_data->id] = socket_data;
        return 0;
    }

    return -1;
}

int ClientMgr::closeSocketData(SocketData*& socket_data) {
    if(nullptr == socket_data) return -1;
    if(nullptr != socket_data->normal_event) {
        event_free(socket_data->normal_event);
    }
    if(nullptr != socket_data->buffer_event) {
        bufferevent_free(socket_data->buffer_event);
    }
    if(socket_data->fd > 0) {
        evutil_closesocket(socket_data->fd);
    }

    delete socket_data;
    socket_data = nullptr;

    auto socket_mgr_ptr = socket_mgr_ptr_.lock();
    assert(nullptr != socket_mgr_ptr);
    socket_mgr_ptr->sendNotify();

    return 0;
}

evutil_socket_t ClientMgr::getFDByClientID(const uint64_t client_id) {
    std::lock_guard<std::mutex> mutex_guard(socket_data_map_mutex_);
    auto it = socket_data_map_.find(client_id);
    if(socket_data_map_.end() == it) {
        return -1;
    }

    return it->second->fd;
}

int ClientMgr::closeClientByClientID(const uint64_t client_id) {
    SocketData* socket_data = nullptr;
    {
        std::lock_guard<std::mutex> mutex_guard(socket_data_map_mutex_);
        auto it = socket_data_map_.find(client_id);
        if(socket_data_map_.end() == it) {
            return 0;
        }
        socket_data = it->second;
        socket_data_map_.erase(it);
    }
    closeSocketData(socket_data);
    return 0;
}

int ClientMgr::send(const uint64_t client_id, std::shared_ptr<BufferUtility> buffer_ptr) {
    std::lock_guard<std::mutex> mutex_guard(socket_data_map_mutex_);
    auto it = socket_data_map_.find(client_id);
    if(socket_data_map_.end() == it) {
        return -1;
    }

    SocketData* socket_data = it->second;
    if(nullptr != socket_data->buffer_event) {
        return bufferevent_write(socket_data->buffer_event, buffer_ptr->getData(), buffer_ptr->getDataLen());
    }

    if(socket_data->fd > 0) {
        return ::send(socket_data->fd, (const char*)buffer_ptr->getData(), buffer_ptr->getDataLen(), 0);
    }

    return -1;
}

int ClientMgr::send(const uint64_t client_id, HostInfo& host_info, std::shared_ptr<BufferUtility> buffer_ptr) {
    auto fd = getFDByClientID(client_id);
    if(fd < 1) return -1;

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = host_info.getIP();
    sin.sin_port = htons(host_info.getPort());
    struct sockaddr* sock_addr = (struct sockaddr*)&sin;

    int ret = ::sendto(fd, (const char*)buffer_ptr->getData(), buffer_ptr->getDataLen(), 0, sock_addr, sizeof(sin));
    return ret;
}

void ClientMgr::handleError(SocketData* socket_data, const int error_code) {
    auto client_ptr = socket_data->client_ptr.lock();
    if(nullptr!= client_ptr) {
        client_ptr->handleError(error_code);
    }

    stopClient(socket_data->id);
}

//////////////////////////////////////////////////////////

// 读缓冲区回调
void read_callback(struct bufferevent* bev, void* arg) {
    assert(nullptr != bev && nullptr != arg);
    SocketData* socket_data = (SocketData*)arg;
    BufferUtilityPtr buffer_ptr = std::make_shared<BufferUtility>(1024);
    assert(nullptr != buffer_ptr);
    size_t read_bytes = bufferevent_read(bev, buffer_ptr->getData(), buffer_ptr->getBufferMaxLen());
    if(read_bytes < 0) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " read data failed.");
        if(nullptr != socket_data->client_mgr) {
            socket_data->client_mgr->handleError(socket_data, read_bytes);
        }
        return;
    }
    buffer_ptr->setDataLen(read_bytes);
    auto client_ptr = socket_data->client_ptr.lock();
    if(nullptr != client_ptr) {
        client_ptr->handleRead(socket_data->host_info, buffer_ptr, read_bytes);
    }
    else if(nullptr != socket_data->client_mgr) {
        socket_data->client_mgr->handleError(socket_data, 0);
    }
}

// 写缓冲区回调
void write_callback(struct bufferevent* bev, void* arg) {
    //printf("我是写缓冲区的回调函数...您已发送\n");
}

// 事件
void tcp_event_callback(struct bufferevent* bev, short events, void* arg) {
    bool is_error = false;
    if(events & BEV_EVENT_EOF) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " connection closed.");
        is_error = true;
    }
    else if(events & BEV_EVENT_ERROR) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " some other error.");
        is_error = true;
    }

    assert(nullptr != bev && nullptr != arg);
    SocketData* socket_data = (SocketData*)arg;
    if(is_error && nullptr != socket_data->client_mgr) {
        socket_data->client_mgr->handleError(socket_data, -1);
    }
}

static int udp_read_impl(SocketData* socket_data, evutil_socket_t fd, IClientPtr client_ptr) {
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    int sin_len = sizeof(sin);
    BufferUtilityPtr& buffer_ptr = socket_data->read_buff_ptr;
    int read_bytes = recvfrom(fd, (char*)buffer_ptr->getData(), buffer_ptr->getBufferMaxLen(), 0, (struct sockaddr*)&sin, &sin_len);
    if(read_bytes < 0) {
        int error_code = GetLastError();
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " " << socket_data->host_info.getPort() << ", read data failed: " << read_bytes << ", last_error_no: " << error_code);
        return -1;
    }
    buffer_ptr->setDataLen(read_bytes);
    HostInfo remote_host_info(sin.sin_addr.S_un.S_addr, ntohs(sin.sin_port));
    client_ptr->handleRead(remote_host_info, buffer_ptr, read_bytes);
    buffer_ptr->setDataLen(0);
    return 0;
}

void udp_event_callback(evutil_socket_t fd, short events, void* param) {
    if(nullptr == param) return;
    SocketData* socket_data = (SocketData*)param;

    auto client_ptr = socket_data->client_ptr.lock();
    if(nullptr == client_ptr) {
        if(nullptr != socket_data->client_mgr) {
            socket_data->client_mgr->handleError(socket_data, 0);
        }
        return;
    }
    //printf(" %llu normal event_callback\n", socket_data->id);
    if(events & EV_READ) {
        int ret = udp_read_impl(socket_data, fd, client_ptr);
        if(0 != ret) {
            return;
        }
    }
    if(events & EV_WRITE) {
        client_ptr->handleSend(socket_data->host_info, -1);
    }
    if(events & EV_ERR) {
        int error_code = GetLastError();
        printf(" %llu, read data failed: last error_no[%d].\n", socket_data->id, error_code);
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " " << socket_data->id << ", read data failed， last_error_no: " << error_code);
        socket_data->client_mgr->handleError(socket_data, error_code);
    }
}

}
