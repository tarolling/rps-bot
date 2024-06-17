# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:4c4ad7524fcf5b4830c2067e11c71b4d10d736ee920c5afc545b376f2789addd

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.5.0-1 libfmt-dev=6.1.2+ds-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

ENTRYPOINT [ "/bin/sh" ]