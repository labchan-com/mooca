#include <iostream>
#include <string>
#include "mooca_http.h"
#include "mooca_status.h"

namespace mooca {

//    class HttpClient public BaseClient{
//        private:
 //       public:
//            HttpClient();
//            HttpClient(std::string url,int thread_num =1);
//            ~HttpClient();
//    }
//

HttpClient::HttpClient(std::string url){
    this->url = url;
    this->thread_num = 0;
}

bool HttpClient::SetThreadNum(int thread_num){
    this->thread_num = thread_num;
    return true;
}

bool HttpClient::NotifyMe(Status status){
    switch(status){
        case Status::FILESIZE:
            if(this->file->size > -1){
                std::shared_ptr<File> tmpfile(new File);
                tmpfile->size = this->file->size;
                call_back(Status::FILESIZE,file); // 建议修改成异步的操作放入线程池，让队列去处理回调是比较合理的，防止阻塞主线程
            }
            else{
                // 启动一个新线程，获取并调用回调函数，告知文件大小
                //
            }
            break;
        case Status::FILENAME:
            //获取文件名并通知回调  一般情况下不会出现该函数的调用.
            break;
        case Status::PROGRESS:
            // 统计一个进度并返回 //这是主动要求返回进度,另外应当有一个适当时机的进度通知,主动调用callback通知进度
            break;
        case Status::ALL:
            //返回一个File对象的完整副本
             break;
        default:
             std::cout << "crash" << std::endl;
            break;
    }
}
}
