# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:0f14cc382cd94004a806735dc82e97de4f3587dc59c27b66d302106534e711ed

RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.12.0+ds-2build1 libfmt-dev=9.1.0+ds1-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

WORKDIR /usr/src/rps-bot/build

RUN cmake ..
RUN make -j "$(nproc)"