# syntax=docker/dockerfile:1
FROM brainboxdotcc/dpp@sha256:aa3377f7d7ec6d834de2aa7beda41a13efb2cc29dbc4fca78d54658b4b63fc8a

WORKDIR /usr/src/rps-bot

COPY . .

# RUN cmake -S ../ -B .
# RUN make -j "$(nproc)"

ENTRYPOINT [ "/bin/bash" ]
# CMD [ "./src/rps_bot" ]