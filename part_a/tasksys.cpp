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
    _cur_idx = 0;
    _num_total_tasks = 0;
    std::mutex _lock;
    _run_flag = false;
    _kill_flag = false;
    for (auto& thread : _thread_pool)
    {
        thread = std::move(
            std::thread(&TaskSystemParallelThreadPoolSpinning::ImplRun, this, _run , std::ref(_lock), std::ref(_cur_idx),
                std::ref(_num_total_tasks), std::ref(_run_flag), std::ref(_kill_flag)));
    }
}


std::condition_variable cv;
std::mutex m;
void TaskSystemParallelThreadPoolSpinning::ImplRun(IRunnable* runable, std::mutex& lock,
    int& cur_idx, int &num_total_tasks, bool &runflag, bool& kill_flag)
{
    while (1)
    {
        std::cout << runflag << std::endl;
        if (runflag)
        {
            int idx = -1;
            while (idx < num_total_tasks)
            {
                lock.lock();
                idx = cur_idx;
                cur_idx++;
                std::cout << "run idx " << idx << std::endl; 
                lock.unlock();
                if (idx >= num_total_tasks){
                    std::unique_lock<std::mutex> lock(m);
                    cv.notify_all();
                    break;
                }

                runable->runTask(idx, num_total_tasks);
            }
            if (kill_flag)
                return;
        }
    }
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
    _kill_flag = true;
    for (auto &x: _thread_pool){
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

    // for (int i = 0; i < num_total_tasks; i++)
    // {
    //     runnable->runTask(i, num_total_tasks);
    // }
    _run = runnable;
    _cur_idx = 0;
    _num_total_tasks = num_total_tasks;
    _run_flag = true;
    std::cout << num_total_tasks << " run" << std::endl;

    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock);
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
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping()
{
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks)
{
    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    for (int i = 0; i < num_total_tasks; i++)
    {
        runnable->runTask(i, num_total_tasks);
    }
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
