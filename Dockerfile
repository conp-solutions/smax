FROM ubuntu:16.04

RUN apt-get update && apt-get -y upgrade && \
    apt-get -y install gcc automake zlib1g-dev make cmake g++ git build-essential

RUN apt-get -y install libgmp3-dev
