#include <iostream>
#include <string>
#include <curl/curl.h>
#include "mooca_http.hpp"
#include "mooca_status.hpp"
#include "mooca_threadpool.hpp"
#include "mooca_http_helper.hpp"

namespace mooca {

//    class HttpClient public BaseClient{
//        private:
 //       public:
//            HttpClient();
//            HttpClient(std::string url,int thread_num =1);
//            ~HttpClient();
//    }
//

size_t writeFunc (void *ptr, size_t size, size_t nmemb, void *userdata)  
{  
    BlockNode  *node = (BlockNode *) userdata;  
    std::lock_guard<std::mutex> lock(node->cl->mutex_io);
    if (node->start + size * nmemb <= node->end)  
    {  
        node->fp->seekp(node->start,std::ios::beg);
        node->fp->write ((char*)ptr, (size*nmemb));  
        node->start += size * nmemb;  
    }  
    else  
    {  
        node->fp->seekp(node->start,std::ios::beg);
        node->fp->write((char*)ptr,node->end - node->start + 1);
        node->start = node->end;  
    }  
    return size*nmemb;  
}  



HttpClient::HttpClient(std::string url,std::ofstream *fp,size_t thread_num){
    this->url = url;
    this->thread_num = thread_num;
    this->file_ptr = new File;
    worker_ptr = new ThreadPool(thread_num+2);
    this->fp = fp;
}


void HttpClient::sync_http_file_size(){
	double size = 0;
    CURL *handle = curl_easy_init();  
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());  
    curl_easy_setopt(handle, CURLOPT_HEADER, 1L);    //只需要header头  
    curl_easy_setopt(handle, CURLOPT_NOBODY, 1L);    //不需要body  
    if (curl_easy_perform(handle) == CURLE_OK) {  
        curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);  
        std::unique_lock<std::mutex> lock(this->mutex_file);
    	this->file_ptr->size = size;
	} 
}

bool HttpClient::NotifyStart(){
    if(!(this->is_start) ){
        if(0 == this->file_ptr->size){
            this->sync_http_file_size();
            //分大小 启动各种下载线程
            size_t block_size = this->file_ptr->size / this->thread_num;
            for(int i=0; i<=this->thread_num; i++){
                BlockNode* tmp = new BlockNode;
                if(i<(this->thread_num-1)){
                    tmp->start = i*block_size;
                    tmp->end = (i+1)*block_size;
                }
                else{
                    if((this->file_ptr->size) % (this->thread_num) != 0){
                        tmp->start = i*block_size;
                        tmp->end = this->file_ptr->size;
                    }
                }
                CURL* curl = curl_easy_init();
                curl_easy_setopt (curl, CURLOPT_URL, url.c_str ());
                curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, writeFunc);
                tmp->curl = curl;
                tmp->fp = this->fp;
                 
                this->blocks.push_back(tmp);
                this->worker_ptr->enqueue( [this,tmp](){
                   //do something 
                   CURL *curl = tmp->curl;
                   //wait start
                   CURLcode code;
                   code = curl_easy_perform(curl);
                   if(code != CURLE_OK){
                        //notify download failed
                   }
                });
            }
        }
        this->is_start = true;
    }
    return true;
}

bool HttpClient::NotifyMe(Status status){
    switch(status){
        case Status::FILESIZE:
            if(this->file_ptr->size != 0){
                File tmpfile;
                {
                    std::unique_lock<std::mutex> lock(this->mutex_file);
                    tmpfile = *(this->file_ptr);
                }
                call_back(Status::FILESIZE,tmpfile); 
            }
            else{ 
                // 启动一个新线程，获取并调用回调函数，告知文件大小
                worker_ptr->enqueue(
                        [this](){
                            this->sync_http_file_size();
                            this->call_back(Status::FILESIZE,*(this->file_ptr));
                });
            }
            break;
        case Status::PROGRESS:
            // 统计一个进度并返回 //这是主动要求返回进度,另外应当有一个适当时机的进度通知,主动调用callback通知进度
            if(this->file_ptr->progress != 0){
                File tmpfile;
                {
                    std::unique_lock<std::mutex> lock(this->mutex_file);
                    tmpfile = *(this->file_ptr);
                }
                call_back(Status::PROGRESS,tmpfile);  
            }
            break;
        case Status::ALL:
            File file;
             {
                 std::unique_lock<std::mutex> lock(this->mutex_file);
                 file = *(this->file_ptr);
             }
             call_back(Status::ALL,file);
             break;
        default:
             std::cout << "crash" << std::endl;
            break;
    }
}

HttpClient::~HttpClient(){
    if(this->worker_ptr != NULL){
        delete worker_ptr;
    }
    if(this->file_ptr != NULL){
        delete file_ptr;
    }
    for(auto &i : this->blocks){
        if(NULL != i){
            delete i;
        }
    }
}
}


