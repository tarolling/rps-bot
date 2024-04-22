# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:804cb4d967451b2460a51e38737e56ee50f784565af318902c43b58091f56d25

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev=1:1.5.0-1 libfmt-dev=6.1.2+ds-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/rps-bot

COPY . .

ENTRYPOINT [ "/bin/sh" ]