#include <iostream>
#include "mooca_http_helper.hpp"

int main(){
   
    std::cout << "file size is" << get_http_file_size("http://www.baidu.com") << std::endl; 
}
