#include <arpa/inet.h>  
#include <netinet/in.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>  
#include <sys/types.h>
#include <sys/wait.h>  
#include <unistd.h>    

#include <cstring>
#include <iostream>

using namespace std;

#define SERVER_CERT "server.crt"
#define SERVER_KEY "server.key"
#define CA_CERT "rootca.crt"

#define Server_port 8087

string create_process(string, string, string);
void response200(int, string);
int find_n(string, char, int);
void initial_ssl();
void initial_socket();

int err;
int sock;
struct sockaddr_in Server_addr;
struct sockaddr_in Client_addr;
socklen_t Client_addr_len;

SSL_CTX *ctx;
SSL *ssl;
X509 *client_cert = NULL;

int main() {
    initial_ssl();
    initial_socket();

    while (1) {
        int fd =
            accept(sock, (struct sockaddr *)&Client_addr, &Client_addr_len);
        /*
            socket
            Client_addr
            length
        */

        ssl = SSL_new(ctx);
        // 建立新的SSL結構

        if (ssl == NULL) {
            cout << "SSL new Error!" << endl;
            exit(1);
        }

        SSL_set_fd(ssl, fd);
        // 把插頭 串到ssl上面
        err = SSL_accept(ssl);
        // SSL握手(伺服器)

        if (err == -1) {
            ERR_print_errors_fp(stderr);
            exit(1);
        }

        char recv_message[1000] = {};
        bzero(&recv_message, sizeof(recv_message));

        err = SSL_read(ssl, recv_message, sizeof(recv_message));
        // SSL讀取資料
        if (err == -1) {
            ERR_print_errors_fp(stderr);
            exit(1);
        }

        string recv_string = string(recv_message);
        string first_row = recv_string.substr(0, recv_string.find('\n'));

        string message;
        string method = first_row.substr(0, recv_string.find(' '));
        string path = first_row.substr(find_n(first_row, ' ', 0) + 1);
        path = path.substr(0, path.find("HTTP/1.1") - 1);
        // 切字串

        if (method == string("GET")) {
            cout << "GET " << path << endl;
            if (path.substr(0, 10) == string("/shell.cgi")) {
                message = create_process(path, path, "Shell.cgi");
            } else {
                message = "meow_meow ?__? get";
            }

        } else {
            message = "meow_meow not get ?__?";
        }
        response200(fd, message);
        SSL_free(ssl);
        close(fd);
    }
}

void initial_ssl() {
    //-------------------------------------
    /* 初始化SSL */

    SSL_library_init();
    // 載入SSL圖書館

    SSL_load_error_strings();
    // 載入SSL錯誤的String

    ctx = SSL_CTX_new(SSLv23_method());
    // 創建CTX 結構
    // v23: sslv3可向下支援v2

    if (!ctx) {
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    SSL_CTX_set_ecdh_auto(ctx, 1);

    //-------------------------------------
    /* 載入本地端證書 */

    err = SSL_CTX_use_certificate_file(ctx, SERVER_CERT, SSL_FILETYPE_PEM);

    if (err == -1) {
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    err = SSL_CTX_use_PrivateKey_file(ctx, SERVER_KEY, SSL_FILETYPE_PEM);

    if (err == -1) {
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    err = SSL_CTX_check_private_key(ctx);
    // 確認載入的證書跟私鑰吻合

    if (err == -1) {
        ERR_print_errors_fp(stderr);
        exit(1);
    }
}

void initial_socket() {
    sock = socket(AF_INET, SOCK_STREAM, 0);  // Create socket
    /*
        AF_INET = ipv4 tcp or udp
        SOCK_STREAM = tcp
        type = 預設都０
    */

    if (sock == -1) {
        perror("socket");
        exit(1);
    }

    bzero(&Server_addr, sizeof(Server_addr));   // 先清0這ㄍ struct
    Server_addr.sin_family = AF_INET;           // ipv4
    Server_addr.sin_port = htons(Server_port);  // hton: Host To Network Short
                                                // integer (小印地安轉大印地安)
    Server_addr.sin_addr.s_addr = 0;  // Any ip

    err = bind(sock, (struct sockaddr *)&Server_addr, sizeof(Server_addr));
    /*
        sock = sock status code
        (struct sockaddr *) &Server_addr = 把 Server_addr 轉成 sockaddr ㄉ指標
        socklen = sizeof(Server_addr)
    */
    // 把我家ㄉ地址綁上插頭

    if (err == -1) {
        perror("bind");
        exit(1);
    }

    err = listen(sock, 100);
    /*
        sock
        幾個人能連入server
    */

    if (err == -1) {
        perror("listen");
        exit(1);
    }

    bzero(&Client_addr, sizeof(Client_addr));  // 先清0這ㄍ struct
    Client_addr_len = sizeof(Client_addr);     // 定義Client address的Length

    int alive = 1;
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (const char *)&alive,
               sizeof alive);
    /*
        sock：將要被設定或者獲取選項的套接字。
        level：選項所在的協議層。
        optname：需要訪問的選項名。
        optval：對於getsockopt()，指向返回選項值的緩衝。對於setsockopt()，指向包含新選項值的緩衝。
        optlen：對於getsockopt()，作為入口引數時，選項值的最大長度。作為出口引數時，選項值的實際長度。對於setsockopt()，現選項的長度。
    */

    cout << "Server start at https://b10507004:" << Server_port << endl;
    cout << "Get CGI https://b10507004:" << Server_port << "/shell.cgi" << endl;
    cout << endl << "Press Ctrl+C to terminate the server" << endl;
}


// 回應HTTP Request 200 的Header 加上 message
void response200(int fd, string message) {
    string response = "HTTP/1.1 200 OK\nContent-Length: ";
    response.append(to_string(message.length()));
    response.append("\nContent-Type: text/html\nConnection: close\n\n");
    response.append(message);
    SSL_write(ssl, message.c_str(), message.length());
}

//尋找string中，第n個f的位置
int find_n(string i, char f, int n) {
    int counter = -1;
    for (int j = 0; j < i.length(); j++) {
        if (i[j] == f) {
            counter++;
        }
        if (counter == n) {
            return j;
        }
    }
    return -1;
}

string create_process(string path, string data, string cgi_name) {
    string return_data;
    int cgiInput[2];
    int cgiOutput[2];
    int status;
    char inputData[1000];
    sprintf(inputData, "%s", data.c_str());
    pid_t cpid;
    char c;
    /* Use pipe to create a data channel betweeen two process
       'cgiInput'  handle  data from 'host' to 'CGI'
       'cgiOutput' handle data from 'CGI' to 'host'*/
    if (pipe(cgiInput) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    if (pipe(cgiOutput) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    /* Creates a new process to execute cgi program */
    cpid = fork();
    if (cpid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    /*child process*/
    if (cpid == 0) {
        // close unused fd
        close(cgiOutput[0]);
        close(cgiInput[1]);

        //把stdout的寫入 導到cgiOutput
        dup2(cgiOutput[1], STDOUT_FILENO);

        //把stdin的讀取 導到cgiInput
        dup2(cgiInput[0], STDIN_FILENO);

        // after redirect we don't need the old fd
        close(cgiOutput[1]);
        close(cgiInput[0]);

        /* execute cgi program
           the stdout of CGI program is redirect to cgiOutput
           the stdin  of CGI program is redirect to cgiInput
        */

        // 執行CGI
        execlp((string("./") + cgi_name).c_str(),
               (string("./") + cgi_name).c_str(), NULL);
        exit(0);
    }

    else {
        //關閉舊的fd
        close(cgiOutput[1]);
        close(cgiInput[0]);

        // 把資料Write到cgi
        write(cgiInput[1], inputData, strlen(inputData));

        // 把cgi吐回來資料塞回string
        while (read(cgiOutput[0], &c, 1) > 0) {
            return_data += c;
        }

        close(cgiOutput[0]);
        close(cgiInput[1]);
        waitpid(cpid, &status, 0);
    }
    return return_data;
}
