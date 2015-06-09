#include <cassert>
#include "ClockScheduler.hpp"

namespace GBonk
{

    ClockScheduler::Task::Task(Callable f, unsigned int d)
        : callback_(f), execDate_(d)
    {
    }

    ClockScheduler::Task::Task(const Task& t)
        : callback_(t.callback_), execDate_(t.execDate_)
    {
    }

    ClockScheduler::ClockScheduler()
        : date_(0)
    {
    }
    
    void ClockScheduler::reset()
    {
        date_ = 0;
        auto empty = decltype(tasks_)();
        tasks_.swap(empty);
    }

    void ClockScheduler::step()
    {
        step(1);
    }

    void ClockScheduler::step(unsigned int c)
    {
        date_ += c;
        if (tasks_.empty())
            return;

        while (date_ >= tasks_.top().execDate_)
        {
            tasks_.top().callback_();
            tasks_.pop();
        }
    }

    void ClockScheduler::schedule(const Task& t)
    {
        uint64_t execDate = t.execDate_ + date_;
        tasks_.push(Task(t.callback_, execDate));
    }

}
