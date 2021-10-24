#pragma once
#include <string>
#include <atomic>
#include "netwrapper/HostInfo.h"
#include "netwrapper/IClient.h"
#include "netwrapper/IClientCallback.h"
#include "netwrapper/ITimerCallback.h"
#include "baselib/Utility.h"
#include "cmd/UdpCmd.h"

namespace hq {

class MessageController: public IClientCallback, public ITimerCallback{
public:
    MessageController();
    virtual ~MessageController();

    void init(IClientPtr client_ptr, const HostInfo &remote_server);
    int start(HostInfo &stun_server);
    int stop();

    virtual void handleRead(const HostInfo& remote_host_info, BufferUtilityPtr read_buffer_ptr, std::size_t size);
    virtual void handleSend(const HostInfo& remote_host_info, std::size_t size);
    virtual void handleError(const int errror_code);

    virtual void timerCallback(const int timer_id);

protected:
    IClientPtr   client_ptr_;
    bool         is_start_;
    uint64_t    begin_ticks_;
    BufferUtilityPtr buffer_ptr_;
    UdpCmdPtr   udp_cmd_ptr_;
};

typedef std::shared_ptr<MessageController> MessageControllerPtr;

}