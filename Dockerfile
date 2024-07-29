# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:8ce4ba0a6d115d30cafdde76127c08f0812c5bb5b119599c9ff3e0623317e1ba

RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.12.0+ds-2build1 libfmt-dev=9.1.0+ds1-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

WORKDIR /usr/src/rps-bot/build

RUN cmake ..
RUN make -j "$(nproc)"