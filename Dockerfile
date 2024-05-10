FROM ubuntu:24.04
 ARG USER_NAME
 ARG USER_ID
 ARG GROUP_ID
 RUN useradd $USER_NAME 
 RUN usermod -aG sudo $USER_NAME
 RUN echo "$USER_NAME:$USER_NAME" | chpasswd 
 RUN echo '----->'
 RUN echo 'root:Docker!' | chpasswd
 RUN apt-get update -qq
 RUN apt-get install -y vim hdparm ninja-build valgrind curl llvm gdb clang-format sudo python3 git python3-dev wget cmake g++ clang libcmocka-dev libknot-dev linux-tools-generic
 RUN addgroup --gid $GROUP_ID user; exit 0
 RUN adduser --disabled-password --gecos '' --uid $USER_ID --gid $GROUP_ID $USER_NAME; exit 0
 ENV TERM xterm-256color
 USER $USER_NAME
