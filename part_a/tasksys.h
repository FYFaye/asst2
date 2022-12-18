#ifndef _TASKSYS_H
#define _TASKSYS_H

#include "itasksys.h"
#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>

/*
 * TaskSystemSerial: This class is the student's implementation of a
 * serial task execution engine.  See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemSerial : public ITaskSystem
{
public:
    TaskSystemSerial(int num_threads);
    ~TaskSystemSerial();
    const char* name();
    void run(IRunnable* runnable, int num_total_tasks);
    TaskID runAsyncWithDeps(
        IRunnable* runnable, int num_total_tasks, const std::vector<TaskID>& deps);
    void sync();
};

/*
 * TaskSystemParallelSpawn: This class is the student's implementation of a
 * parallel task execution engine that spawns threads in every run()
 * call.  See definition of ITaskSystem in itasksys.h for documentation
 * of the ITaskSystem interface.
 */
class TaskSystemParallelSpawn : public ITaskSystem
{
public:
    TaskSystemParallelSpawn(int num_threads);
    ~TaskSystemParallelSpawn();
    const char* name();
    void run(IRunnable* runnable, int num_total_tasks);
    TaskID runAsyncWithDeps(
        IRunnable* runnable, int num_total_tasks, const std::vector<TaskID>& deps);
    void ImplRun(IRunnable* runable, std::mutex& lock, int& cur_idx, int num_total_tasks);
    void sync();

private:
    std::vector<std::thread> _thread_pool;
};

/*
 * TaskSystemParallelThreadPoolSpinning: This class is the student's
 * implementation of a parallel task execution engine that uses a
 * thread pool. See definition of ITaskSystem in itasksys.h for
 * documentation of the ITaskSystem interface.
 */

typedef struct {
    IRunnable* _runnable = nullptr;
    std::mutex _mutex;
    std::mutex _finished_mutex;
    std::condition_variable _finished_cv;
    std::mutex _sleep_mutex;
    std::condition_variable _sleep_cv;
    int _done_task_cnt = 0;
    int _num_total_tasks;
    int _left_tasks_cnt;  // > confirm task is done
    bool _kill_flag = false;
    bool _run_flag = false;

}RunStatus;
class TaskSystemParallelThreadPoolSpinning : public ITaskSystem
{
public:
    TaskSystemParallelThreadPoolSpinning(int num_threads);
    ~TaskSystemParallelThreadPoolSpinning();
    const char* name();
    void run(IRunnable* runnable, int num_total_tasks);
    TaskID runAsyncWithDeps(
        IRunnable* runnable, int num_total_tasks, const std::vector<TaskID>& deps);
    void ImplRun(RunStatus &status);
    void sync();

private:
    std::vector<std::thread> _thread_pool;
    RunStatus _run_status;
};

/*
 * TaskSystemParallelThreadPoolSleeping: This class is the student's
 * optimized implementation of a parallel task execution engine that uses
 * a thread pool. See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSleeping : public ITaskSystem
{
public:
    TaskSystemParallelThreadPoolSleeping(int num_threads);
    ~TaskSystemParallelThreadPoolSleeping();
    const char* name();
    void run(IRunnable* runnable, int num_total_tasks);
    TaskID runAsyncWithDeps(
        IRunnable* runnable, int num_total_tasks, const std::vector<TaskID>& deps);
    void sync();
    void ImplRun(RunStatus &status);
private:
    std::vector<std::thread> _thread_pool;
    RunStatus _run_status;
};

#endif
