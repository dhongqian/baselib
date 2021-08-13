#ifndef _HQ_BASELIB_TASK_THREAD_H_
#define _HQ_BASELIB_TASK_THREAD_H_


#include <string>
#include <algorithm>
#include <list>
#include <thread>
#include <mutex>
#include "FuncWrapper.h"
#include "Utility.h"

namespace hq{
namespace thread {

class baselib_declspec IThread {
public:
    IThread() {};
    virtual ~IThread() {};

    virtual int exec() = 0;
};

}
}

#endif ///< _HQ_BASELIB_TASK_THREAD_H_