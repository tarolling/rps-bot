# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:5ba9220e2ee540b80e2e1044347301cbb504dd2eaa1dd75e8a98d5890886d0d0

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.5.0-1 libfmt-dev=6.1.2+ds-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

ENTRYPOINT [ "/bin/sh" ]