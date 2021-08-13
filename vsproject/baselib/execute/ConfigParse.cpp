#include "ConfigParse.h"
#include <baselib/Utility.h>
#include <jsoncpp/reader.h>


namespace hq {

ConfigParse::ConfigParse() {

}

ConfigParse::~ConfigParse() {

}

/*
 {
    "server": [{
        "ip": "127.0.0.1",
        "port": 56880,
        "type": "tcp"
    },
    {
        "ip": "127.0.0.1",
        "port": 56880,
        "type": "udp"
    }
    ],
    "client": [{
        "ip": "127.0.0.1",
        "port": 56880,
        "type": "tcp"
    }
    <!-- client no udp protocol -->
    ]
}


 **/

int ConfigParse::readConfig(const std::string& file_name) {
#if 1
    FILE* fp = fopen(file_name.c_str(), "rb");
    if(nullptr == fp) {
        LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), " open file " << file_name << " failed.");
        return -1;
    }

    assert(0 == fseek(fp, 0, SEEK_END));
    long file_size = ftell(fp);
    assert(file_size >= 0);
    char* file_buffer = new char[file_size + 1];
    memset(file_buffer, 0, file_size + 1);
    assert(0 == fseek(fp, 0, SEEK_SET));
    size_t read_size = fread_s(file_buffer, file_size+1, file_size, 1, fp);
    assert(read_size == 1);
    fclose(fp);

    //Json::Reader json_reader;
    //Json::Value  json_root;
    //if(!json_reader.parse(file_buffer, json_root)) {
        //LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), " parse json file string " << file_name << " //failed.");
        //return -1;
    //}
#endif

    //std::ifstream ifs(file_name);
    Json::CharReaderBuilder b;
    Json::CharReader *json_reader = b.newCharReader();
    //Json::Reader json_reader;
    // json_reader->parse(ifs, json_root)
    Json::Value  json_root;
    if(!json_reader->parse(file_buffer, file_buffer+file_size, &json_root, nullptr)) {
        LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), " parse json file " << file_name << " failed.");
        return -1;
    }

    int ret = getHostList("server", json_root, server_list_);
    if(0 != ret) {
        LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), " get server host array failed.");
    }
    ret = getHostList("client", json_root, client_list_);
    if(0 != ret) {
        LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), " get client host array failed.");
    }

    delete json_reader;
    delete[]file_buffer;

    return 0;
}

int ConfigParse::getHostList(const char* key, Json::Value& root, ConfigParse::HostInfoList& host_info_list) {
    if(!root.isMember(key)) {
        LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), " server " << key << "is not exist ");
        return -1;
    }
    Json::Value json_tcp_array = root[key];
    if(!json_tcp_array.isArray()) {
        LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), " server is not array ");
        return -1;
    }

    for(size_t i = 0; i < json_tcp_array.size(); ++i) {
        Json::Value item = json_tcp_array[i];
        HostInfo host_info;
        host_info.address = (item.isMember("ip") && item["ip"].isString()) ? item["ip"].asString() : "";
        host_info.port = (item.isMember("port") && item["port"].isInt()) ? item["port"].asInt() : 0;
        std::string type = (item.isMember("type") && item["type"].isString()) ? item["type"].asString() : "";
        if(host_info.address.empty() || type.empty()) continue;
        if(type == "tcp" || type == "udp") {
            host_info.type = (type == "tcp") ? ConfigHostType::TCP : ConfigHostType::UDP;
            host_info_list.push_back(host_info);
        }
    }

    return 0;
}

int ConfigParse::getHostInfoList(const ConfigHostType host_type, HostInfoList& host_list) {
    if(ConfigHostType::TCP == host_type) {
        //std::copy(tcp_host_list_.begin(), tcp_host_list_.end(), host_list);
        for(auto it = server_list_.begin(); it != server_list_.end(); ++it) {
            host_list.push_back(*it);
        }
    }
    else if(ConfigHostType::UDP == host_type) {
        //std::copy(udp_host_list_.begin(), udp_host_list_.end(), host_list);
        for(auto it = client_list_.begin(); it != client_list_.end(); ++it) {
            host_list.push_back(*it);
        }
    }
    return 0;
}

}
