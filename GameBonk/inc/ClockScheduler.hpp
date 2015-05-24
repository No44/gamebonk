#ifndef _GBONK_CLOCKSCHEDULER_HPP_
#define _GBONK_CLOCKSCHEDULER_HPP_

#include <cstdint>
#include <functional>
#include <queue>
#include <cassert>

namespace GBonk
{

    class ClockScheduler
    {
    public:

        typedef std::function<void()> Callable;

        class Task
        {
        public:
            Task(Callable f, unsigned int delayInCycles);
            Task(const Task&);

            bool operator<(const Task& t) const { return execDate_ < t.execDate_; }

        private:
            Callable callback_;
            uint64_t execDate_;

            friend class ClockScheduler;
        };

        ClockScheduler();

        bool empty() const { return tasks_.empty(); }

        void reset();
        void step();
        void step(unsigned int cycles);
        void schedule(const Task& t);

        uint64_t cyclesBeforeNextTask() const { assert(!tasks_.empty()); return tasks_.top().execDate_ - date_; }

    private:
        std::priority_queue<Task> tasks_;
        uint64_t date_;
    };

    typedef ClockScheduler::Task ClockTask;
    typedef ClockScheduler::Callable ClockCall;
}

#endif
