#include "SocketMgr.h"
#include "Client.h"
#include <cassert>
#include "ServerCallback.h"

namespace hq {

static void listener_callback(struct evconnlistener*, evutil_socket_t, struct sockaddr*, int socklen, void*);
static void read_callback(struct bufferevent* bev, void* arg);
static void write_callback(struct bufferevent* bev, void* arg);
static void tcp_event_callback(struct bufferevent *bev, short events, void *arg);
static void udp_event_callback(evutil_socket_t fd, short events, void* param);
static void timer_callback(int sock, short event, void* arg);

struct sockaddr_in getSockAddrFromHostInfo(const HostInfo &host_info) {
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_addr.s_addr = host_info.getIP();
    sin.sin_family = AF_INET;
    sin.sin_port = htons(host_info.getPort());

    return sin;
}

SocketMgr::SocketMgr(): event_base_(nullptr), socket_thread_(nullptr) {
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
    msg_socket_pair_[0] = msg_socket_pair_[1] = -1;
    msg_socket_event_pair_[0] = msg_socket_event_pair_[1] = nullptr;
    for (int i = 0; i < TIMER_INFO_COUNT; ++i) {
        timer_info_arr_[i] = nullptr;
    }
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

    prepareMsgSocket();
    socket_thread_ = new std::thread([=]() {
        event_base_dispatch(event_base_);
    });
    if(nullptr == socket_thread_) {
        event_base_free(event_base_);
        event_base_ = nullptr;
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " start failed.");
        return -1;
    }

    LOG4CPLUS_DEBUG(log4cplus::Logger::getInstance("netwrapper"), " start success.");

    return 0;
}

int SocketMgr::stop() {
    if(nullptr == event_base_) return 0;

    {
        std::lock_guard<std::mutex> mutex_guard(socket_data_map_mutex_);
        for(auto it = socket_data_map_.begin(); it != socket_data_map_.end();) {
            closeSocketData(it->second);
            socket_data_map_.erase(it++);
        }
    }

    stopTcpServer(0);

    auto closeEventFun = [=](evutil_socket_t& fd, bufferevent*& bev) {
        if (nullptr != bev) {
            bufferevent_free(bev);
            bev = nullptr;
            fd = -1;
        }
    };
    closeEventFun(msg_socket_pair_[0], msg_socket_event_pair_[0]);
    closeEventFun(msg_socket_pair_[1], msg_socket_event_pair_[1]);

    for(int i = 0; i < TIMER_INFO_COUNT; ++i) {
        stopTimer(i);
    }

    event_base_loopbreak(event_base_);
    socket_thread_->join();

    if(nullptr != event_base_) {
        event_base_free(event_base_);
        event_base_ = nullptr;
    }
    delete socket_thread_;
    socket_thread_ = nullptr;

    LOG4CPLUS_DEBUG(log4cplus::Logger::getInstance("netwrapper"), " stop.");

    return 0;
}

int SocketMgr::prepareMsgSocket() {
    /// 没有特殊需求，可只绑定一个fd至event
    if(evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, msg_socket_pair_) == -1) {
        return -1;
    }
    if(-1 == evutil_make_socket_nonblocking(msg_socket_pair_[0])) {
        goto error_flag;
    }
    if(-1 == evutil_make_socket_nonblocking(msg_socket_pair_[1])) {
        goto error_flag;
    }

    msg_socket_event_pair_[0] = bufferevent_socket_new(event_base_, msg_socket_pair_[0], BEV_OPT_CLOSE_ON_FREE);
    if(nullptr == msg_socket_event_pair_[0]) {
        goto error_flag;
    }

    msg_socket_event_pair_[1] = bufferevent_socket_new(event_base_, msg_socket_pair_[1], BEV_OPT_CLOSE_ON_FREE);
    if(nullptr == msg_socket_event_pair_[1]) {
        bufferevent_free(msg_socket_event_pair_[0]);
        evutil_closesocket(msg_socket_pair_[1]);
        return -1;
    }

    bufferevent_disable(msg_socket_event_pair_[0], EV_READ);
    //bufferevent_enable(msg_socket_event_pair_[0], EV_WRITE);

    bufferevent_enable(msg_socket_event_pair_[1], EV_READ);

    return 0;

error_flag:
    evutil_closesocket(msg_socket_pair_[0]);
    evutil_closesocket(msg_socket_pair_[1]);
    return -1;
}

int SocketMgr::stopClient(const uint64_t client_id) {
    std::lock_guard<std::mutex> mutex_guard(socket_data_map_mutex_);
    auto it = socket_data_map_.find(client_id);
    if(socket_data_map_.end() == it) return 0;

    return closeSocketData(it->second);
}

evutil_socket_t SocketMgr::createUdpSocket() {
    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " create socket failed.");
        return fd;
    }

    int ret = evutil_make_socket_nonblocking(fd);
    if(ret < 0) {
        evutil_closesocket(fd);
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " set socket noblock failed.");
        return -1;
    }
    return fd; ///< socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

evutil_socket_t SocketMgr::createUdpBindSocket(const HostInfo &host_info) {
    struct sockaddr_in sin = getSockAddrFromHostInfo(host_info);
    int fd = createUdpSocket();
    if(fd < 0) return -1;

    int ret = ::bind(fd, (struct sockaddr*)&sin, sizeof(sin));
    if(ret < 0) {
        evutil_closesocket(fd);
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " bind socket failed.");
        return ret;
    }
    return fd;
}

IClientPtr SocketMgr::startUdpClient(std::shared_ptr<IClientCallback> callback_ptr) {
    if(nullptr == event_base_) return nullptr;
    evutil_socket_t fd = createUdpSocket();
    if(fd < 0) {
        //delete socket_data;
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " create socket failed.");
        return nullptr;
    }

    auto client_ptr = std::make_shared<Client> ();
    SocketData* socket_data = createSocketData(client_ptr, fd, HostInfo(), nullptr);
    if(nullptr == socket_data) {
        evutil_closesocket(fd);
        delete socket_data;
        return nullptr;
    }

    socket_data->normal_event = event_new(event_base_, fd,
                                        EV_READ | EV_PERSIST,
                                        udp_event_callback, socket_data);
    if(nullptr == socket_data->normal_event) {
        //fprintf(stderr, "socket init failed\n");
        evutil_closesocket(fd);
        event_free(socket_data->normal_event);
        delete socket_data;
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " create event failed.");
        return nullptr;
    }
    event_add(socket_data->normal_event, NULL);

    int ret = add2socketDataMap(socket_data);
    if(ret < 0) {
        evutil_closesocket(fd);
        event_free(socket_data->normal_event);
        delete socket_data;
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " add to socket data map failed.");
        return nullptr;
    }

    client_ptr->start(callback_ptr, (void*)socket_data);
    return client_ptr;
}

int SocketMgr::stopUdpClient(std::shared_ptr<IClient> client_ptr) {
    if (nullptr == client_ptr) return -1;

    return client_ptr->stop();
}

IClientPtr SocketMgr::startUdpServer(const HostInfo& host_info, std::shared_ptr<IClientCallback> callback_ptr) {
    if (!host_info.isValid()) return nullptr;
    evutil_socket_t fd = createUdpBindSocket(host_info);
    if(fd < 0) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " create bind socket failed.");
        return nullptr;
    }

    auto client_ptr = std::make_shared<Client>();
    SocketData* socket_data = createSocketData(client_ptr, fd, host_info, nullptr);
    if(nullptr == socket_data) {
        evutil_closesocket(fd);
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " new socket data failed.");
        return nullptr;
    }
    socket_data->normal_event = event_new(event_base_, fd,
                                          EV_READ | EV_PERSIST,
                                          udp_event_callback, socket_data);
    if(nullptr == socket_data->normal_event) {
        //fprintf(stderr, "socket init failed\n");
        evutil_closesocket(fd);
        event_free(socket_data->normal_event);
        delete socket_data;
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " set event failed.");
        return nullptr;
    }
    int ret = event_add(socket_data->normal_event, NULL);
    ret = add2socketDataMap(socket_data);
    if(ret < 0) {
        evutil_closesocket(fd);
        event_free(socket_data->normal_event);
        delete socket_data;
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " add to socket data map failed.");
        return nullptr;
    }

    client_ptr->start(host_info, callback_ptr, socket_data);
    return client_ptr;
}

int SocketMgr::stopUdpServer(std::shared_ptr<IClient> client_ptr) {
    if (nullptr == client_ptr) return -1;

    return client_ptr->stop();
}

SocketData* SocketMgr::createSocketData(IClientPtr client_ptr, const evutil_socket_t fd, const HostInfo& host_info, bufferevent* bev) {
    if(nullptr == client_ptr) return nullptr;
    SocketData* socket_data = new(std::nothrow) SocketData();
    if(nullptr == socket_data) {
        //bufferevent_free(bev);
       LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " new socket data failed.");
        return nullptr;
    }

    socket_data->id = (uint64_t)socket_data;
    //socket_data->fd = fd;
    socket_data->socket_mgr = this;
    socket_data->client_ptr = client_ptr;
    socket_data->host_info = host_info;
    socket_data->fd = fd;
    socket_data->buffer_event = bev;

    return socket_data;
}

IClientPtr SocketMgr::startTcpClient(const HostInfo& host_info, std::shared_ptr<IClientCallback> callback_ptr) {
    struct sockaddr_in sin = getSockAddrFromHostInfo(host_info);
    struct bufferevent* bev = bufferevent_socket_new(event_base_, -1, BEV_OPT_CLOSE_ON_FREE);
    if(bev == NULL) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " buffer event new failed.");
        return nullptr;
    }

    //连接服务端
    int flag = bufferevent_socket_connect(bev, (struct ::sockaddr*)&sin, sizeof(sin));
    if(-1 == flag) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " buffer event connect failed.");
        bufferevent_free(bev);
        return nullptr;
    }

    int ret = bufferevent_enable(bev, EV_READ | EV_WRITE);
    if(ret < 0) {
        bufferevent_free(bev);
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " buffer event enable failed.");
        return nullptr;
    }

    auto client_ptr = std::make_shared<Client>();
    SocketData* socket_data = createSocketData(client_ptr, 0, host_info, bev); ///< new(std::nothrow) SocketData();
    if(nullptr == socket_data) {
        bufferevent_free(bev);
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " new socket data failed.");
        return nullptr;
    }

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

    client_ptr->start(host_info, callback_ptr, socket_data);
    return client_ptr;
}

int SocketMgr::stopTcpClient(std::shared_ptr<IClient> client_ptr) {
    if (nullptr == client_ptr) return 0;
    return client_ptr->stop();
}

int SocketMgr::startTcpServer(const HostInfo& host_info, std::shared_ptr<IServerCallback> callback_ptr) {
    struct sockaddr_in sin = getSockAddrFromHostInfo(host_info);

    ListenerData* listener_data = new ListenerData();
    if(nullptr == listener_data) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " new listener data failed.");
        return -1;
    }
    //创建、绑定、监听socket
    listener_data->tcp_listener = evconnlistener_new_bind(event_base_, listener_callback, (void*)listener_data,
                                       LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
                                       (struct ::sockaddr*)&sin,
                                       sizeof(sin));

    if(nullptr == listener_data->tcp_listener) {
        //fprintf(stderr, "Could not create a listener!\n");
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " create listener socket failed.");
        return 1;
    }

    listener_data->id = (uint64_t)listener_data->tcp_listener;
    listener_data->socket_mgr = this;
    //listener_data->event_base = socket_base_;
    listener_data->host_info = host_info;
    listener_data->server_callback = callback_ptr;

    std::lock_guard<std::mutex> mutex_guard(listener_data_map_mutex_);
    listener_data_map_[listener_data->id] = listener_data;

    return 0;
}

int SocketMgr::stopTcpServer(const uint64_t id/*=0*/) {
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

    std::lock_guard<std::mutex> mutex_guard(listener_data_map_mutex_);
    for(auto item : listener_data_map_) {
        ListenerData* listner_data = item.second;
        evconnlistener_free(listner_data->tcp_listener);
        delete listner_data;
    }
    listener_data_map_.clear();
    return 0;
}

void SocketMgr::handleAccept(struct evconnlistener* tcp_listener, evutil_socket_t fd, HostInfo& remote_host_info, void* ptr) {
    uint64_t id = (uint64_t)tcp_listener;
    ListenerData* listener_data = (ListenerData*)ptr;
    assert(id == listener_data->id);

    int ret = 0;
    auto client_ptr = std::make_shared<Client>();
    SocketData* socket_data = createSocketData(client_ptr, fd, remote_host_info, nullptr);
    if (nullptr == socket_data || nullptr == client_ptr) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " new socket data failed.");
        goto accept_error_1;
    }

    {
        std::lock_guard<std::mutex> mutex_guard(listener_data_map_mutex_);
        auto it = listener_data_map_.find(id);
        if(listener_data_map_.end() == it || nullptr == listener_data->server_callback) {
            goto accept_error_2;
        }
    }

    //构造一个bufferevent
    socket_data->buffer_event = bufferevent_socket_new(event_base_, fd, BEV_OPT_CLOSE_ON_FREE);
    if(!socket_data->buffer_event) {
        //fprintf(stderr, "Error constructing bufferevent!");
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " constructing bufferevent failed.");
        goto accept_error_2;
    }

    //绑定读事件回调函数、写事件回调函数、错误事件回调函数
    bufferevent_setcb(socket_data->buffer_event, read_callback, write_callback, tcp_event_callback, socket_data);

    bufferevent_enable(socket_data->buffer_event, EV_WRITE);
    bufferevent_enable(socket_data->buffer_event, EV_READ);
    ret = add2socketDataMap(socket_data);
    if(ret < 0) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " add to socket data map failed.");
        goto accept_error_3;
    }

    client_ptr->start(remote_host_info, nullptr, socket_data);
    listener_data->server_callback->handleAcceptor(client_ptr);

accept_error_3:
    event_free(socket_data->normal_event);
accept_error_2:
    delete socket_data;
accept_error_1:
    //evconnlistener_free(tcp_listener);
    evutil_closesocket(fd);
}

void SocketMgr::handleError(SocketData* socket_data, const int error_code) {
    if(nullptr!=socket_data->normal_event) {
        event_free(socket_data->normal_event);
    } 
    if(nullptr!=socket_data->buffer_event) {
        bufferevent_free(socket_data->buffer_event);
    }
    auto client_ptr = socket_data->client_ptr.lock();
    if(nullptr!= client_ptr) {
        client_ptr->handleError(error_code);
        client_ptr.reset();
    }

    std::lock_guard<std::mutex> mutex_guard(socket_data_map_mutex_);
    socket_data_map_.erase(socket_data->id);
    delete socket_data;
}

int SocketMgr::add2socketDataMap(SocketData* socket_data) {
    if(nullptr == socket_data || 0 == socket_data->id) return -1;

    std::lock_guard<std::mutex> mutex_guard(socket_data_map_mutex_);
    auto it = socket_data_map_.find(socket_data->id);
    if(socket_data_map_.end() == it) {
        socket_data_map_[socket_data->id] = socket_data;
        return 0;
    }

    return -1;
}

int SocketMgr::closeSocketData(SocketData* &socket_data) {
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
    //memset(this, 0, offsetof(SocketData, host_info));
    delete socket_data;
    socket_data = nullptr;
    return 0;
}

int SocketMgr::startTimer(const int seconds, const int useconds, ITimerCallbackPtr callback_ptr) {
    if((seconds <= 0 && useconds < 0) || (nullptr == callback_ptr)) {
        return -1;
    }

    int timer_id = 0;
    {
        std::lock_guard<std::mutex> lock_guard(timer_arr_mutex_);
        for(; timer_id < TIMER_INFO_COUNT; ++timer_id) {
            if(NULL == timer_info_arr_[timer_id]) break;
        }
    }
    if(timer_id == TIMER_INFO_COUNT) {
        return -1;
    }
    TimerEventInfo *timer_info = new(std::nothrow) TimerEventInfo();
    if(nullptr == timer_info) {
        return -1;
    }

    timer_info->tv.tv_sec = seconds;
    timer_info->tv.tv_usec = useconds;
    timer_info->timer_event = event_new(event_base_, -1, 0, timer_callback, timer_info);
    if(nullptr == timer_info->timer_event) {
        delete timer_info;
        return -1;
    }
    //evtimer_set(timer_info->timer_event, )
    event_add(timer_info->timer_event, &timer_info->tv);

    timer_info->timer_id = timer_id;
    timer_info->socket_msg = this;
    timer_info->timer_callback = callback_ptr;

    /// @brief need lock 
    std::lock_guard<std::mutex> lock_guard(timer_arr_mutex_);
    timer_info_arr_[timer_id] = timer_info;

    return 0;
}

int SocketMgr::stopTimer(const int timer_id) {
    if(timer_id >= TIMER_INFO_COUNT || timer_id < 0) {
        return -1;
    }

    // need lock
    //::assert(timer_id == timer_info->timer_id);
    std::lock_guard<std::mutex> lock_guard(timer_arr_mutex_);
    TimerEventInfo* timer_info = timer_info_arr_[timer_id];
    if(0 == timer_info) return 0;
    if(nullptr != timer_info->timer_event) {
        evtimer_del(timer_info->timer_event);
        event_free(timer_info->timer_event);
        timer_info->timer_event = nullptr;
    }

    delete timer_info;
    timer_info_arr_[timer_id] = 0;
    return 0;
}

ISocketMgrPtr createSocketMgr() {
    return std::shared_ptr<ISocketMgr>(new SocketMgr());
}

////////////////////////////////////////////////////////////////////
void listener_callback(struct evconnlistener* listener, evutil_socket_t fd, struct ::sockaddr* addr, int len, void* ptr) {
    if(nullptr == ptr || nullptr==addr) return;
    struct sockaddr_in* addr_v4 = (struct sockaddr_in*)addr;
    HostInfo host_info(addr_v4->sin_addr.s_addr, addr_v4->sin_port);
    struct ListenerData* listener_data = (struct ListenerData*)ptr;
    if(nullptr != listener_data->socket_mgr) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " accept a new client [" << host_info.getIPString() << ":" << host_info.getPort() << "].");
        listener_data->socket_mgr->handleAccept(listener, fd, host_info, ptr);
    }
    else {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " accept [" << host_info.getIPString() << ":" << host_info.getPort() << "] failed.");
    }
}

// 读缓冲区回调
void read_callback(struct bufferevent* bev, void* arg) {
    assert(nullptr!=bev && nullptr!=arg);
    SocketData *socket_data = (SocketData*)arg;
    BufferUtilityPtr buffer_ptr = std::make_shared<BufferUtility>(1024);
    assert(nullptr != buffer_ptr);
    size_t read_bytes = bufferevent_read(bev, buffer_ptr->getData(), buffer_ptr->getBufferMaxLen());
    if(read_bytes < 0) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " read data failed.");
        if(nullptr != socket_data->socket_mgr) {
            socket_data->socket_mgr->handleError(socket_data, read_bytes);
        }
        return ;
    }
    buffer_ptr->setDataLen(read_bytes);
    auto client_ptr = socket_data->client_ptr.lock();
    if(nullptr!=client_ptr) {
        client_ptr->handleRead(socket_data->host_info, buffer_ptr, read_bytes);
    }
    else if(nullptr != socket_data->socket_mgr) {
        socket_data->socket_mgr->handleError(socket_data, 0);
    }
}

// 写缓冲区回调
void write_callback(struct bufferevent* bev, void* arg) {
    //printf("我是写缓冲区的回调函数...您已发送\n");
}

// 事件
void tcp_event_callback(struct bufferevent* bev, short events, void* arg) {
    if(events & BEV_EVENT_EOF) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " connection closed.");
    }
    else if(events & BEV_EVENT_ERROR) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " some other error.");
    }

    assert(nullptr!=bev && nullptr!=arg);
    SocketData *socket_data = (SocketData*)arg;
    if(nullptr != socket_data->socket_mgr) {
        socket_data->socket_mgr->handleError(socket_data, -1);
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
        if(nullptr != socket_data->socket_mgr) {
            socket_data->socket_mgr->handleError(socket_data, 0);
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
        socket_data->socket_mgr->handleError(socket_data, error_code);
    }
}

// 定时事件回调函数
void timer_callback(int sock, short event, void* arg) {
    // 重新添加定时事件（定时事件触发后默认自动删除）
    TimerEventInfo* timer_info = (TimerEventInfo*)arg;
    if(nullptr == timer_info) {
        return ;
    }

    if(nullptr != timer_info->socket_msg && nullptr != timer_info->timer_callback) {
        timer_info->timer_callback->timerCallback(timer_info->timer_id);
    }

    if(nullptr != timer_info->timer_event) {
        event_add(timer_info->timer_event, &timer_info->tv);
    }
}

}
