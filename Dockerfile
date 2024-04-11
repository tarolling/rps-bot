# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:483d69270fcd497101abd054ca46ec1c661a2ed4b37c09ec976e028338792739

RUN apt-get update && apt-get install --no-install-recommends -y libspdlog-dev libfmt-dev && apt-get clean

WORKDIR /usr/src/rps-bot

COPY . .

ENTRYPOINT [ "/bin/sh" ]