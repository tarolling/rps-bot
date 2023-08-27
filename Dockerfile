# syntax=docker/dockerfile:1
FROM ubuntu:23.04

RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install --no-install-recommends \
    -y libssl-dev zlib1g-dev libsodium-dev libopus-dev cmake \
    pkg-config g++ gcc git make wget dpkg ca-certificates

RUN wget -O dpp.deb https://dl.dpp.dev/
RUN dpkg -i dpp.deb
RUN rm dpp.deb

WORKDIR /usr/src/rps-bot

COPY . .

WORKDIR /usr/src/rps-bot/build

RUN cmake -S ../ -B .
RUN make -j "$(nproc)"
RUN make

ENTRYPOINT [ "/bin/bash", "-l", "-c" ]
CMD [ "./RPS-BOT" ]