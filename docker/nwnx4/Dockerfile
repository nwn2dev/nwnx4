FROM debian:bullseye

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
    apt-get clean && \
    apt-get autoclean && \
    apt-get autoremove && \
    rm -rf /var/lib/apt/lists/*

# Setup user
RUN groupadd -g 1000 nwnx4 && \
    useradd -g 1000 -G sudo -u 1000 -d /home/nwnx4 --create-home nwnx4

# All commands after this point are nwnx4:nwnx4
USER nwnx4:nwnx4

COPY --chown=nwnx4:nwnx4 . /srv/nwnx4/
COPY --chown=nwnx4:nwnx4 docker/nwnx4/supervisord.conf /etc/
COPY --chown=nwnx4:nwnx4 docker/nwnx4/docker-entrypoint.sh /
COPY --chown=nwnx4:nwnx4 docker/nwnx4/nwn2.reg /etc/nwn2/
RUN chmod +x /docker-entrypoint.sh

# Apply wine setup and NWN2 registry keys
RUN xvfb-run wineboot && \
    xvfb-run winetricks -q sound=disabled && \
    xvfb-run wine regedit /C /etc/nwn2/nwn2.reg && \
    rm -rf /tmp/wine* && \
    mkdir -p "$HOME/.wine/drive_c/users/nwnx4/Temp/NWN2/LOGS.0" && \
    ln -s /tmp/nwn2-logs "/home/nwnx4/.wine/drive_c/users/nwnx4/Temp/NWN2/LOGS.0"

WORKDIR /opt/nwnx4

VOLUME ["/srv/nwnx4", "/opt/nwnx4", "/srv/nwn2", "/opt/nwn2", "/tmp/nwn2-logs"]

# Expose UDP port 5121 for port access
EXPOSE 5121/udp

# Establish entrypoint; run supervisord for tasks
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["supervisord"]
