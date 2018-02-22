#ifndef __MOOCA_STATUS_HPP_
#define __MOOCA_STATUS_HPP_

//
//  mooca_status.h
//  mooca
//
//  Created by jelly on 18/02/2018.
//  Copyright © 2018 mooca.io. All rights reserved.
//
#include <curl/curl.h>


enum class Status
{
    FILESIZE,       //文件的大小
    PROGRESS_DOWNLOAD,       //进度
    REMAINING_TIME, //剩余时间
    DOWNLOAD_SIZE,  //已经下载的文件大小
    ALL,            //所有的数据
    DOWNLOAD_FAILED,
};

#define CALLBACK_INFO_SPEED_DOWNLOAD   0   //当前文件的下载速度
#define CALLBACK_INFO_PROGRESS_DOWNLOAD    1 //当前文件的下载进度 
#define CALLBACK_INFO_DOWNLOAD 2            //获取所有的信息
#define CALLBACK_GET_PASSWORD 3             //密码请求回调

#endif
