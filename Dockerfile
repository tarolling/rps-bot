# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:7c673c7be30674a8eb11f1a63e34980590da8f513e6397e03bc6e11b634aff60

RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.5.0-1 libfmt-dev=6.1.2+ds-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

ENTRYPOINT [ "/bin/sh" ]