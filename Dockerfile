# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:c45f2729ccc4da337e41ee27a05479692a649a3c40390b10f906bcf7cb190803

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.5.0-1 libfmt-dev=6.1.2+ds-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

ENTRYPOINT [ "/bin/sh" ]