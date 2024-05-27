# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:c79a2f374f057f23459b015fbaf59e72df65c185a83c1e11113ac7b3f62287ed

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.5.0-1 libfmt-dev=6.1.2+ds-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

ENTRYPOINT [ "/bin/sh" ]