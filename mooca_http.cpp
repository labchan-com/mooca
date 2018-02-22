#include "mooca_http.hpp"
#include "mooca_status.hpp"
#include "mooca_threadpool.hpp"
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <string>

namespace mooca
{

size_t writeFunc(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    BlockNode* node = (BlockNode*)userdata;
    std::lock_guard<std::mutex> lock(node->cl->mutex_io);
    if (node->start + size * nmemb <= node->end)
    {
        node->fp->seekp(node->start, std::ios::beg);
        node->fp->write((char*)ptr, (size * nmemb));
        node->start += size * nmemb;
    }
    else
    {
        node->fp->seekp(node->start, std::ios::beg);
        node->fp->write((char*)ptr, node->end - node->start + 1);
        node->start = node->end;
    }
    return size * nmemb;
}

HttpClient::HttpClient(std::string url, std::ofstream* fp, size_t thread_num)
{
    this->url = url;
    this->thread_num = thread_num;
    this->file_ptr = new File;
    file_ptr->size = 0;
    file_ptr->remaining_time = 0;
    file_ptr->progress = 0;
    worker_ptr = new ThreadPool(thread_num + 2);
    this->fp = fp;
}

void HttpClient::sync_http_file_size()
{
    double size = 0;
    CURL* handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle, CURLOPT_HEADER, 1L); //只需要header头
    curl_easy_setopt(handle, CURLOPT_NOBODY, 1L); //不需要body
    curl_easy_setopt(handle, CURLOPT_FAILONERROR, 1L);
    if (curl_easy_perform(handle) == CURLE_OK)
    {
        curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);
        std::unique_lock<std::mutex> lock(this->mutex_file);
        this->file_ptr->size = size;
    }
}

bool HttpClient::NotifyStart()
{
    //std::cout << this->is_start << std::endl;
    if (!(this->is_start))
    {
        std::function<void()>func = [this]
        {
            //std::cout << this->file_ptr->size << std::endl;
            if (0 == this->file_ptr->size)
            {
                this->sync_http_file_size();
                //分大小 启动各种下载线程
                size_t block_size = this->file_ptr->size / this->thread_num;
                for (int i = 0; i < this->thread_num; i++)
                {
                    //     std::cout << "block" <<std::endl;
                    BlockNode* tmp = new BlockNode;
                    tmp->start = i * block_size;
                    tmp->end = (i + 1) * block_size;
                    if (i == this->thread_num - 1)
                    {
                        if ((this->file_ptr->size) % (this->thread_num) != 0)
                        {
                            tmp->end = this->file_ptr->size;
                        }
                    }
                    char range[64] = { 0 };
                    snprintf(range, sizeof(range), "%ld-%ld", tmp->start, tmp->end);
                    std::cout << range << std::endl;
                    CURL* curl = curl_easy_init();
                    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)tmp);
                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
                    curl_easy_setopt(curl, CURLOPT_RANGE, range);
                    tmp->curl = curl;
                    tmp->fp = this->fp;
                    tmp->cl = this;
                    this->blocks.push_back(tmp);
                    this->worker_ptr->enqueue([this, tmp, i]
                    {
                        CURLcode code = CURLcode(0);
                        code = curl_easy_perform(tmp->curl);
                        if (code != CURLE_OK)
                        {
                            //notify download failed
                            File file;
                            {
                                std::lock_guard<std::mutex> lock(this->mutex_file);
                                file = *(this->file_ptr);
                            }
                            this->is_start = false;
                            this->call_back(Status::DOWNLOAD_FAILED, file);
                        }
                        else
                        {
                            std::cout << "Thread download success " << i << "Exit " << std::endl;
                        }
                    });
                }
            }
        };
        this->worker_ptr->enqueue(func);
        this->is_start = true;
    }
    else
    {

    }
    return true;
}


template <int CALLBACK_TYPE>
void HttpClient::SetNotifyCallback(std::function<void()> && call_back)
{

}


void HttpClient::NotifySume()
{
    for (auto &i : this->blocks)
    {
       curl_easy_pause(i->curl ,CURLPAUSE_RECV);
    }
}

void HttpClient::NotifyResume(){
    for(auto i:this->blocks){
        curl_easy_pause(i->curl, CURLPAUSE_CONT);
    }
}

void HttpClient::NotifyStop(){
    this->is_start = false;
}

bool HttpClient::NotifyMe(Status status)
{
    switch (status)
    {
    case Status::FILESIZE:
        if (this->file_ptr->size != 0)
        {
            File tmpfile;
            {
                std::unique_lock<std::mutex> lock(this->mutex_file);
                tmpfile = *(this->file_ptr);
            }
            call_back(Status::FILESIZE, tmpfile);
        }
        else
        {
            // 启动一个新线程，获取并调用回调函数，告知文件大小
            worker_ptr->enqueue(
                [this]()
            {
                this->sync_http_file_size();
                this->call_back(Status::FILESIZE, *(this->file_ptr));
            });
        }
        break;
    case Status::PROGRESS_DOWNLOAD:
        // 统计一个进度并返回 //这是主动要求返回进度,另外应当有一个适当时机的进度通知,主动调用callback通知进度
        if (this->file_ptr->progress != 0)
        {
            File tmpfile;
            {
                std::unique_lock<std::mutex> lock(this->mutex_file);
                tmpfile = *(this->file_ptr);
            }
            call_back(Status::PROGRESS_DOWNLOAD, tmpfile);
        }
        break;
    case Status::ALL:
        File file;
        {
            std::unique_lock<std::mutex> lock(this->mutex_file);
            file = *(this->file_ptr);
        }
        call_back(Status::ALL, file);
        break;
    default:
        std::cout << "crash" << std::endl;
        break;
    }
    return true;
}
HttpClient::~HttpClient()
{
    if (this->worker_ptr != NULL)
    {
        delete worker_ptr;
    }
    if (this->file_ptr != NULL)
    {
        delete file_ptr;
    }
    for (auto& i : this->blocks)
    {
        if (NULL != i)
        {
            curl_easy_cleanup(i->curl);
            delete i;
        }
    }
}
}
