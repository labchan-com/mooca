//
//  mooca_base.h
//  mooca
//
//  Created by jelly on 18/02/2018.
//  Copyright © 2018 mooca.io. All rights reserved.
//

#include <string>
#include<functional>
#include "mooca_status.h"

namespace mooca{

typedef struct File{
    std::string url;
    
    long size;
    double remaining_time;
    double progress;

} File;

class BaseClient{
    protected:
        std::function<void(Status status, std::shared_ptr<File> file)> call_back;
    public: 
        virtual bool NotifyMe(Status status) = 0;
        virtual bool NotifyStart() = 0;
        virtual bool NotifyStop() = 0;
        virtual bool NotifySume() = 0;
        virtual bool NotifyResume() = 0;

        virtual bool SetNotifyCallback(std::function<void(Status status,  std::shared_ptr<File> file)> call_back){
                this->call_back = call_back;
                return true;
        }
};

}
