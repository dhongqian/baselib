#include "SocketMgr.h"
#include "TimerMgr.h"
#include "netwrapper/Utility.h"
#include "Client.h"

namespace hq {

static void timer_callback(int sock, short event, void* arg);

//////////////////////////////////////////////////////////////////////////

TimerMgr::TimerMgr(std::shared_ptr<SocketMgr> socket_mgr_ptr) {
    socket_mgr_ptr_ = socket_mgr_ptr;
    for(int i = 0; i < TIMER_INFO_COUNT; ++i) {
        timer_info_arr_[i] = nullptr;
    }
}

TimerMgr::~TimerMgr() {
}

int TimerMgr::startTimer(const int seconds, const int useconds, ITimerCallbackPtr callback_ptr) {
    if((seconds <= 0 && useconds < 0) || (nullptr == callback_ptr)) {
        return -1;
    }

    int timer_id = 0;
    {
        std::lock_guard<std::mutex> lock_guard(timer_arr_mutex_);
        for(; timer_id < TIMER_INFO_COUNT; ++timer_id) {
            if(nullptr == timer_info_arr_[timer_id]) break;
        }
    }
    if(timer_id == TIMER_INFO_COUNT) {
        return -1;
    }
    TimerEventInfo* timer_info = new(std::nothrow) TimerEventInfo();
    if(nullptr == timer_info) {
        return -1;
    }

    auto socket_mgr_ptr = socket_mgr_ptr_.lock();
    assert(nullptr != socket_mgr_ptr);
    timer_info->tv.tv_sec = seconds;
    timer_info->tv.tv_usec = useconds;
    timer_info->timer_event = event_new(socket_mgr_ptr->event_base_, -1, 0, timer_callback, timer_info);
    if(nullptr == timer_info->timer_event) {
        delete timer_info;
        return -1;
    }
    //evtimer_set(timer_info->timer_event, )
    event_add(timer_info->timer_event, &timer_info->tv);

    timer_info->timer_id = timer_id;
    timer_info->timer_mgr = this;
    timer_info->timer_callback = callback_ptr;

    /// @brief need lock 
    std::lock_guard<std::mutex> lock_guard(timer_arr_mutex_);
    timer_info_arr_[timer_id] = timer_info;

    return timer_id;
}

int TimerMgr::stopTimer(const int timer_id) {
    if(timer_id >= TIMER_INFO_COUNT || timer_id < 0) {
        return -1;
    }

    // need lock
    //::assert(timer_id == timer_info->timer_id);
    std::lock_guard<std::mutex> lock_guard(timer_arr_mutex_);
    TimerEventInfo* timer_info = timer_info_arr_[timer_id];
    if(0 == timer_info) return 0;
    if(nullptr != timer_info->timer_event) {
        evtimer_del(timer_info->timer_event);
        event_free(timer_info->timer_event);
        timer_info->timer_event = nullptr;
    }

    delete timer_info;
    timer_info_arr_[timer_id] = nullptr;
    return 0;
}

int TimerMgr::stopAllTimer() {
    for(int i = 0; i < TIMER_INFO_COUNT; ++i) {
        stopTimer(i);
    }
    return 0;
}


////////////////////////////////////////////////////////////////////
// 定时事件回调函数
void timer_callback(int sock, short event, void* arg) {
    // 重新添加定时事件（定时事件触发后默认自动删除）
    TimerEventInfo* timer_info = (TimerEventInfo*)arg;
    if(nullptr == timer_info) {
        return;
    }

    if(nullptr != timer_info->timer_mgr && nullptr != timer_info->timer_callback) {
        timer_info->timer_callback->timerCallback(timer_info->timer_id);
    }

    if(nullptr != timer_info->timer_event) {
        event_add(timer_info->timer_event, &timer_info->tv);
    }
}

}
