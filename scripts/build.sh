#!/bin/bash
docker build --pull -f "Dockerfile" -t rpsbot:latest "." --no-cache