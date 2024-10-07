#include <iostream>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>
#include <functional>
#include <future>

class ThreadPool
{

    using func_t = std::function<void()>;

public:
    static ThreadPool *getInstance();

    template <typename F, typename... Args>
    auto enques(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>;

    ~ThreadPool();

private:
    ThreadPool(int thread_num);

    void worker();

private:
    bool isstop_; // 用于标识该线程池是属于运行状态还是停止状态
    std::mutex mtx_;
    std::vector<std::thread> thread_pool; // 线程池
    std::condition_variable cv_;          // 条件变量
    std::queue<func_t> tasks_;            // 任务对列

    // 单例
    static ThreadPool *pool_;
};

ThreadPool *ThreadPool::pool_ = nullptr;

ThreadPool *ThreadPool::getInstance()
{
    if (pool_ == nullptr)
    {
        std::mutex mutex_;
        std::unique_lock<std::mutex> lock(mutex_);
        if (pool_ == nullptr)
        {
            pool_ = new ThreadPool(10);
        }
    }
    return pool_;
}

ThreadPool::ThreadPool(int thread_num) : isstop_(false)
{
    for (int i = 0; i < thread_num; ++i)
    {
        thread_pool.emplace_back([this]()
                                 { this->worker(); });
    }
}

template <typename F, typename... Args>
auto ThreadPool::enques(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>
{
    using functype = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<functype()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<functype> rsfuture = task->get_future();

    {
        // 将任务添加到任务对列中
        std::unique_lock<std::mutex> lock(mtx_);

        if (isstop_)
            throw std::runtime_error("出错了， 线程池已停止");

        tasks_.emplace([task]()
                       { (*task)(); });
    }

    // 唤醒一个线程
    cv_.notify_one();

    return rsfuture;
}

void ThreadPool::worker()
{
    while (true)
    {
        func_t task;

        {
            std::unique_lock<std::mutex> lock(mtx_);

            cv_.wait(lock, [this]()
                     { return isstop_ || !tasks_.empty(); });

            if (isstop_ && tasks_.empty())
                return; // 如果线程池为停止状态并且任务对列为空则退出

            task = std::move(tasks_.front());
            tasks_.pop();
        }

        task();
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex>(mutex_);
        isstop_ = true;
    }

    cv_.notify_all(); // 唤醒所有线程

    // 确保每个线程都完整退出
    for (auto &thread : thread_pool)
    {
        thread.join();
    }

    delete pool_;
}