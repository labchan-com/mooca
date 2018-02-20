//
//  mooca_http.h
//  mooca
//
//  Created by jelly on 18/02/2018.
//  Copyright © 2018 mooca.io. All rights reserved.
//

#include <iostream>
#include <mutex>
#include <atomic>
#include <curl/curl.h>
#include <fstream>
#include "mooca_base.hpp"
#include "mooca_status.hpp"
#include "mooca_threadpool.hpp"

namespace mooca{
class HttpClient;

static size_t writeFunc (void *ptr, size_t size, size_t nmemb, void *userdata);

    typedef struct BlockNode{
        size_t start;
        size_t end;
        CURL* curl;
        std::ofstream* fp;
        HttpClient *cl;
    } BlockNode;

    class HttpClient :public BaseClient{
        private:
            std::string url;
            int thread_num = 0;
            File *file_ptr = NULL;
            
            std::mutex mutex_file;
            ThreadPool* worker_ptr = NULL;

            std::atomic<bool> is_start{false};

            std::ofstream* fp = NULL;
            std::mutex mutex_io;
            std::vector<BlockNode* > blocks;
        public:
            HttpClient(std::string url,std::ofstream* fp,size_t thread_num);
            void sync_http_file_size();
            virtual bool NotifyMe(Status status);
            virtual bool NotifyStart();
            ~HttpClient();
            friend size_t writeFunc (void *ptr, size_t size, size_t nmemb, void *userdata);
    };



}
