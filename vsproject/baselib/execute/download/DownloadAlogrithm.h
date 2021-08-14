#pragma once
#include <string>
#include <atomic>
#include <list>
#include "baselib/Utility.h"
#include "netwrapper/HostInfo.h"
#include "netwrapper/IClient.h"
#include "netwrapper/IClientCallback.h"
#include "baselib/TaskThread.h"

namespace hq {
namespace stun {

class DownloadAlogrithm: public IClientCallback {
public:
    DownloadAlogrithm();
    virtual ~DownloadAlogrithm();

    struct RequestInfo {
        uint16_t   msg_id;
        uint32_t   msg_len;

        RequestInfo() {
            memset(this, 0, sizeof(RequestInfo));
        }
    };

    struct ResponseInfo {
        uint16_t  msg_id;
        uint32_t msg_len;
        uint8_t  msg_buff[1];

        ResponseInfo() {
            memset(this, 0, sizeof(*this));
        }
    };

    void init(IClientPtr client_ptr, const HostInfo &remote_server);
    int start(HostInfo &stun_server);
    int stop();

    virtual void handle_read(const HostInfo& remote_host_info, BufferUtilityPtr read_buffer_ptr, std::size_t size);
    virtual void handle_send(const HostInfo& remote_host_info, std::size_t size);
    virtual void handle_error(const int errror_code);

protected:
    BufferUtilityPtr construct_req_message(const int cmd);
    BufferUtilityPtr construct_resp_message(const int cmd);
    int send_request_message(HostInfo& host_info);
    int send_response_message(HostInfo& host_info);
    void handle_request_message(RequestInfo &request);
    void handle_response_message(ResponseInfo &response);

protected:
    std::weak_ptr<IClient>   client_ptr_;
    std::atomic<bool>         is_start_;
    BufferUtilityPtr request_buffer_ptr_;
    BufferUtilityPtr response_buffer_ptr_;
    HostInfo remote_server_;
    uint64_t start_send_ticks_;
    uint64_t recv_bytes_;
    thread::TaskThread    task_thread_;
};

typedef std::shared_ptr<DownloadAlogrithm> DownloadAlogrithmPtr;

}
}