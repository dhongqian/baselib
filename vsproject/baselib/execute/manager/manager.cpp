#include "manager.h"
#include "ManagerServerCallback.h"

namespace hq {

Manager::Manager(): timer_id_(-1) {

}

Manager::~Manager() {
    stop();
}

int Manager::start() {
    socket_manager_ptr_ = hq::createSocketMgr();
    if(nullptr == socket_manager_ptr_) return -1;
    int ret = socket_manager_ptr_->start();
    if(0 != ret) {
        socket_manager_ptr_->stop();
        socket_manager_ptr_.reset();
    }

    readConfig();
    startServerFromConfigFile();
    startClientFromConfigFile();

    timer_id_ = socket_manager_ptr_->startTimer(1, 0, shared_from_this());

    return -1;
}

int Manager::stop() {
    if(nullptr == socket_manager_ptr_) return 0;
    socket_manager_ptr_->stopTimer(timer_id_);
    stopClient();
    stopServer();
    if(nullptr != socket_manager_ptr_) {
        socket_manager_ptr_->stop();
    }
    socket_manager_ptr_.reset();
    return 0;
}

int Manager::readConfig() {
    config_parse_.readConfig("config.json");
    //hq::ConfigParse::HostInfoList udp_host_info_list, tcp_host_info_list;
    config_parse_.getHostInfoList(ConfigParse::ConfigHostType::TCP, tcp_host_info_list_);
    config_parse_.getHostInfoList(ConfigParse::ConfigHostType::UDP, udp_host_info_list_);

    std::for_each(tcp_host_info_list_.begin(), tcp_host_info_list_.end(), [](ConfigParse::ConfigHostInfo& host_info) {
        std::cout << " TCP: address: " << host_info.address
            << ", port: " << host_info.port << ", is client: " << (host_info.is_client?"yes":"no")
            << std::endl;
    });

    std::for_each(udp_host_info_list_.begin(), udp_host_info_list_.end(), [](ConfigParse::ConfigHostInfo& host_info) {
        std::cout << " UDP: address: " << host_info.address
            << ", port: " << host_info.port << ", is client: " << (host_info.is_client ? "yes" : "no")
            << std::endl;
    });

    return 0;
}

int Manager::startServerFromConfigFile() {
    auto server_callback = std::make_shared<ManagerServerCallback>();
    for(auto& item : tcp_host_info_list_) {
        if(item.is_client) continue;
        socket_manager_ptr_->startTcpServer(item.getHostInfo(), server_callback);
    }

    for(auto& item : udp_host_info_list_) {
        if(item.is_client) continue;
        HostInfo host_info = item.getHostInfo();
        std::string key = host_info.getIPString() + ":" + std::to_string(host_info.getPort());
        auto it = udp_server_map_.find(key);
        if(it != udp_server_map_.end()) continue;
        IClientPtr client_ptr = socket_manager_ptr_->startUdpServer(host_info, nullptr);
        if(nullptr != client_ptr) {
            udp_server_map_[key] = client_ptr;
        }
    }

    return 0;
}

int Manager::startClientFromConfigFile() {
    for(auto& item : tcp_host_info_list_) {
        if(!item.is_client) continue;
        HostInfo host_info = item.getHostInfo();
        std::string key = host_info.getIPString() + ":" + std::to_string(host_info.getPort());
        auto it = tcp_client_map_.find(key);
        if(it != tcp_client_map_.end()) continue;
        IClientPtr client_ptr = socket_manager_ptr_->startTcpClient(host_info, nullptr);
        if(nullptr != client_ptr) {
            tcp_client_map_[key] = client_ptr;
        }
    }

    for(auto& item : udp_host_info_list_) {
        if(!item.is_client) continue;
        HostInfo host_info = item.getHostInfo();
        std::string key = host_info.getIPString() + ":" + std::to_string(host_info.getPort());
        auto it = udp_client_map_.find(key);
        if(it != udp_client_map_.end()) continue;
        IClientPtr client_ptr = socket_manager_ptr_->startUdpClient(nullptr);
        if(nullptr != client_ptr) {
            udp_client_map_[key] = client_ptr;
        }
    }

    return 0;
}

int Manager::stopServer() {
    for(auto it = tcp_server_map_.begin(); it != tcp_server_map_.end(); ++it) {
        socket_manager_ptr_->stopTcpServer(0);
    }
    tcp_server_map_.clear();

    for(auto it = udp_server_map_.begin(); it != udp_server_map_.end(); ++it) {
        socket_manager_ptr_->stopUdpServer(it->second);
    }
    udp_server_map_.clear();

    return 0;
}

int Manager::stopClient() {
    for(auto it = tcp_client_map_.begin(); it != tcp_client_map_.end(); ++it) {
        uint64_t client_id = reinterpret_cast<uint64_t>(it->second.get());
        socket_manager_ptr_->stopClient(client_id);
    }
    tcp_client_map_.clear();

    for(auto it = udp_client_map_.begin(); it != udp_client_map_.end(); ++it) {
        uint64_t client_id = reinterpret_cast<uint64_t>(it->second.get());
        socket_manager_ptr_->stopClient(client_id);
    }
    udp_client_map_.clear();
    return 0;
}

void Manager::timerCallback(const int timer_id) {
    BufferUtilityPtr buffer_ptr = std::make_shared<BufferUtility>(1500);
    if(nullptr == buffer_ptr) return;

    std::ostringstream oss;
    oss << " send data: " << GetTickCount();
    memcpy(buffer_ptr->getBuffer(), oss.str().c_str(), oss.str().length());
    buffer_ptr->setDataLen(oss.str().length());

    for(auto item : tcp_client_map_) {
        item.second->send(buffer_ptr);
    }
}

}