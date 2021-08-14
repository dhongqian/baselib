#include "DownloadAlogrithm.h"
#include <baselib/Utility.h>
#include <iomanip>
#include "baselib/DataStream.h"

namespace hq {
namespace stun {

DownloadAlogrithm::DownloadAlogrithm(): is_start_(false), start_send_ticks_(0), recv_bytes_(0), task_thread_("download_alogrithm") {
}

DownloadAlogrithm::~DownloadAlogrithm() {

}

void DownloadAlogrithm::init(IClientPtr client_ptr, const HostInfo& remote_server) {
    client_ptr_ = client_ptr;
    request_buffer_ptr_ = std::make_shared<BufferUtility>(100);
    response_buffer_ptr_ = std::make_shared<BufferUtility>(1500);
    //assert(nullptr != buffer_ptr_);
    remote_server_ = remote_server;
}

int DownloadAlogrithm::start(HostInfo& stun_server) {
    if(nullptr == client_ptr_.lock() || is_start_) {
        LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), " start failed, is_start[ " << (is_start_?"true":"false") << "].");
        return -1;
    }

    is_start_ = true;
    task_thread_.start();
    start_send_ticks_ = GetTickCount64();
    std::cout << " start " << hq::utility::getCurrentDate() << std::endl;
    LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), " start. ");

    send_request_message(remote_server_);

    return 0;
}

int DownloadAlogrithm::stop() {
    is_start_ = false;
    task_thread_.stop();
    start_send_ticks_ = 0;
    recv_bytes_ = 0;
    return 0;
}

void DownloadAlogrithm::handle_read(const HostInfo& remote_host_info, BufferUtilityPtr read_buffer_ptr, std::size_t size) {
    HostInfo host_info = remote_host_info;
    LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), " receive data from: " << host_info.getIPString().c_str() << ":" << host_info.getPort());

    ResponseInfo response;
    DataStream ds(read_buffer_ptr->getData(), read_buffer_ptr->getDataLen());
    ds >> response.msg_id;
    ds >> response.msg_len;

    recv_bytes_ += response.msg_len;

    send_request_message(remote_server_);
}

void DownloadAlogrithm::handle_send(const HostInfo& remote_host_info, std::size_t size) {

}

void DownloadAlogrithm::handle_error(const int errror_code) {

}

BufferUtilityPtr DownloadAlogrithm::construct_req_message(const int cmd) {
    RequestInfo request;
    request.msg_id = cmd;
    request.msg_len = 1024;

    DataStream ds(request_buffer_ptr_->getData(), request_buffer_ptr_->getBufferMaxLen());
    ds.writeword(request.msg_id);
    ds.writedword(request.msg_len);
    request_buffer_ptr_->setDataLen(ds.size());
    return request_buffer_ptr_;
}

BufferUtilityPtr DownloadAlogrithm::construct_resp_message(const int cmd) {
    RequestInfo request;
    request.msg_id = cmd;
    request.msg_len = 1024;

    DataStream ds(response_buffer_ptr_->getData(), response_buffer_ptr_->getBufferMaxLen());
    ds.writeword(request.msg_id);
    ds.writedword(request.msg_len);
    response_buffer_ptr_->setDataLen(ds.size());
    return response_buffer_ptr_;
}

int DownloadAlogrithm::send_request_message(HostInfo& host_info) {
    BufferUtilityPtr request_msg = construct_req_message(0);
    if(nullptr == request_msg) return -1;

    auto client = client_ptr_.lock();
    if(nullptr == client) {
        LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), " client is null, send failed.");
        return -1;
    }
    int ret = client->send(host_info, request_msg);
    if(ret < 0) {
        //
        return -1;
    }
    LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), "send msg cmd: 0x" << std::hex << std::setw(4) << std::setfill('0') << 1 << ", remote server: " << host_info.getIPString() << ":" << std::dec << host_info.getPort());
    return 0;
}

int DownloadAlogrithm::send_response_message(HostInfo& host_info) {
    BufferUtilityPtr request_msg = construct_req_message(1);
    if(nullptr == request_msg) return -1;

    auto client = client_ptr_.lock();
    if(nullptr == client) {
        LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), " client is null, send failed.");
        return -1;
    }
    int ret = client->send(host_info, request_msg);
    if(ret < 0) {
        //
        return -1;
    }
    LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), "send msg cmd: 0x" << std::hex << std::setw(4) << std::setfill('0') << 1 << ", remote server: " << host_info.getIPString() << ":" << std::dec << host_info.getPort());
    return 0;
}

void DownloadAlogrithm::handle_request_message(RequestInfo& request) {
}

void DownloadAlogrithm::handle_response_message(ResponseInfo& response) {
}


}
}