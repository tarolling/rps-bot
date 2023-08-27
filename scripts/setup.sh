#!/bin/bash
docker build --pull --rm -f "Dockerfile" -t rpsbot:latest "." --no-cache