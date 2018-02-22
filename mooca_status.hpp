#ifndef __MOOCA_STATUS_HPP_
#define __MOOCA_STATUS_HPP_

//
//  mooca_status.h
//  mooca
//
//  Created by jelly on 18/02/2018.
//  Copyright © 2018 mooca.io. All rights reserved.
//


enum class Status{
   UNDEFINE = -1,        //未定义的状态
   ZERO,            //初始状态
   FILENAME,        //文件名
   FILESIZE,        //文件的大小
   PROGRESS,        //进度
   REMAINING_TIME,  //剩余时间
   DOWNLOAD_SIZE,   //已经下载的文件大小
   ALL,             //所有的数据
   DOWNLOAD_FAILED, 
};



#endif
