# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp:latest

WORKDIR /usr/src/rps-bot

COPY . .

WORKDIR /usr/src/rps-bot/build

RUN cmake -S ../ -B .
RUN make -j$(nproc)

ENTRYPOINT [ "/bin/bash", "-l", "-c" ]
CMD [ "./src/rps_bot" ]