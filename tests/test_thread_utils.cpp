#include <string>
#include <thread>
#include <iostream>

#if defined(__linux__)
#include <pthread.h>
#endif

#include <utils/thread_utils.h>
#include <unistd.h>


int main(){
    std::cout<< "1:main thread id:"<<pans::GetThreadId() <<std::endl;
    std::cout<< "2:main thread name:"<<pans::GetThreadName() <<std::endl;
    sleep(30);
    pans::SetThreadName("main_theadname");
    std::cout <<"3:main thread name:" <<pans::GetThreadName() <<std::endl;
    sleep(30);
    std::thread worker([](){
        std::cout<<"4:son thread id:"<<pans::GetThreadId()<<std::endl;
        std::cout<<"7:son thread name:"<<pans::GetThreadName()<<std::endl;
        sleep(30);
        pans::SetThreadName("son-thread-long-name");
        std::cout<<"8: son thread name:"<<pans::GetThreadName()<<std::endl;

        sleep(30);
        pans::SetThreadName("");
        std::cout<<"9: son thread name:"<<pans::GetThreadName()<<std::endl;
    });
    worker.join();
    return 0;
}