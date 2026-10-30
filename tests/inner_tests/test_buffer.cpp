#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include "logger/buffer.h"
#include "logger/buffer_config.h"
constexpr int RUNS = 7;
std::uint64_t g_target=0;
std::atomic <std::uint64_t> B_SINK{0};

std::uint64_t Observe(std::string_view value){
    return static_cast<std::uint64_t>(value.size()) +
           static_cast<unsigned char> (value.front())+
           static_cast<unsigned char> (value[value.size()/2]) +
           static_cast<unsigned char> (value.back())
    ;          
} 

std::uint64_t WriteWithSmallStringBuffer(std::string_view message){
    pans::detail::Inline_buffer<pans::detail::LOG_MESSAGE_INLINE_SIZE> buffer;
    pans::detail::SmallStringBuffer<pans::detail::LOG_MESSAGE_INLINE_SIZE> smallbuffer(buffer);
    std::ostream stream(&smallbuffer);
    stream << message;
    return Observe(buffer.view());
}

std::uint64_t WriteWithStringStream(std::string_view message){
    std::stringstream stream;
    stream << message;
    return Observe(stream.str());
}

template<typename Operation>
std::chrono::nanoseconds BWrite(Operation&& operation){
    std::uint64_t check_sum =0;
    const auto begin = std::chrono::steady_clock::now();
    for(uint64_t idx =0;idx<g_target;idx++){
        check_sum+=operation();
    }
    const auto end = std::chrono::steady_clock::now();
    B_SINK.fetch_xor(check_sum,std::memory_order_relaxed);//防止编译器优化
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin);
}

template<typename B>
double Run(B&& b){
    long double total_time = 0.0L;
    for(int i=0;i<RUNS;i++){
        total_time += static_cast<long double>(b().count());
    }
    return static_cast<double>(total_time/RUNS);
}

void printresult(std::string_view name,double total_time){
     std::cout<<name <<" "
     <<total_time<<" "
     <<total_time/(static_cast<double>(g_target))<<std::endl;
}

int main(int argc,char** arg){
    if(argc!=2){
        std::cout<<"wrong";
        return -1;
    }
    g_target = std::atoi(arg[1]);
    std::cout<<std::fixed<<std::setprecision(2);
    std::cout<<"operations:"<<g_target<<", average of"<<RUNS<<"runs\n\n";
    const std::array<std::size_t,6> message_size ={
        50,
        100,
        200,
        500,
        1000,
        2048
    };
    for(const std::size_t messagesize : message_size){
        const std::string message(messagesize,'x');
        const double small_times =
        Run([&](){
            return BWrite(
                [&](){
                    return WriteWithSmallStringBuffer(message);
                }
            );
        });
        const double times = 
        Run([&](){
           return BWrite(
            [&](){
                return WriteWithStringStream(message);
            }
           );
        }
        );
        printresult("samll_string_buffer",small_times);
        printresult("string_stream",times);
    }
    return 0;
}