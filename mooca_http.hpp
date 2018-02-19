//
//  mooca_http.h
//  mooca
//
//  Created by jelly on 18/02/2018.
//  Copyright © 2018 mooca.io. All rights reserved.
//

#include <iostream>
#include "mooca/base.h"
#include "mooca/status.h"

namespace mooca{

class HttpClient :public BaseClient{
    private:
        std::string url;
        int thread_num = 0;
        std::shared_ptr<File> file;
    public:
        HttpClient();
        HttpClient(std::string url);
        virtual bool NotifyMe(Status );
        bool SetThreadNum(int thread_num);
        ~HttpClient();
};

}
