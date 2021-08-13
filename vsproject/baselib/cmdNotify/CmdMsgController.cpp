#include "CmdMsgController.h"
#include <baselib/Utility.h>
#include <baselib/DataStream.h>

namespace hq {

CmdMsgController::CmdMsgController(): is_start_(false), begin_ticks_(0), task_thread_("task_thread") {
}

CmdMsgController::~CmdMsgController() {

}

void CmdMsgController::init(IClientPtr client_ptr) {
    client_ptr_ = client_ptr;
    buffer_ptr_ = std::make_shared<BufferUtility>(100);
    assert(nullptr != buffer_ptr_);
    task_thread_.start();
}

int CmdMsgController::start(HostInfo& remote_server, HostInfo& host_info) {
    if(nullptr == client_ptr_ || is_start_) return -1;

    is_start_ = true;
    std::cout << " begin to start message controller: " << hq::utility::getCurrentDate() << std::endl;
    LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), " begin to stun " << hq::utility::getCurrentDate());
    begin_ticks_ = GetTickCount64();
    task_thread_.postDelayTask(hq::Bind(&CmdMsgController::notify, this, remote_server, host_info), 1000);
    //notify(remote_server, host_info);

    return 0;
}

int CmdMsgController::stop() {
    is_start_ = false;
    begin_ticks_ = 0;
    task_thread_.stop();

    return 0;
}

void CmdMsgController::notify(HostInfo& remote_server, HostInfo& host_info) {
    std::cout << "notify " << hq::utility::getCurrentDate() << std::endl;
    task_thread_.postDelayTask(hq::Bind(&CmdMsgController::notify, this, remote_server, host_info), 1000);
    return;
    uint16_t cmd = 0x01;
    uint32_t ip = host_info.getIP();
    uint16_t port = host_info.getPort();
    char buf[8] = {0};
    DataStream ds(buffer_ptr_->getData(), buffer_ptr_->getBufferMaxLen());
    ds << cmd;
    ds << ip;
    ds << port;
    if(nullptr != client_ptr_) {
        buffer_ptr_->setDataLen(ds.size());
        client_ptr_->send(remote_server, buffer_ptr_);
        auto local_host = client_ptr_->localHostInfo();
        std::string ip = local_host.getIPString();
    }
}

void CmdMsgController::handleRead(const HostInfo& remote_host_info, BufferUtilityPtr read_buffer_ptr, std::size_t size) {
    HostInfo host_info = remote_host_info;
    LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), hq::utility::getCurrentDate() << " receive data from: " << host_info.getIPString().c_str() << ":" << host_info.getPort());

    DataStream ds(read_buffer_ptr->getData(), read_buffer_ptr->getDataLen());
    uint16_t command = ds.readword();
}

void CmdMsgController::handleSend(const HostInfo& remote_host_info, std::size_t size) {
    hq::HostInfo host_info = remote_host_info;
    LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), " send " << size << " byte data to " << host_info.getIPString() << ":" << host_info.getPort());
}

void CmdMsgController::handleError(const int errror_code) {

}

void CmdMsgController::timerCallback(const int timer_id) {
}

}
