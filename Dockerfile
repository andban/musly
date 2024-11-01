FROM python:3-slim-bookworm

RUN apt-get update \
    && apt-get install -y \
        cmake \
        curl \
        g++ \
        libavcodec-dev \
        libavformat-dev \
        libavutil-dev \
        libeigen3-dev \
    && rm -rf /var/lib/apt/lists/*

COPY . /usr/src/musly

RUN mkdir /usr/src/musly/build \
    && cd /usr/src/musly/build \
    && cmake .. \
    && make \
    && make install

