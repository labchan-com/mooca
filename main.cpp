//
//  main.cpp
//  mooca
//
//  Created by jelly on 2018/2/18.
//  Copyright © 2018年 mooca. All rights reserved.
//
#include "mooca_http.hpp"
#include "mooca_status.hpp"
#include <iostream>
using namespace mooca;
using namespace std;
int main()
{
    // insert code here...
    ofstream* fp = new ofstream();
    fp->open("ubuntu-16.04.3-desktop-amd64.iso");
//    string url = ("http://mirror.nyist.edu.cn/");
    string url = "https://mirror.nyist.edu.cn/ubuntu-releases/16.04/ubuntu-16.04.3-desktop-amd64.iso";
    HttpClient cli(url, fp, size_t(10));
    cli.NotifyStart();
    getchar();
    fp->close();
    return 0;
}
