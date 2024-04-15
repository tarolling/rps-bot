# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:827d323292aa816e96326c0fcca683eed2dcec161a988c4b68ff4a4d32f4a695

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.5.0-1 libfmt-dev=6.1.2+ds-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

ENTRYPOINT [ "/bin/sh" ]