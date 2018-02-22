#include <iostream>
#include <fstream>


using namespace std;
int main(){
    //ofstream f("123.txt",ios::out);
    ofstream fddd;
    fddd.open("123.txt",ios::out | ios::app);
    for(int i =0; i< 3000;i++){
        fddd << i <<"-";
    }
    fddd.close();
}
