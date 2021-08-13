// cmdNotify.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
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
#include "CmdMsgController.h"

using namespace hq;

int main(int argc, char** argv)
{
    //std::cout << "Hello World!\n";
    if(argc < 5) {
        std::cout << "argv[0] server_ip server_port ip port\n";
        std::cout << " notify server_ip:server_port to send msg to ip:port\n";
        return 0;
    }

    int remote_port = atoi(argv[2]);
    int port = atoi(argv[4]);
    std::cout << "server is: " << argv[1] << ": " << remote_port << ", ip is: " << argv[3] << ":" << port << std::endl;
    LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), "server is: " << argv[1] << ": " << remote_port << ", ip is: " << argv[3] << ":" << port);

    auto socket_manager = hq::createSocketMgr();
    if(nullptr == socket_manager) return -1;

    hq::utility::logInit("log.property");

    socket_manager->start();
    CmdMsgControllerPtr message_controller = std::make_shared<hq::CmdMsgController>();
    hq::IClientPtr client_ptr = socket_manager->startUdpClient(message_controller);

    assert(nullptr != message_controller);
    message_controller->init(client_ptr);
    hq::HostInfo remote_server(argv[1], remote_port);
    hq::HostInfo host_info(argv[3], port);

    message_controller->start(remote_server, host_info);
    while(true) {
        //std::this_thread::sleep_for(std::chrono::duration<int>(1));

        Sleep(1000 * 10);
        //break;
    }
    socket_manager->stop();
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
