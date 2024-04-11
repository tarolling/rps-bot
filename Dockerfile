# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:7768537ec0d42eb5ec198bb6762555703dd2288f446ffa5876e190f9ffbf7f7f

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev="1:1.5.0-1" libfmt-dev="6.1.2+ds-2" \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

ENTRYPOINT [ "/bin/sh" ]