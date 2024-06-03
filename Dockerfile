# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:8f04f93420b791b3512ef9d3bbcce08292623935d9c4f73d854d6d6a085a66e7

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.5.0-1 libfmt-dev=6.1.2+ds-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

ENTRYPOINT [ "/bin/sh" ]