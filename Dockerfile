# Start from Ubuntu base image
FROM ubuntu:latest

RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install -y curl
RUN apt-get install -y git
RUN apt-get install -y unzip
RUN apt-get install -y tar
RUN apt-get install -y gcc g++
RUN apt-get install -y wget
RUN apt-get install -y cmake
RUN apt-get install -y ninja-build
RUN apt-get install -y build-essential
RUN apt-get install -y clang=1:14.0-55~exp2 
RUN apt-get autoremove -y
RUN apt-get clean
RUN dpkg --configure -a 
RUN apt-get install -y --no-install-recommends clang++=1:14.0-55~exp2
RUN apt-get autoremove -y
RUN apt-get clean
RUN apt-get install -y zip
RUN git clone https://github.com/microsoft/vcpkg.git && ./vcpkg/bootstrap-vcpkg.sh -disableMetrics

WORKDIR /workspace
COPY . .
