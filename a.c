#include<sys/socket.h>
#include<sys/stat.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<string.h>
 
#define MAXLINE 1024
 
int main(int argc, char **argv)
{
    int server_sockfd, client_sockfd;
    int client_len, n;
    char buf[MAXLINE];
    char ans[1024];
 
    struct sockaddr_in clientaddr, serveraddr;
    client_len = sizeof(clientaddr);
 
    if((server_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP )) == -1){
        perror("서버 소켓 에러 : ");
        exit(0);
    }
    
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(atoi(argv[1]));
 
    if( bind(server_sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) == -1 ){
        printf("Bind error : ");
        exit(1);        
    }
 
    if( listen(server_sockfd, 5) == -1){
        printf("Listen error : ");
        exit(1);
    }
 
    if((client_sockfd = (accept(server_sockfd, (struct sockaddr *) &clientaddr, &client_len))) > 0){
            printf("새로운 클라이언트 전속 IP : %s\n",inet_ntoa(clientaddr.sin_addr));
    }
    
    while(1){
            if(client_sockfd == -1){
            printf("client connection fail");
            exit(1);
        }
        memset(buf, 0x00, MAXLINE);
        read(client_sockfd, buf, MAXLINE);    // 클라이언트의 데이터를 수신한다 (대기모드)
        printf("IP : %s : %s \n",inet_ntoa(clientaddr.sin_addr), buf);
        
        memset(buf, 0x00, MAXLINE);
        
        read(0, ans, 1024);
        write(client_sockfd, ans, 8);
    }
    close(client_sockfd);
    close(server_sockfd);
    return 0;
}
