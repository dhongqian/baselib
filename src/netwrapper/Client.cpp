#include "SocketMgr.h"
#include "Client.h"

namespace hq {

Client::Client(std::shared_ptr<SocketMgr> socket_mgr_ptr, const uint64_t client_id, const evutil_socket_t fd): client_id_(client_id), socket_fd_(fd) {
    read_buffer_ptr_ = std::make_shared<BufferUtility>(MAX_UDP_MTU);
    socket_mgr_ptr_ = socket_mgr_ptr;
}

Client::~Client() {
    read_buffer_ptr_ = nullptr;
}

int Client::setCallback(IClientCallbackPtr callback_ptr) {
    callback_ptr_ = callback_ptr;
    return 0;
}

int Client::send(std::shared_ptr<BufferUtility> buffer_ptr) {
    auto socket_mgr_ptr = socket_mgr_ptr_.lock();
    if(nullptr == socket_mgr_ptr) return -1;

    return socket_mgr_ptr->send(client_id_, buffer_ptr);
}

int Client::send(HostInfo& host_info, std::shared_ptr<BufferUtility> buffer_ptr) {
    auto socket_mgr_ptr = socket_mgr_ptr_.lock();
    if(nullptr == socket_mgr_ptr) return -1;

    return socket_mgr_ptr->send(client_id_, host_info, buffer_ptr);
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
    printf("recv data %s from %s: %u\r\n", read_buffer_ptr->getData(), remote_host.getIPString().c_str(), remote_host.getPort());

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
    return client_id_;
}

HostInfo Client::localHostInfo() {
    struct sockaddr_in local_addr;
    int socket_len = sizeof(local_addr);
    /// The getsockname function retrieves the local name for a socket.
    int ret = getsockname(socket_fd_, (sockaddr*)&local_addr, &socket_len);
    if(0 != ret) {
        int error_code = WSAGetLastError();
        return HostInfo();
    }

    HostInfo host_info(local_addr.sin_addr.s_addr, local_addr.sin_port);
    return host_info;
}

HostInfo Client::remoteHostInfo() {
    /// The getpeername function retrieves the address of the peer to which a socket is connected.
    struct sockaddr_in remote_addr;
    int socket_len = sizeof(remote_addr);
    int ret = getpeername(socket_fd_, (sockaddr*)&remote_addr, &socket_len);
    if(0 != ret) {
        int error_code = WSAGetLastError();
        return HostInfo();
    }

    HostInfo host_info(remote_addr.sin_addr.s_addr, remote_addr.sin_port);
    return host_info;
}

}
