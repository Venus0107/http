# http
c언어 HTTP 서버

# 컴파일
gcc -o http http.c -pthread

# 명령어
./http <port>

# notice
candy.jpg 는 content-type: image/jpeg로 이미지를 불러오려고 했으나 WSL환경에서 .jpg 파일의 경로나 파일을 읽어오지 못하여 html로 이미지를 불러오는 형식으로 했습니다.
