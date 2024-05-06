# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:19c7bf7116cabc2845b8da7678b480cbb27b259a5528f28f523860b0462508bf

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.5.0-1 libfmt-dev=6.1.2+ds-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

ENTRYPOINT [ "/bin/sh" ]