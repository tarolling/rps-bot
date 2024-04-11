# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:7768537ec0d42eb5ec198bb6762555703dd2288f446ffa5876e190f9ffbf7f7f

RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev libfmt-dev && apt-get clean

WORKDIR /usr/src/rps-bot

COPY . .

ENTRYPOINT [ "/bin/sh" ]