#pragma once
#ifndef HQ_EXECUTE_CONFIG_PARSE_H_
#define HQ_EXECUTE_CONFIG_PARSE_H_

#include <string>
#include <list>
#include <jsoncpp/json.h>

namespace hq {
class ConfigParse final {
public:
    ConfigParse();
    ~ConfigParse();

    enum class ConfigHostType {
        UDP,
        TCP,
        ConfigHostTypeEnd
    };

    struct HostInfo {
        std::string address;
        uint16_t    port;
        ConfigHostType type;

        HostInfo(): port(0), type(ConfigHostType::UDP) {}
    };

    typedef std::list<HostInfo>   HostInfoList;
    int readConfig(const std::string& file_name);
    int getHostInfoList(const ConfigHostType host_type, HostInfoList& host_list);

protected:
    int getHostList(const char* key, Json::Value& root, HostInfoList& host_info_list);

protected:
    HostInfoList  server_list_;
    HostInfoList  client_list_;
};
}

#endif  ///< HQ_EXECUTE_CONFIG_PARSE_H_
