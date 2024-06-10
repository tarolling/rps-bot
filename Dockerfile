# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:aab5167fb9fd4859c997486402b7b4d34b0e4e20342af84b8bff0003bddb60d3

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.5.0-1 libfmt-dev=6.1.2+ds-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

ENTRYPOINT [ "/bin/sh" ]