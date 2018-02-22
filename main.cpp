//
//  main.cpp
//  mooca
//
//  Created by jelly on 2018/2/18.
//  Copyright © 2018年 mooca. All rights reserved.
//

#include <iostream>
#include "mooca_status.hpp"
#include "mooca_http.hpp"
#include "mooca_status.hpp"
using namespace mooca;
using namespace std;
int main(int argc, const char * argv[]) {
    // insert code here...
    ofstream *fp = new ofstream();
    fp->open("a.ios");
//    string url = ("http://mirror.nyist.edu.cn/ubuntu-releases/ubuntu-core/16/ubuntu-core-16-amd64.img.xz");
    string url = "http://www.baidu.com";
    HttpClient cli(url,fp,size_t(10));
    cli.NotifyStart();

    while(true);
    return 0;
}
