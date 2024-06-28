FROM ubuntu:latest
WORKDIR /opt

ARG SDK_VERSION=2.4.2

# Install Playdate SDK and dependencies
RUN apt-get update -q && apt-get install -qy --no-install-recommends \
    curl \
    ca-certificates \
    git \
    make \
    cmake \
    gcc-arm-none-eabi \
    libpng-dev \
    libnewlib-arm-none-eabi \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

RUN curl -OL https://download.panic.com/playdate_sdk/Linux/PlaydateSDK-${SDK_VERSION}.tar.gz
RUN tar -xf PlaydateSDK-${SDK_VERSION}.tar.gz 
RUN mv PlaydateSDK-${SDK_VERSION} playdate
ENV PATH="${PATH}:/opt/playdate/bin"
ENV PLAYDATE_SDK_PATH="/opt/playdate"

# Set the working directory
WORKDIR /app

# Copy your project files into the container
COPY . .

# Build
RUN mkdir build && cd build && cmake -DCMAKE_TOOLCHAIN_FILE=$PLAYDATE_SDK_PATH/C_API/buildsupport/arm.cmake .. && make
