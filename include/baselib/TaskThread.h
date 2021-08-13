#ifndef _HQ_BASELIB_TASK_THREAD_H_
#define _HQ_BASELIB_TASK_THREAD_H_

#include <string>
#include <algorithm>
#include <list>
#include <thread>
#include <mutex>
#include <map>>
#include <condition_variable>
#include "FuncWrapper.h"
#include "Utility.h"

namespace hq{
namespace thread {

class baselib_declspec TaskThread {
public:
    TaskThread(const std::string &thread_name);
    virtual ~TaskThread();

    virtual int start();
    virtual int stop();

    virtual int postTask(Closure task);
    virtual int postDelayTask(Closure task, const uint64_t milli_seconds);

protected:
    virtual int loop();
    void runingTask();
    void adding2RunningTask();
    void adding2DelayTask();

protected:
    std::mutex                 condition_mutex_;
    std::condition_variable    condition_var_;
    volatile bool                      is_run_;
    std::thread* thread_ptr_;
    std::mutex                add_list_mutex_;
    std::list<Closure>        adding_task_list_;
    std::list<Closure>        running_task_list_;
    std::mutex                add_delay_task_map_mutex_;
    std::multimap<uint64_t, Closure>        delay_task_map_;
    std::multimap<uint64_t, Closure>        add_delay_task_map_;

private:
    std::string              thread_name_;

};

typedef std::shared_ptr<TaskThread>  TaskThreadPtr;

}
}

#endif ///< _HQ_BASELIB_TASK_THREAD_H_