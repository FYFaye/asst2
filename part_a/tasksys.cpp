#include "tasksys.h"
#include <condition_variable>
#include <iostream>

IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char* TaskSystemSerial::name()
{
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads)
    : ITaskSystem(num_threads)
{
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks)
{
    for (int i = 0; i < num_total_tasks; i++)
    {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(
    IRunnable* runnable, int num_total_tasks, const std::vector<TaskID>& deps)
{
    return 0;
}

void TaskSystemSerial::sync()
{
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelSpawn::name()
{
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads)
    : ITaskSystem(num_threads)
{
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    _thread_pool.resize(num_threads);
    // for (auto thread: _thread_pool){

    // }
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks)
{
    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    std::mutex lock;
    int cur_idx = 0;
    for (auto& thread : _thread_pool)
    {
        thread = std::move(std::thread(&TaskSystemParallelSpawn::ImplRun, this, runnable,
            std::ref(lock), std::ref(cur_idx), num_total_tasks));
    }
    for (auto& thread : _thread_pool)
    {
        thread.join();
    }
}

void TaskSystemParallelSpawn::ImplRun(
    IRunnable* runable, std::mutex& lock, int& cur_idx, int num_total_tasks)
{
    int idx = -1;
    while (idx < num_total_tasks)
    {
        lock.lock();
        idx = cur_idx;
        cur_idx++;
        lock.unlock();
        if (idx >= num_total_tasks)
            break;
        runable->runTask(idx, num_total_tasks);
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(
    IRunnable* runnable, int num_total_tasks, const std::vector<TaskID>& deps)
{
    return 0;
}

void TaskSystemParallelSpawn::sync()
{
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinning::name()
{
    return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads)
    : ITaskSystem(num_threads)
{
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    _thread_pool.resize(num_threads);
    for (auto& thread : _thread_pool)
    {
        thread = std::move(std::thread(
            &TaskSystemParallelThreadPoolSpinning::ImplRun, this, std::ref(_run_status)));
    }
}

std::condition_variable cv;
std::mutex m;
bool wait_flag = false;

void TaskSystemParallelThreadPoolSpinning::ImplRun(RunStatus& status)
{
    while (1)
    {
        int idx;
        if (status._run_flag)
        {
            // wait_flag = true;
            status._mutex.lock();
            idx = status._cur_idx;
            status._cur_idx++;
            status._mutex.unlock();
            // std::cout << std::hex << status._runnable << std::endl;
            // confirm
            if (idx < status._num_total_tasks)
            {

                status._runnable->runTask(idx, status._num_total_tasks);
                status._mutex.lock();
                std::cout << "TID [" << std::this_thread::get_id() << "], [runid, total] = [" << idx
                          << ", " << status._num_total_tasks << "]" << std::endl;
                status._left_tasks_cnt--;
                status._mutex.unlock();
                if (status._left_tasks_cnt == 0)
                {
                    // std::lock_guard<std::mutex> lk(status._not_mutex);
                    // // status._left_tasks_cnt = status._num_total_tasks;
                    // // status._cur_idx = 0;
                    // status._run_flag = false;
                    std::cout << "TID [" << std::this_thread::get_id() << "] notify_all"
                              << std::endl;
                    cv.notify_all();
                }
            }
        }
        if (status._kill_flag)
            return;
    }
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning()
{
    _run_status._kill_flag = true;
    for (auto& x : _thread_pool)
    {
        x.join();
    }
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks)
{
    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    _run_status._mutex.lock();
    _run_status._runnable = runnable;
    _run_status._cur_idx = 0;
    _run_status._num_total_tasks = num_total_tasks;
    _run_status._left_tasks_cnt = num_total_tasks;
    _run_status._mutex.unlock();
    std::unique_lock<std::mutex> lock(m);
    std::cout << "totol task " << num_total_tasks << "Runing" << std::endl;
    _run_status._run_flag = true;
    cv.wait(lock);
    lock.unlock();
    std::cout << "task Done" << std::endl;
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(
    IRunnable* runnable, int num_total_tasks, const std::vector<TaskID>& deps)
{
    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync()
{
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSleeping::name()
{
    return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads)
    : ITaskSystem(num_threads)
{
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    _thread_pool.resize(num_threads);
    for (auto& thread : _thread_pool)
    {
        thread = std::move(std::thread(
            &TaskSystemParallelThreadPoolSleeping::ImplRun, this, std::ref(_run_status)));
    }
}

void TaskSystemParallelThreadPoolSleeping::ImplRun(RunStatus& status)
{
    while (1)
    {
        int idx;
        if (status._run_flag)
        {
            status._mutex.lock();
            idx = status._cur_idx;
            status._cur_idx++;
            status._mutex.unlock();
            // std::cout << std::hex << status._runnable << std::endl;
            if (idx < status._num_total_tasks)
            {
                status._runnable->runTask(idx, status._num_total_tasks);
            }
            else
            {
                std::lock_guard<std::mutex> lk(status._mutex);
                cv.notify_all();
            }
        }
        if (status._kill_flag)
            return;
    }
}
TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping()
{
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    _run_status._kill_flag = true;
    for (auto& x : _thread_pool)
    {
        x.join();
    }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks)
{
    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    _run_status._runnable = runnable;
    _run_status._cur_idx = 0;
    _run_status._num_total_tasks = num_total_tasks;
    std::unique_lock<std::mutex> lock(m);
    // std::cout << "totol task " << num_total_tasks << "Runing" << std::endl;
    _run_status._run_flag = true;
    cv.wait(lock);
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(
    IRunnable* runnable, int num_total_tasks, const std::vector<TaskID>& deps)
{
    //
    // TODO: CS149 students will implement this method in Part B.
    //

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync()
{
    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
