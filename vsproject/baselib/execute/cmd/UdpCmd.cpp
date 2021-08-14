#include "UdpCmd.h"
#include <baselib/Utility.h>
#include <iomanip>
#include <baselib/DataStream.h>

namespace hq {

UdpCmd::UdpCmd(): is_start_(false), keep_alive_count_(0) {
}

UdpCmd::~UdpCmd() {

}

void UdpCmd::init(IClientPtr client_ptr) {
    client_ptr_ = client_ptr;
    //assert(nullptr != buffer_ptr_);
}

int UdpCmd::start() {
    if(client_ptr_.expired() || is_start_) return -1;

    is_start_ = true;

    return 0;
}

int UdpCmd::stop() {
    is_start_ = false;
    keep_alive_count_ = 0;
    return 0;
}

void UdpCmd::handleRead(const HostInfo& remote_host_info, BufferUtilityPtr read_buffer_ptr, std::size_t size) {
    HostInfo host_info = remote_host_info;
    LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), " receive data from: " << host_info.getIPString().c_str() << ":" << host_info.getPort());
    if(size < 8) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getRoot(), " invalid receive data, less than 8 byte.");
        return;
    }

    DataStream ds(read_buffer_ptr->getData(), read_buffer_ptr->getDataLen());
    uint16_t cmd = ds.readword();
    uint32_t remote_ip = ds.readdword();
    uint16_t remote_port = ds.readword();
    remote_server_.set(remote_ip, remote_port);
    LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), " get remote server: " << remote_server_.getIPString() << ":" << remote_server_.getPort());
    return;
}

void UdpCmd::handleSend(const HostInfo& remote_host_info, std::size_t size) {

}

void UdpCmd::handleError(const int errror_code) {

}

void UdpCmd::timerCallback(const int timer_id) {
    keepAlive();
}

void UdpCmd::keepAlive() {
    auto client_ptr = client_ptr_.lock();
    if(nullptr!=client_ptr && keep_alive_count_++ % 5 == 0) {
        const char* data = "keep alive";
        BufferUtilityPtr buffer_ptr = std::make_shared<BufferUtility>((uint8_t*)data, strlen(data));
        client_ptr->send(remote_server_, buffer_ptr);
        LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), " send keep alive to " << remote_server_.getIPString() << ":" << remote_server_.getPort());
    }
}

}