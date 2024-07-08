# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:f8d2f10e5350d785005e825c877d0df70b7a738d799a13674000c0fae1541642

RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.12.0+ds-2build1 libfmt-dev=9.1.0+ds1-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

WORKDIR /usr/src/rps-bot/build

RUN cmake ..
RUN make -j "$(nproc)"