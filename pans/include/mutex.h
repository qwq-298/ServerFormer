#ifndef MUTEX_INCULDE
#define MUTEX_INCULDE

#include <atomic>
#include <mutex>

#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#elif defined(__aarch64__) || defined(__arm___)
#include <arm_acle.h>
#endif

#if defined(__linux__) || defined(__GLIBC__)
#include <pthread.h>
#endif

inline void cpu_relax() noexcept{
    #if defined(__x86_64__) || defined(__i386__)
    _mm_pause();
    #elif defined(__aarch64__) || defined(__arm___)
    __yield();
    #endif
}

namespace pans{
    #if defined(__linux__) || defined(__GLIBC__)
    class SpinLock{
        private:
        pthread_spinlock_t m_mutex{};
        public:
        SpinLock() noexcept{
            pthread_spin_init(&m_mutex,PTHREAD_PROCESS_PRIVATE);
        }
        ~SpinLock() noexcept{
            pthread_spin_destroy(&m_mutex);
        }
        SpinLock(const SpinLock&) = delete;
        SpinLock& operator=(const SpinLock&) = delete;
        void lock() noexcept{
            pthread_spin_lock(&m_mutex);
        }
        void unlock() noexcept{
            pthread_spin_unlock(&m_mutex);
        }
    };
    #else
    class SpinLock{
        private:
        std::atomic_flag m_mutex =ATOMIC_FLAG_INIT;
        public:
        SpinLock() = default;
        ~SpinLock() = default;
        SpinLock (const SpinLock&) = delete;
        SpinLock& operator=(const SpinLock&) =delete;
        void lock(){
            while(m_mutex.test_and_set(std::memory_order_acquire)){
                while(m_mutex.test(std::memory_order_relaxed)){
                    cpu_relax();
                }
            }
        }//m_mutex.test_and_set(std::memory_order_acquire) 为true时表示锁已被占有
        //m_mutex.test(std::memory_order_relaxed) 为true时表示锁已被占有 轮询
        bool try_lock(){
              return !m_mutex.test_and_set(std::memory_order_acquire);
        }
        void unlock(){
            m_mutex.clear(std::memory_order_release);
        }
    };
    #endif
}


#endif