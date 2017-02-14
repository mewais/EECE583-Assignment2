#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <pthread.h>
#include <list>

namespace TOOLS
{
    template <typename T>
    class WorkQueue
    {
        private:
            std::list<T> Queue;
            pthread_mutex_t Mutex;
            pthread_cond_t Cond;
            pthread_barrier_t Barrier;
            uint32_t MaxSize;
        public:
            WorkQueue(uint32_t NumThreads)
            {
                pthread_barrier_init(&Barrier, NULL, NumThreads);
                pthread_mutex_init(&Mutex, NULL);
                pthread_cond_init(&Cond, NULL);
            }

            ~WorkQueue()
            {
                pthread_mutex_destroy(&Mutex);
                pthread_cond_destroy(&Cond);
                pthread_barrier_destroy(&Barrier);
            }

            void push(T Item)
            {
                pthread_mutex_lock(&Mutex);
                Queue.push_back(Item);
                pthread_cond_signal(&Cond);
                pthread_mutex_unlock(&Mutex);
            }

            void pop()
            {
                pthread_mutex_lock(&Mutex);
                Queue.pop_front();
                pthread_mutex_unlock(&Mutex);
            }

            T peak(uint32_t ID)
            {
                pthread_mutex_lock(&Mutex);
                while (Queue.size() == 0)
                {
                    pthread_cond_wait(&Cond, &Mutex);
                }
                T Item = Queue.front();
                pthread_mutex_unlock(&Mutex);
                pthread_barrier_wait(&Barrier);
                if(ID == 0)
                {
                    pop();
                }
                return Item;
            }

            int size()
            {
                pthread_mutex_lock(&Mutex);
                int Size = Queue.size();
                pthread_mutex_unlock(&Mutex);
                return Size;
            }
    };
}

#endif // TOOLS_HPP
