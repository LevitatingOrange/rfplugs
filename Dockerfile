FROM resin/rpi-raspbian:stretch

# Install dependencies
RUN apt-get update && apt-get install -y \
    git-core \
    build-essential \
    gcc \
    python3 \
    python3-dev \
    python3-pip \
    python3-virtualenv \
    python3-setuptools \
    --no-install-recommends && \
    rm -rf /var/lib/apt/lists/*

RUN pip3 install pyserial toml paho-mqtt
RUN git clone git://git.drogon.net/wiringPi
RUN cd wiringPi && ./build
RUN pip3 install wiringpi2

RUN git clone https://github.com/technion/lol_dht22 /lol_dht22_src
RUN cd lol_dht22_src && ./configure && make
RUN mv /lol_dht22_src/loldht /loldht && rm -Rf /lol_dht22_src

ENV CONF_FILE /example_conf.toml

WORKDIR /src
COPY ./example_conf.toml /
COPY ./src /src
RUN gcc -o rfplug rfplug.c -lwiringPi

CMD ["python3", "-u", "rflamps.py"]