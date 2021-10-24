#pragma once
#include <iostream>
#include "netwrapper/HostInfo.h"
#include "netwrapper/ISocketMgr.h"
#include "netwrapper/IClient.h"
#include <Windows.h>
//#include <synchapi.h>
#include <chrono>
#include <thread>
#include <baselib/BufferUtility.h>
#include "baselib/Utility.h"
#include "netwrapper/ITimerCallback.h"
#include "../MessageController.h"
#include "../ConfigParse.h"

namespace hq {

class Manager: public ITimerCallback, public std::enable_shared_from_this<Manager> {
public:
    Manager();
    virtual ~Manager();

    int start();
    int stop();

    virtual void timerCallback(const int timer_id);

protected:
    int readConfig();
    int startServerFromConfigFile();
    int startClientFromConfigFile();
    int stopServer();
    int stopClient();

private:
    ISocketMgrPtr   socket_manager_ptr_;
    ConfigParse config_parse_;
    ConfigParse::HostInfoList tcp_host_info_list_;
    ConfigParse::HostInfoList udp_host_info_list_;
    std::map<std::string, IClientPtr>  tcp_client_map_;
    std::map<std::string, IClientPtr>  tcp_server_map_;
    std::map<std::string, IClientPtr>  udp_client_map_;
    std::map<std::string, IClientPtr>  udp_server_map_;
    int   timer_id_;
};

typedef std::shared_ptr<Manager> ManagerPtr;

}