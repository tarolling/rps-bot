# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:7c673c7be30674a8eb11f1a63e34980590da8f513e6397e03bc6e11b634aff60

RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.12.0+ds-2build1 \
    libfmt-dev=9.1.0+ds1-2 libcurl4-openssl-dev=8.5.0-2ubuntu10 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

WORKDIR /usr/src/rps-bot/build

RUN cmake ..
RUN make -j "$(nproc)"