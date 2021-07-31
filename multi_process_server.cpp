#include <iostream>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define SERVER_PORT 9999

void sysErr(const char* str){
    perror(str);
    exit(1);
}
        
void reap_child(int sig){
    pid_t wid;
    while((wid = waitpid(0, NULL, -1)) > 0){
        std::cout << "child process " <<  wid << " has benn reaped" << std::endl;
    }
    return;
}



int main(){
    int serverfd, clientfd, ret;
    pid_t pid;
    char buf[BUFSIZ];
    

    struct sockaddr_in server_addr, client_addr;
    
    socklen_t client_addr_len;
    client_addr_len = sizeof(client_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    ret = bind(serverfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(serverfd, 128);

    

    while(1){
        clientfd = accept(serverfd, (struct sockaddr*)&client_addr, &client_addr_len);

        pid = fork();
        if(pid < 0){
            sysErr("fork error");
        } else if(pid == 0){
            close(serverfd);
            break;
        } else if(pid > 0){
            struct sigaction act;
            act.sa_handler = reap_child;
            sigemptyset(&act.sa_mask);
            act.sa_flags = 0;

            sigaction(SIGCHLD, &act, NULL);


            close(clientfd);
        }
    }

    if(pid == 0){
        while(1){
            ret = read(clientfd, buf, sizeof(buf));
                if(ret == 0){
                close(clientfd);
                exit(1);
            }
            for(int i = 0; i < ret; i++){
                buf[i] = toupper(buf[i]);
            }
        write(clientfd, buf, ret);
        write(STDOUT_FILENO, buf, ret);
        }
    }

}




