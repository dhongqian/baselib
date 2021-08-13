#include "MessageController.h"
#include <baselib/Utility.h>
#include <baselib/DataStream.h>

namespace hq {

MessageController::MessageController(): is_start_(false), begin_ticks_(0) {
}

MessageController::~MessageController() {

}

void MessageController::init(IClientPtr client_ptr, const HostInfo& remote_server) {
    client_ptr_ = client_ptr;
    buffer_ptr_ = std::make_shared<BufferUtility>(100);
    assert(nullptr != buffer_ptr_);
}

int MessageController::start(HostInfo& stun_server) {
    if(nullptr == client_ptr_ || is_start_) return -1;

    is_start_ = true;
    std::cout << " begin to start message controller: " << hq::utility::getCurrentDate() << std::endl;
    LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), " begin to stun " << hq::utility::getCurrentDate());
    begin_ticks_ = GetTickCount64();

    return 0;
}

int MessageController::stop() {
    is_start_ = false;
    begin_ticks_ = 0;
    return 0;
}

void MessageController::handleRead(const HostInfo& remote_host_info, BufferUtilityPtr read_buffer_ptr, std::size_t size) {
    HostInfo host_info = remote_host_info;
    LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), hq::utility::getCurrentDate() << " receive data from: " << host_info.getIPString().c_str() << ":" << host_info.getPort());
    std::cout << " local ip: " << client_ptr_->localHostInfo().getIPString() << ":" << std::dec << client_ptr_->localHostInfo().getPort() << std::endl;
    DataStream ds(read_buffer_ptr->getData(), read_buffer_ptr->getDataLen());
    uint16_t command = ds.readword();
   if(0x0001 == command) {
        if(nullptr == udp_cmd_ptr_) {
            udp_cmd_ptr_ = std::make_shared<UdpCmd>();
            udp_cmd_ptr_->init(client_ptr_);
        }
        if(nullptr != udp_cmd_ptr_) {
            udp_cmd_ptr_->handleRead(remote_host_info, read_buffer_ptr, size);
        }
    }
}

void MessageController::handleSend(const HostInfo& remote_host_info, std::size_t size) {

}

void MessageController::handleError(const int errror_code) {

}

void MessageController::timerCallback(const int timer_id) {
    if(nullptr != udp_cmd_ptr_) {
        udp_cmd_ptr_->timerCallback(timer_id);
    }
}

}
