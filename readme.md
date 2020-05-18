# CPP_HTTPS_Server
## 系統環境
- OS: Ubuntu 20.04
    - Linux 5.4.0-26-generic #30-Ubuntu SMP Mon Apr 20 16:58:30 UTC 2020 x86_64 
- Compiler: gcc version 9.3.0 (Ubuntu 9.3.0-10ubuntu2) 
- OpenSSL: OpenSSL 1.1.1f  31 Mar 2020
- libssl-dev: 1.1.1f-1ubuntu2
- Text Editor: Vscode


## 功能說明
基於 TLS 的 https remote shell server

1. Server與shell程式碼皆放置於bin資料夾內
2. 使用./Server 即可執行開啟伺服器
3. 使用ctrl+c 即可關閉伺服器

![](https://i.imgur.com/TyRCZ1u.png)

## 重要程式碼說明
```cpp
initial_ssl(); //初始化ssl
initial_socket(); //初始化socket

ssl = SSL_new(ctx); // 建立新的SSL結構

SSL_set_fd(ssl, fd); // 把插頭 串到ssl上面
err = SSL_accept(ssl); // SSL握手

SSL_read(ssl, recv_message, sizeof(recv_message));
// SSL讀取資料

SSL_write(ssl, message.c_str(), message.length());
// ssl寫入資料

SSL_free(ssl); // 釋放ssl
close(fd); // 關閉fd
     
```

## 系統架構
![](https://i.imgur.com/z5YUdKs.png)

## 成果截圖
![](https://i.imgur.com/aVwX4Zx.png)

![](https://i.imgur.com/NHz262w.png)

![](https://i.imgur.com/BNTDY21.png)

![](https://i.imgur.com/DXFs4dz.png)

## 困難與⼼得
這一次的Project相較於前兩次，我覺得寫起來輕鬆了許多。因為只要將前兩次的Project串接在一起就完成了完整的功能。整體來說都沒有遇到重大的難題，只有在讀取get的parameter時，發現瀏覽器會自動幫一些字元進行URL Encode，因此cgi程式在解析的時候也需要先進行解碼的動作。
這三次作業中，讓我一步一步的從http、ssl的基礎架構開始學習，最後自己建立了一個https的server，感覺還滿有成就感的！