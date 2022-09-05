FROM debian:bullseye

ARG NWNX4_DIST_DIR=dist/

ENV DISPLAY :0

# All downloads will go to /usr/local/bin
WORKDIR /usr/local/bin

# Install all base requirements
RUN dpkg --add-architecture i386 \
    && apt-get update && apt-get upgrade -y \
    && apt-get install -y software-properties-common apt-transport-https wget openssl xvfb \
    \
    && apt-get install -y python3 python3-pip \
    && pip install --no-cache --upgrade pip supervisor \
    \
    && wget -qO- https://dl.winehq.org/wine-builds/winehq.key | gpg --dearmor > /etc/apt/trusted.gpg.d/wine.gpg \
    && echo 'deb http://dl.winehq.org/wine-builds/debian/ bullseye main' > /etc/apt/sources.list.d/wine.list \
    && apt-get update \
    && apt-get install -y --install-recommends winehq-stable \
    \
    && wget https://raw.githubusercontent.com/Winetricks/winetricks/master/src/winetricks \
    && chmod +x winetricks \
    \
    && apt-get clean \
    && apt-get autoclean \
    && apt-get autoremove \
    && rm -rf /var/lib/apt/lists/*

# Setup base distribution setup and base Docker commands
COPY $NWNX4_DIST_DIR/* /srv/nwnx4/
COPY docker/nwnx4/supervisord.conf /etc/
COPY docker/nwnx4/docker-entrypoint.sh /
COPY docker/nwnx4/nwn2.reg /root/.wine/

# Entrypoint is set to execution
RUN chmod +x /docker-entrypoint.sh

# Apply wine setup and NWN2 registry keys
ENV WINEARCH=win32
ENV WINEDEBUG=-fixme

RUN xvfb-run wineboot \
    && xvfb-run winetricks -q sound=disabled \
    && xvfb-run wine regedit /C "$HOME/.wine/nwn2.reg" && sleep 5 \
    && rm -rf /tmp/wine* \
    \
    && mkdir -p "$HOME/.wine/drive_c/users/nwnx4/Temp/NWN2/LOGS.0" \
    && ln -s /tmp/nwn2-logs "$HOME/.wine/drive_c/users/nwnx4/Temp/NWN2/LOGS.0"

# Add any DLLs in the nwnx4 image folder to system32, so we can use them instead of local copies
COPY docker/nwnx4/*.dll /home/nwnx4/.wine/drive_c/windows/system32/

# Set to NWNX4 user directory
WORKDIR /opt/nwnx4

# Volumes with importance in the setup
VOLUME ["/srv/nwnx4", "/opt/nwnx4", "/srv/nwn2", "/opt/nwn2", "/tmp/nwn2-logs", "/home/nwnx4/.wine"]

# Expose UDP port 5121 for port access to the NWN2 server
EXPOSE 5121/udp

# Establish entrypoint; run supervisord for tasks
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["supervisord"]
