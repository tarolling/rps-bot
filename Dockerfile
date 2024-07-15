# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:abaa043d753728ba99f1866e1234f450c0a15c4c5be28112262924d39e5a31b2

RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.12.0+ds-2build1 libfmt-dev=9.1.0+ds1-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

WORKDIR /usr/src/rps-bot/build

RUN cmake ..
RUN make -j "$(nproc)"