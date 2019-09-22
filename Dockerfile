FROM ubuntu:disco
RUN apt update
RUN apt upgrade -y
RUN apt install -y manpages-dev manpages-posix-dev
RUN apt install -y build-essential autotools-dev autoconf libtool
RUN apt install -y vim emacs
RUN apt install -y man
RUN yes | unminimize
RUN apt install -y lsof psmisc
RUN apt install -y valgrind
RUN apt install -y gdb

WORKDIR /conc
