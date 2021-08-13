/*
 * @Author: your name
 * @Date: 2021-05-26 19:21:03
 * @LastEditTime: 2021-05-28 19:11:13
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \baselib\src\netwrapper\SocketMgr.h
 */
#pragma once

#include <memory>
#include <thread>
extern "C" {
#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/thread.h>

#ifndef WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#else
#include <Windows.h>
#include <WinSock2.h>
#pragma comment(lib,"Ws2_32.lib ")
#include <Iphlpapi.h>
#pragma  comment(lib, "Iphlpapi.lib")
#include <io.h>
#endif
}

namespace hq {

namespace net {


}

}

