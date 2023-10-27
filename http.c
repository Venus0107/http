#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h> // 소켓과 스레드를 위한 각종 헤더들
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
    pthread_t t_id; /* 스레드는 프로세스 내에서 고유한 스레드 ID로 구분하게 되는데, 이 스레드 ID를 나타내는 데이터 타입이 pthread_t 이다. 
    pthread_t 는 운영체제마다 다르게 구현되어 있기 때문에 다양한 데이터 타입으로 구현할 수 있도록 허용되어 있다.*/

    sockfd = socket(AF_INET, SOCK_STREAM, 0);    /*AF_INET :IPv4를 사용, SOCK_STEAM : 연결지향의 TCP/IP 기반 통신에 사용,  
    세번째 인자값으로 0을 넣어주게 되면 첫번째 두번째 매개변수를 기준으로 인자값이 지정된다 -> IPPROTO_TCP가 자동 설정
    반환값 : -1(실패), 0이상(성공, 소켓 객체를 가르키는 숫자)*/

    servaddr.sin_family = AF_INET; // IPv4 모드
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  // INADDR_ANY : 모든 주소에서 받겠다(0.0.0.0 ~ 255.255.255.255)
    servaddr.sin_port = htons(atoi(args[1])); /* 파일 실행시 두번째 인자값을 포트로 받아옴. htons : short 메모리 값을 호스트 바이트 바이트 순서에서 네트워크 바이트 순서로
    atoi : string을 정수값으로 변환*/

    if(bind(sockfd,(struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        send_error("bind error");
    }/* sockfd : 앞서 socket 함수로 생성된 소켓의 식별번호, &serv_addr : IP주소와 port 번호를 저장하기 위한 변수가 있는 구조체,
    sizeof(serv_addr) : 두번째 인자의 데이터 크기(주소 길이)*/
    if(listen(sockfd, BACKLOG) < 0){
        send_error("listen error");
    }/*int listen(int socket, int queueLimit) 첫번째 인자 socket 사용할 소켓, 두번째 인자 BAKLOG는 연결 요구 개수의 최댓값
        listen() 함수는 대기만 할뿐 연결을 해주는 함수가 아님*/

    while(1){
    printf("Waiting....\n\n");
    clntaddr_size = sizeof(clntaddr); // 클라이언트 주소 길이(sockaddr_in 구조체 크기)를 clntaddr_size에 저장
    if((clntfd = accept(sockfd, (struct sockaddr*)&clntaddr, &clntaddr_size)) < 0){
        // sockfd의 생성된 소켓을 받아 clntaddr을 연결 주소로 설정하여 새로운 소켓(새로운 디스크립터)을 생성한다. 세 번째 인자는 두번째 인자의크기
                send_error("accept error");
    }
    printf("Connection : %s : %d\n\n",inet_ntoa(clntaddr.sin_addr), ntohs(clntaddr.sin_port));
    // 연결된 주소와 포트번호

    pthread_create(&t_id, NULL, GET, &clntfd); // t_id : 생성된 스레드 id를 저장하는 변수의 주소값, NULL 설정시 기본 스레드 생성을 의미, GET함수를 스레드로 생성, 4번째 인자 GET함수의 인자값을 넣어줌
    pthread_detach(t_id); // main함수의 실행과 스레드 함수를 독립시키며 스레드의 종료 후 자동 소멸 유도
    }
    close(sockfd);
}
void *GET(int *clntfd){
    int fd = *clntfd, fp;
    char buf[BUF_SIZE]={0x00, };
    char *str1;
    char *URI;

    memset(buf, 0, sizeof(buf)); // buf배열의 buf사이즈만큼 초기화


    read(fd, buf, BUF_SIZE); // 클라이언트에서 받은 HTTP메세지의 내용(fd)을 buf에 저장
    printf("Request HTTP message\n");
    printf("%s", buf);

    str1 = strtok(buf, " \r\n"); // 클라이언트에게 받은 메세지의 첫줄에서 문자열 시작부터 NULL
    URI = strtok(NULL, " "); // NULL부터 NULL까지의 문자열 주소를 URI포인터에 저장

    if(strcmp(URI, idex) == 0 || strlen(URI) == 1){ // 문자열비교. 웹브라우저의 경로 이름이 /idex.html이거나 / 하나라면 html 함수 실행
        html(&fd);
    }
    else if(strstr(URI, ".jpg") != NULL){ // URI 포인터값과 .jpg 문자열 비교후 있다면 포인터 반환
        char *candy = strtok(URI, "."); // URI문자열을 .을 기준으로 나눈후 문자열 시작 포인터 반환
        if(strcmp(candyname, candy) == 0 ){  // "/candy"와 방금 반환받음 포인터 값의 문자열을 비교
            jpg(&fd); // /candy.jpg가 맞다면 jpg 함수 실행
        }
        else{
            write(fd,"HTTP/1.1 404 Bad Request\n", 25); // 조건이 맞지 않을경우 404 에러 상태코드를 보냄
            close(fd);
        }
    }
    else{
        write(fd,"HTTP/1.1 400 Bad Request\n", 25); // 조건이 맞지 않을경우 400 에러 상태코드를 보냄
        close(fd);
    }
}
void send_error(char *message){ // 에러메세지 반환 함수
    fputs(message, stderr); // 표준 에러출력 스트림 stderr으로 매개변수로 받은 message 문자열 출력
    fputc('\n', stderr);
    exit(0);
}

void html(int *clntfd){
        int fd = *clntfd, fp;
        char buf[BUF_SIZE]={0x00, };
        char *res="HTTP/1.1 200 OK\r\nContent-Type:text/html;charset=UTF-8\n\n"; /* HTTP 메세지 시작줄과 헤더. 
        HTTP/1.1 프로토콜 버전으로 200 OK상태 코드 발송 헤더 Content-type 으로 바디에 어떤 타입의 파일을 보여줘야하는지 알려줌*/

        fp=open("./idex.html", O_RDONLY); // 읽기모드로 현재경로의 idex.html 파일을 열어 파일 디스크립터를 int형 fp에 저장

        memset(buf, 0, sizeof(buf)); // buf배열 초기화

        write(fd, res, 55); // HTTP 메세지 res에 들어있는 문자열을 클라이언트와 연결된 소켓을 통해 보냄
	    read(fp, buf, sizeof(buf)); // idex.html 파일을 읽어와 buf배열에 저장
        write(fd, buf, sizeof(buf)); // buf에 저장되어 있는 내용을 소켓을 통해 전송
        close(fd); // 소켓을 닫음
        return NULL;
}
void jpg(int *clntfd){ // html함수와 작동 원리는 같음
        int fd = *clntfd, fp;
        char buf[BUF_SIZE]={0x00, };
        char *res="HTTP/1.1 200 OK\n\n"; // HTTP 프로토콜 GET 방식은 value=text로 보내지기 때문에 content-type을 안써줘두 된다.

        fp=open("./image.html", O_RDONLY);

        memset(buf, 0, sizeof(buf));

        write(fd, res, 17);
	    read(fp, buf, sizeof(buf));
        write(fd, buf, sizeof(buf));
        close(fd);
        return NULL;
}