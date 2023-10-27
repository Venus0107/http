#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#define BUF_SIZE 1024
#define BACKLOG 5

char *idex = "/idex.html";
char *candyname = "/candy";
void send_error(char *);
void *GET(int *);
void html(int*);


int main(int argc, char* args[]){
    int sockfd;
    struct sockaddr_in servaddr, clntaddr;
    int clntfd, clntaddr_size;
    pthread_t t_id; 

    sockfd = socket(AF_INET, SOCK_STREAM, 0);   

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(atoi(args[1])); 
    if(bind(sockfd,(struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        send_error("bind error");
    }
    if(listen(sockfd, BACKLOG) < 0){
        send_error("listen error");
    }

    while(1){
    printf("Waiting....\n\n");
    clntaddr_size = sizeof(clntaddr); 
    if((clntfd = accept(sockfd, (struct sockaddr*)&clntaddr, &clntaddr_size)) < 0){
    
                send_error("accept error");
    }
    printf("Connection : %s : %d\n\n",inet_ntoa(clntaddr.sin_addr), ntohs(clntaddr.sin_port));

    pthread_create(&t_id, NULL, GET, &clntfd);
    pthread_detach(t_id); 
    }
    close(sockfd);
}
void *GET(int *clntfd){
    int fd = *clntfd, fp;
    char buf[BUF_SIZE]={0x00, };
    char *str1;
    char *URI;

    memset(buf, 0, sizeof(buf));


    read(fd, buf, BUF_SIZE); 
    printf("Request HTTP message\n");
    printf("%s", buf);

    str1 = strtok(buf, " \r\n"); 
    URI = strtok(NULL, " "); 

    if(strcmp(URI, idex) == 0 || strlen(URI) == 1){ 
        html(&fd);
    }
    else if(strstr(URI, ".jpg") != NULL){ 
        char *candy = strtok(URI, "."); 
        if(strcmp(candyname, candy) == 0 ){  
            jpg(&fd); 
        }
        else{
            write(fd,"HTTP/1.1 404 Bad Request\n", 25);
            close(fd);
        }
    }
    else{
        write(fd,"HTTP/1.1 400 Bad Request\n", 25);
        close(fd);
    }
}
void send_error(char *message){ 
    fputs(message, stderr); 
    fputc('\n', stderr);
    exit(0);
}

void html(int *clntfd){
        int fd = *clntfd, fp;
        char buf[BUF_SIZE]={0x00, };
        char *res="HTTP/1.1 200 OK\r\nContent-Type:text/html;charset=UTF-8\n\n"; 
   

        fp=open("./idex.html", O_RDONLY);

        memset(buf, 0, sizeof(buf)); 

        write(fd, res, 55); 
	    read(fp, buf, sizeof(buf)); 
        write(fd, buf, sizeof(buf)); 
        close(fd); 
        return NULL;
}
void jpg(int *clntfd){ 
        int fd = *clntfd, fp;
        char buf[BUF_SIZE]={0x00, };
        char *res="HTTP/1.1 200 OK\n\n";

        fp=open("./image.html", O_RDONLY);

        memset(buf, 0, sizeof(buf));

        write(fd, res, 17);
	    read(fp, buf, sizeof(buf));
        write(fd, buf, sizeof(buf));
        close(fd);
        return NULL;
}