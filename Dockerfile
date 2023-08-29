# syntax=docker/dockerfile:1
FROM ubuntu:23.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install --no-install-recommends -y libssl-dev zlib1g-dev libsodium-dev libopus-dev cmake pkg-config g++ gcc git make wget dpkg ca-certificates \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

WORKDIR /usr/src/rps-bot/build

RUN cmake -S ../ -B .
RUN make -j "$(nproc)"

ENTRYPOINT [ "/bin/bash", "-l", "-c" ]
CMD [ "./RPS-BOT" ]