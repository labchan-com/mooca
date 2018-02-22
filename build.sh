#!/bin/bash
g++ main.cpp mooca_http.hpp mooca_status.hpp mooca_threadpool.hpp mooca_base.hpp mooca_http.cpp -std=c++11 -lcurl

rm -fr *.gch

