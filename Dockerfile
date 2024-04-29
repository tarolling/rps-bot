# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:3e232b355a806bfab4cc2d89d6f40bf079024a2d6d542aada2ac167d8af883d6

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.5.0-1 libfmt-dev=6.1.2+ds-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

ENTRYPOINT [ "/bin/sh" ]