# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:dc956faaf4d54c1a2e56c5386e9980815f8eaed14730304ff95414b09d97b6fc

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.5.0-1 libfmt-dev=6.1.2+ds-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

ENTRYPOINT [ "/bin/sh" ]