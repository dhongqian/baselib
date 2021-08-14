/*
 * @Author: your name
 * @Date: 2021-05-26 19:20:01
 * @LastEditTime: 2021-05-28 18:05:16
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: \baselib\include\netwrapper\IClientCallback.h
 */
#pragma once
#ifndef __HQ_NET_WRAPPER_ITIMER_CALLBACK_H__

#include <string>
#include <memory>
#include "baselib/Utility.h"

namespace hq {

class baselib_declspec ITimerCallback {
public:
    ITimerCallback() = default;
    virtual ~ITimerCallback() {};

    virtual void timerCallback(const int timer_id) = 0;
};

typedef std::shared_ptr<ITimerCallback>  ITimerCallbackPtr;

};

#endif ///< __HQ_NET_WRAPPER_ITIMER_CALLBACK_H__
