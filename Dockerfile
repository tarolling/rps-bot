# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:3e1585c457a09ee6bd11f25e9df90f78b99c23d955c9f4cfcb8dbea0ad42d8c6

RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.12.0+ds-2build1 libfmt-dev=9.1.0+ds1-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

WORKDIR /usr/src/rps-bot/build

RUN cmake ..
RUN make -j "$(nproc)"