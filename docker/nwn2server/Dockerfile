FROM debian:latest

ENV DISPLAY :0

# Install requirements
WORKDIR /usr/local/bin
RUN dpkg --add-architecture i386 && \
    apt-get update && \
    apt-get install -y wget && \
    wget  https://raw.githubusercontent.com/Winetricks/winetricks/master/src/winetricks && \
    chmod +x winetricks && \
    apt-get install -y xvfb wine python3 python3-pip && \
    pip install --no-cache --upgrade pip supervisor && \
    xvfb-run wineboot && \
    xvfb-run winetricks -q sound=disabled && \
    apt-get clean && \
    apt-get autoclean && \
    apt-get autoremove && \
    rm -rf /var/lib/apt/lists/*

# NWN2 setup
RUN mkdir -p /tmp/nwn2
COPY ./docker/nwn2server/granny2.dll /opt/nwn2_stage/

# NWNX4 setup
COPY . /opt/nwnx4

# Final
COPY ./docker/nwn2server/supervisord.conf /etc/
COPY ./docker/nwn2server/docker-entrypoint.sh /
WORKDIR /opt/nwnx4
RUN chmod +x /docker-entrypoint.sh
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["supervisord"]

EXPOSE 5121/udp
