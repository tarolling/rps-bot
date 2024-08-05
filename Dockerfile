# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:98e461a6cc7f49add0112a5330d90a1eae87871d938f2c8f9f42cfb3bf018c8e

RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.12.0+ds-2build1 libfmt-dev=9.1.0+ds1-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

WORKDIR /usr/src/rps-bot/build

RUN cmake ..
RUN make -j "$(nproc)"