// filename: cond_var_queue.cpp
// g++ -std=c++17 -O2 -pthread -o cond_var_queue cond_var_queue.cpp
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <iostream>

std::queue<int>          work_queue;
std::mutex               q_mtx;
std::condition_variable  q_cv;
bool                     done = false;

void producer()
{
    for (int i = 0; i < 20; ++i) {
        {
            std::lock_guard lk(q_mtx);
            work_queue.push(i);
        }
        q_cv.notify_one(); // 소비자 깨우기
    }
    {
        std::lock_guard lk(q_mtx);
        done = true;
    }
    q_cv.notify_all();
}

void consumer()
{
    while (true) {
        std::unique_lock lk(q_mtx);
        q_cv.wait(lk, []{ return !work_queue.empty() || done; }); // spurious wakeup 처리
        while (!work_queue.empty()) {
            int item = work_queue.front();
            work_queue.pop();
            lk.unlock();
            std::cout << "처리: " << item << "\n";
            lk.lock();
        }
        if (done) break;
    }
}

int main()
{
    std::thread prod(producer);
    std::thread cons(consumer);
    prod.join();
    cons.join();
    return 0;
}
