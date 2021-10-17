/*
 * @Author: your name
 * @Date: 2021-05-28 16:46:43
 * @LastEditTime: 2021-05-28 18:04:41
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \baselib\src\netwrapper\Client.h
 */
#pragma once
#ifndef __HQ_NET_WRAPPER_TIMER_MANAGER_H__

#include <string>
#include <memory>
#include <iostream>
#include "ClientCallback.h"
#include "netwrapper/IClient.h"

namespace hq {

class TimerMgr;
struct TimerEventInfo {
    int timer_id;
    timeval tv;
    event* timer_event;
    TimerMgr *timer_mgr;
    ITimerCallbackPtr timer_callback;

    TimerEventInfo(): timer_id(-1), timer_event(nullptr), timer_mgr(nullptr) {
        tv.tv_sec = 0;
        tv.tv_usec = 0;
    }
};

class SocketMgr;
class baselib_declspec TimerMgr {
public:
    TimerMgr(std::shared_ptr<SocketMgr> socket_mgr_ptr);
    virtual ~TimerMgr();

    virtual int startTimer(const int seconds, const int useconds, ITimerCallbackPtr callback_ptr);
    virtual int stopTimer(const int timer_id);
    virtual int stopAllTimer();

protected:
    std::weak_ptr<SocketMgr>         socket_mgr_ptr_;
    TimerEventInfo* timer_info_arr_[TIMER_INFO_COUNT];
    std::mutex             timer_arr_mutex_;
};
};

#endif ///< __HQ_NET_WRAPPER_TIMER_MANAGER_H__
