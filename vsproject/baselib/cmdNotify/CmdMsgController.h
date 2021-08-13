#pragma once
#include <string>
#include <atomic>
#include "baselib/Utility.h"
#include "netwrapper/HostInfo.h"
#include "netwrapper/IClient.h"
#include "netwrapper/IClientCallback.h"
#include "netwrapper/ITimerCallback.h"
#include "baselib/TaskThread.h"

namespace hq {

class CmdMsgController: public IClientCallback, public ITimerCallback{
public:
    CmdMsgController();
    virtual ~CmdMsgController();

    void init(IClientPtr client_ptr);
    int start(HostInfo &remote_server, HostInfo &host_info);
    int stop();

    virtual void handleRead(const HostInfo& remote_host_info, BufferUtilityPtr read_buffer_ptr, std::size_t size);
    virtual void handleSend(const HostInfo& remote_host_info, std::size_t size);
    virtual void handleError(const int errror_code);

    virtual void timerCallback(const int timer_id);

protected:
    void notify(HostInfo& remote_server, HostInfo& host_info);

protected:
    IClientPtr   client_ptr_;
    bool         is_start_;
    uint64_t    begin_ticks_;
    BufferUtilityPtr buffer_ptr_;
    hq::thread::TaskThread task_thread_;
};

typedef std::shared_ptr<CmdMsgController> CmdMsgControllerPtr;

}