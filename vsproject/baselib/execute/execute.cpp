// execute.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "netwrapper/ISocketMgr.h"
#include "netwrapper/IClient.h"
#include <Windows.h>
//#include <synchapi.h>
#include <chrono>
#include <thread>
#include <baselib/BufferUtility.h>
#include "baselib/Utility.h"
#include "MessageController.h"
#include "ConfigParse.h"

using namespace hq;

int main(int argc, char **argv)
{
    //std::cout << "Hello World!\n";
    if(argc < 6) {
        std::cout << "argv[0] stun_server_ip stun_server_port interval(seconds) remote_ip remote_port local_port(defalut is 32586)\n";
        return 0;
    }

    int interval = atoi(argv[3]);
    int stun_port = atoi(argv[2]);
    int remote_port = atoi(argv[5]);
    int local_port = 32586;
    if(argc > 6) {
        local_port = atoi(argv[6]);
    }
    if(interval < 1) {
        std::cout << " error argument: " << argv[2] << ", " << argv[3] << std::endl;
        return -1;
    }
    std::cout << "interval is: " << interval << ", stun_server is: " << argv[1] << ":" << stun_port << std::endl;
    LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), "interval is: " << interval << ", stun_server is: " << argv[1] << ":" << stun_port << ", remote_ip is: " << argv[4] << ":" << remote_port << ", local_port: " << local_port);

    auto socket_manager = hq::createSocketMgr();
    if(nullptr == socket_manager) return -1;

    hq::utility::logInit("log.property");

    hq::ConfigParse config_parse;
    config_parse.readConfig("config.json");
    hq::ConfigParse::HostInfoList udp_host_info_list, tcp_host_info_list;
    config_parse.getHostInfoList(ConfigParse::ConfigHostType::TCP, tcp_host_info_list);
    config_parse.getHostInfoList(ConfigParse::ConfigHostType::UDP, udp_host_info_list);

    std::for_each(tcp_host_info_list.begin(), tcp_host_info_list.end(), [](ConfigParse::HostInfo& host_info) {
        std::cout << " address: " << host_info.address
            << ", port: " << host_info.port
            << std::endl;
    });

    std::for_each(udp_host_info_list.begin(), udp_host_info_list.end(), [](ConfigParse::HostInfo& host_info) {
        std::cout << " address: " << host_info.address
            << ", port: " << host_info.port
            << std::endl;
    });

    socket_manager->start();
    //socket_manager->start_udp_client(nullptr, client_ptr);
   // std::string ip("127.0.0.1");
    //hq::stun::StunAlogithmPtr stun_algorithm = std::make_shared<hq::stun::StunAlogrithm>();
    hq::MessageControllerPtr message_controller = std::make_shared<MessageController>();
    hq::HostInfo host_info(INADDR_ANY, local_port);
    //hq::IClientPtr client_ptr = socket_manager->start_udp_server(host_info, stun_algorithm);
    hq::IClientPtr client_ptr = socket_manager->startUdpServer(host_info, message_controller);

    HostInfo remote_server(argv[4], remote_port);
    assert(nullptr != message_controller);
    //stun_algorithm->init(client_ptr, remote_server);
    message_controller->init(client_ptr, remote_server);
    hq::HostInfo stun_server(argv[1], stun_port);
    //socket_manager->start_timer(2, 0, stun_algorithm);
    socket_manager->startTimer(2, 0, message_controller);

    //socket_manager->stop();
    //auto send_client_ptr = socket_manager->start_udp_client(nullptr);
    //hq::HostInfo host_info_2(ip, 5890);
    //hq::IClientPtr send_client_ptr = socket_manager->start_udp_server(host_info_2, nullptr);

    //std::string str("who are you!");
    message_controller->start(stun_server);
    while(true) {
        //std::this_thread::sleep_for(std::chrono::duration<int>(1));
        //if(nullptr == send_client_ptr) break;
        //BufferUtilityPtr buffer_ptr = std::make_shared<BufferUtility>((uint8_t*)str.c_str(), str.size());
        //HostInfo host_info(ip, 5880);
        //buffer_ptr->set_data_len(str.size());
        //int ret = client_ptr->send(host_info, buffer_ptr);
        //printf("%d\n", ret);
        
        //std::cout << "cur seconds: " << time(NULL) << std::endl;
        Sleep(1000 * interval);
    }

    return 0;

}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
