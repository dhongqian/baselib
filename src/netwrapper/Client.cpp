#include "SocketMgr.h"
#include "Client.h"

namespace hq {

Client::Client(): is_start_(false), socket_data_(NULL) {
    read_buffer_ptr_ = std::make_shared<BufferUtility>(MAX_UDP_MTU);
}

Client::~Client() {
    read_buffer_ptr_ = nullptr;
}

int Client::start(IClientCallbackPtr callback_ptr, void* param/* = NULL*/) {
    is_start_ = true;
    callback_ptr_ = callback_ptr;
    read_buffer_ptr_ = std::make_shared<BufferUtility>(1600);
    socket_data_ = (SocketData*)param;
    return 0;
}

int Client::start(const HostInfo& host_info, IClientCallbackPtr callback_ptr, void* param/* = NULL*/) {
    start(callback_ptr, param);

    return 0;
}

int Client::stop() {
    is_start_ = false;
    if(nullptr != socket_data_ && nullptr != socket_data_->socket_mgr) {
        socket_data_->socket_mgr->stopClient(socket_data_->id);
    }
    return 0;
}

int Client::send(std::shared_ptr<BufferUtility> buffer_ptr) {
    if(nullptr == socket_data_) return -1;
    if(nullptr != socket_data_->buffer_event) {
        return bufferevent_write(socket_data_->buffer_event, buffer_ptr->getData(), buffer_ptr->getDataLen());
    }

    if(socket_data_->fd > 0) {
        return ::send(socket_data_->fd, (const char*)buffer_ptr->getData(), buffer_ptr->getDataLen(), 0);
    }
    return -1;
}

int Client::send(HostInfo& host_info, std::shared_ptr<BufferUtility> buffer_ptr) {
    if(nullptr == socket_data_) return -1;
    if(socket_data_->fd > 0) {
        struct sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = host_info.getIP();
        sin.sin_port = htons(host_info.getPort());

        struct sockaddr* sock_addr = (struct sockaddr*)&sin;

        int ret = ::sendto(socket_data_->fd, (const char*)buffer_ptr->getData(), buffer_ptr->getDataLen(), 0, sock_addr, sizeof(sin));
        return ret;
    }
    return 0;
}

int Client::read(std::shared_ptr<BufferUtility> buffer_ptr) {
    return 0;
}

int Client::read(HostInfo& host_info, std::shared_ptr<BufferUtility> buffer_ptr) {
    return 0;
}

void Client::handleRead(const HostInfo& host_info, BufferUtilityPtr read_buffer_ptr, std::size_t size) {
    struct in_addr inet_addr;
    inet_addr.S_un.S_addr = host_info.getIP();
    HostInfo remote_host(host_info);
    //printf("recv data %s from %s: %u\n", read_buffer_ptr->get_data(), remote_host.getIPString().c_str(), remote_host.getPort());

    //std::string str("I am server!");
    //BufferUtilityPtr send_buffer_ptr = std::make_shared<BufferUtility>((uint8_t*)str.c_str(), str.size());
    //send_buffer_ptr->set_data_len(str.size());
    //int ret = this->send(remote_host, send_buffer_ptr);
    if(nullptr != callback_ptr_) {
        callback_ptr_->handleRead(remote_host, read_buffer_ptr, size);
    }
    return;
}

void Client::handleSend(const HostInfo& host_info, std::size_t size) {

}

void Client::handleError(const int error_code) {

}

uint64_t Client::getClientID() {
    if(nullptr != socket_data_) return socket_data_->id;
    return 0;
}

HostInfo Client::localHostInfo() {
    if(nullptr == socket_data_ || socket_data_->fd < 0) {
        return HostInfo();
    }

    struct sockaddr_in local_addr;
    int socket_len = sizeof(local_addr);
    /// The getsockname function retrieves the local name for a socket.
    int ret = getsockname(socket_data_->fd, (sockaddr*)&local_addr, &socket_len);
    if(0 != ret) {
        int error_code = WSAGetLastError();
        return HostInfo();
    }

    HostInfo host_info(local_addr.sin_addr.s_addr, local_addr.sin_port);
    return host_info;
}

HostInfo Client::remoteHostInfo() {
    if(nullptr == socket_data_ || socket_data_->fd < 0) {
        return HostInfo();
    }

    /// The getpeername function retrieves the address of the peer to which a socket is connected.
    struct sockaddr_in remote_addr;
    int socket_len = sizeof(remote_addr);
    int ret = getpeername(socket_data_->fd, (sockaddr*)&remote_addr, &socket_len);
    if(0 != ret) {
        int error_code = WSAGetLastError();
        return HostInfo();
    }

    HostInfo host_info(remote_addr.sin_addr.s_addr, remote_addr.sin_port);
    return host_info;
}

}
