FROM debian:bullseye

ENV DISPLAY :0

# All downloads will go to /usr/local/bin
WORKDIR /usr/local/bin

# Install all base requirements
RUN dpkg --add-architecture i386 && \
    apt-get update && apt-get upgrade -y && \
    apt-get install -y software-properties-common apt-transport-https wget openssl xvfb && \
    \
    apt-get install -y python3 python3-pip && \
    pip install --no-cache --upgrade pip supervisor && \
    \
    wget -O- https://dl.winehq.org/wine-builds/winehq.key | gpg --dearmor | tee /usr/share/keyrings/winehq.gpg && \
    echo deb [signed-by=/usr/share/keyrings/winehq.gpg] https://dl.winehq.org/wine-builds/debian/ bullseye main | tee /etc/apt/sources.list.d/winehq.list && \
    apt-get update && \
    apt-get install -y --install-recommends wine32 && \
    \
    wget https://raw.githubusercontent.com/Winetricks/winetricks/master/src/winetricks && \
    chmod +x winetricks && \
    \
    apt-get clean && \
    apt-get autoclean && \
    apt-get autoremove && \
    rm -rf /var/lib/apt/lists/*

# Setup user
RUN groupadd -g 1000 nwnx4 && \
    useradd -g 1000 -u 1000 -G sudo -d /home/nwnx4 --create-home nwnx4

# All run commands will be ran as nwnx4:nwnx4
USER nwnx4:nwnx4

COPY --chown=nwnx4:nwnx4 . /srv/nwnx4/
COPY --chown=nwnx4:nwnx4 docker/nwnx4/supervisord.conf /etc/
COPY --chown=nwnx4:nwnx4 docker/nwnx4/docker-entrypoint.sh /
COPY --chown=nwnx4:nwnx4 docker/nwnx4/nwn2.reg /etc/nwn2/

# Entrypoint is set to execution
RUN chmod +x /docker-entrypoint.sh

# Apply wine setup and NWN2 registry keys
ENV WINEARCH=win32
ENV WINEDLLPATH=/etc/nwn2
ENV WINEDLLOVERRIDES="granny2,mmdevapi=b,n"

COPY --chown=nwnx4:nwnx4 docker/nwnx4/*.dll /etc/nwn2/

RUN xvfb-run winetricks -q sound=disabled && \
    xvfb-run wine regedit /C /etc/nwn2/nwn2.reg && sleep 1 && \
    rm -rf /tmp/wine* && \
    \
    mkdir -p "$HOME/.wine/drive_c/users/nwnx4/Temp/NWN2/LOGS.0" && \
    ln -s /tmp/nwn2-logs "$HOME/.wine/drive_c/users/nwnx4/Temp/NWN2/LOGS.0"

# Set to NWNX4 user directory
WORKDIR /opt/nwnx4

# Volumes with importance in the setup
VOLUME ["/srv/nwnx4", "/opt/nwnx4", "/srv/nwn2", "/opt/nwn2", "/tmp/nwn2-logs", "/home/nwnx4/.wine"]

# Expose UDP port 5121 for port access to the NWN2 server
EXPOSE 5121/udp

# Establish entrypoint; run supervisord for tasks
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["supervisord"]
