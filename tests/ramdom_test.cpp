#include <atomic>
#include <thread>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <latch>
#include <mutex>
#include <shared_mutex>
#include <numeric>
#include <stdexcept>
#include <iostream>
#include <vector>
constexpr int RUNS = 7;
std::atomic<int> flag;
uint64_t g_target = 0;
uint64_t g_counter = 0;
std::atomic<uint64_t> g_atomicounter{0};
std::mutex g_mutex;
std::shared_mutex g_sharedmutex;

uint64_t OperationForThread(std::size_t thread_idx,std::size_t thread_count){
    const uint64_t base = g_target/thread_count;
    const uint64_t remain = g_target%thread_count;
    if(remain>thread_idx){
    return base + 1 ;
    }else{
        return base;
    }
}
template <typename Operation> 
std::chrono::nanoseconds BThreads(std::size_t threadcount,Operation&& operation){
    std::latch ready(static_cast<std::ptrdiff_t>(threadcount));
    std::latch start_gate(1);
    std::latch finish(static_cast<std::ptrdiff_t>(threadcount));
    std::vector<std::thread> threads;
    threads.reserve(threadcount);
    for(std::size_t thread_idx = 0; thread_idx<threadcount ; thread_idx++){
        threads.emplace_back(
            [&,thread_idx](){
                ready.count_down();
                start_gate.wait();
                operation(thread_idx,OperationForThread(thread_idx,threadcount));
                finish.count_down();
            }
        );
    }
    ready.wait();
    const auto begin = std::chrono::steady_clock::now();
    start_gate.count_down();
    finish.wait();
    const auto end = std::chrono::steady_clock::now();
    for(std::thread& thread: threads){
        thread.join();
    }
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin);
}

std::chrono::nanoseconds bMutex(std::size_t threadcount){
    g_counter =0;
    const auto time = BThreads(
        threadcount,
        [](std::size_t,uint64_t operations){
        for(uint64_t index =0;index<operations;index++){
            std::lock_guard<std::mutex> lock(g_mutex);
            g_counter++;
        }
        }
    );
    return time;
}
std::chrono::nanoseconds bShared_Mutex(std::size_t threadcount){
    g_counter =0;
    const auto time = BThreads(
        threadcount,
        [](std::size_t,uint64_t operations){
        for(uint64_t index =0;index<operations;index++){
            std::unique_lock<std::shared_mutex> lock(g_sharedmutex);
            g_counter++;
        }
        }
    );
    return time;
}
std::chrono::nanoseconds bAtomic(std::size_t threadcount){
   g_atomicounter.store(0,std::memory_order_relaxed);
   const auto time = BThreads(
        threadcount,
        [](std::size_t,uint64_t operations){
            for(uint64_t index =0; index < operations;index++){
                g_atomicounter.fetch_add(1,std::memory_order_relaxed);
            }
        }
   );
   return time;
}
std::chrono::nanoseconds bAtomicCAS(std::size_t threadcount){
   g_atomicounter.store(0,std::memory_order_relaxed);
   const auto time = BThreads(
        threadcount,
        [](std::size_t,uint64_t operations){
            for(uint64_t index =0; index < operations;index++){
                uint64_t expected = g_atomicounter.load(std::memory_order_relaxed);
                while(!g_atomicounter.compare_exchange_weak(expected,expected+1,std::memory_order_relaxed,std::memory_order_relaxed)){

                }
            }
        }
   );
   return time;
}
template <typename B>
double Run(B&& func){
     long double total_time = 0.0L;
     for(int i=0;i<RUNS;i++){
        total_time += static_cast<long double>(func().count());
     }
     return static_cast<double>(total_time/RUNS);
}
void printresult(std::string_view name,double total_nanoseconds){
     const double nanoseconds_per_operation = total_nanoseconds / static_cast<double> (g_target);
     std::cout<<name<<"  "<<total_nanoseconds<<"  "<<nanoseconds_per_operation<<std::endl;
}
int main(int argc,char** argv){
    if(argc!=2){
        std::cerr<<"Usage conunt\n";
        return -1;
    }
    g_target = std::atoi(argv[1]);
    std::cout<<std::fixed<<std::setprecision(2);
    std::cout<<"operations:"<<g_target<<", average of"<<RUNS<<"runs\n\n";
    std::vector<std::size_t> threads_count = {1,2,4,8,10};
    for(auto threadcount : threads_count){
        std::cout<<"thread: "<<threadcount<<std::endl;
        printresult("mutex",Run([&](){return bMutex(threadcount);}));
        printresult("shared_mutex",Run([&](){return bShared_Mutex(threadcount);}));
        printresult("atomic",Run([&](){return bAtomic(threadcount);}));
        printresult("atomic CAS",Run([&](){return bAtomicCAS(threadcount);}));
    }
    return 0;
}