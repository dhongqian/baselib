/*
 * @Author: your name
 * @Date: 2021-05-12 16:40:13
 * @LastEditTime: 2021-05-14 18:32:39
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \base_lib\src\string_utility.cpp
 */
#include "baselib/TaskThread.h"
#include <iterator>
#include <iostream>
#include "baselib/Utility.h"

namespace hq{
namespace thread {

TaskThread::TaskThread(const std::string& thread_name): is_run_(false), thread_ptr_(nullptr), thread_name_(thread_name) {

}

TaskThread::~TaskThread() {

}

int TaskThread::start() {
    if(is_run_) return 0;
    is_run_ = true;
    thread_ptr_ = new std::thread(std::bind(&TaskThread::loop, this));
    if(nullptr == thread_ptr_) {
        is_run_ = false;
        return -1;
    }
    return 0;
}

int TaskThread::stop() {
    if(!is_run_) return 0;
    is_run_ = false;
    thread_ptr_->join();
    delete thread_ptr_;
    thread_ptr_ = nullptr;

    std::lock_guard<std::mutex>  lock_guard(add_list_mutex_);
    adding_task_list_.clear();

    return 0;
}

int TaskThread::postTask(Closure task) {
    {
        //std::cout << "postTask 1\n";
        std::lock_guard<std::mutex>  lock_guard(add_list_mutex_);
        adding_task_list_.push_back(task);
    }
    {
        //std::cout << "postTask 2\n";
        std::unique_lock <std::mutex> lock(condition_mutex_);
        condition_var_.notify_all();
    }
    //std::cout << "postTask 3\n";

    return 0;
}

int TaskThread::postDelayTask(Closure task, const uint64_t milli_seconds) {
    {
        //std::cout << "PostDelayTask 1\n";
        std::lock_guard<std::mutex> lock_guard(add_delay_task_map_mutex_);
        uint64_t delay_interval = GetTickCount64() + milli_seconds;
        add_delay_task_map_.insert(std::make_pair(delay_interval, task));
    }

    {
        //std::cout << "PostDelayTask 2\n";
        std::unique_lock <std::mutex> lock(condition_mutex_);
        condition_var_.notify_all();
    }
    //std::cout << "PostDelayTask 3\n";

    return 0;
}

int TaskThread::loop() {
    uint64_t timeout_ticks = 0;
    while(is_run_) {
        //std::cout << "run 1\n";
        LOG4CPLUS_DEBUG(log4cplus::Logger::getInstance("baselib"), "begin.");
        runingTask();
        if(!is_run_) break;
        adding2RunningTask();
        adding2DelayTask();

        //std::cout << "run 2\n";
        if(running_task_list_.empty()) {
            //std::cout << "run 3\n";
            if(!delay_task_map_.empty()) {
                //std::cout << "run 4\n";
                auto it = delay_task_map_.begin();
                timeout_ticks = it->first - GetTickCount64();
            }
            else {
                timeout_ticks = 100;
            }

            //std::cout << "run 5 " << timeout_ticks << " \n";
            std::unique_lock <std::mutex> lock(condition_mutex_);
            condition_var_.wait_for(lock, std::chrono::microseconds(timeout_ticks));
            //std::cout << "run 6\n";
        }
    }
    running_task_list_.clear();
    delay_task_map_.clear();
    //std::cout << "run exit \n";
    LOG4CPLUS_DEBUG(log4cplus::Logger::getInstance("baselib"), "exit.");

    return 0;
}

void TaskThread::runingTask() {
    for(auto it = running_task_list_.begin(); it != running_task_list_.end();) {
        Closure task_ptr = *it;
        task_ptr->Run();
        if(!is_run_) break;
        running_task_list_.erase(it++);
    }
}

void TaskThread::adding2RunningTask() {
    std::lock_guard<std::mutex>  lock_guard(add_list_mutex_);
    std::copy(adding_task_list_.begin(), adding_task_list_.end(), std::back_inserter(running_task_list_));
}

void TaskThread::adding2DelayTask() {
    {
        std::lock_guard<std::mutex> lock_guard(add_delay_task_map_mutex_);
        for(auto item : add_delay_task_map_) {
            delay_task_map_.insert(item);
        }
        add_delay_task_map_.clear();
    }

    uint64_t cur_ticks = GetTickCount64();
    for(auto it = delay_task_map_.begin(); it != delay_task_map_.end();) {
        if(it->first > cur_ticks) {
            break;
        }
        running_task_list_.push_back(it->second);
        delay_task_map_.erase(it++);
    }
}

}
}
