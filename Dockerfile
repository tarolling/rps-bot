# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:d02f228a08a107c82058c31cee70241e0ce60362ddef50020d3f76cf0e97c4d2

RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.12.0+ds-2build1 libfmt-dev=9.1.0+ds1-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

WORKDIR /usr/src/rps-bot/build

RUN cmake ..
RUN make -j "$(nproc)"