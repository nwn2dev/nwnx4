FROM debian:bullseye

# Location to NWNX4 distribution
ARG NWNX4_DIST_DIR=dist/

# Install requirements
RUN dpkg --add-architecture i386 \
    && apt-get update \
    && apt-get install -y wget supervisor xvfb wine32 \
    \
    && wget https://raw.githubusercontent.com/Winetricks/winetricks/master/src/winetricks -O /usr/local/bin/winetricks \
    && chmod +x /usr/local/bin/winetricks \
    \
    && apt-get clean \
    && apt-get autoclean \
    && apt-get autoremove \
    && rm -rf /var/lib/apt/lists/*

# Setup user
RUN useradd -u 1000 -d /home/nwnx4 --create-home nwnx4

# Default user for service calls
USER nwnx4

# Copy the NWNX4 release into the image
COPY --chown=nwnx4:nwnx4 $NWNX4_DIST_DIR /opt/nwnx4/

# Add the key registry for NWN2 setup
COPY --chown=nwnx4:nwnx4 docker/nwnx4/nwn2.reg /opt/

# Configure Wine
ENV WINEARCH=win32

RUN ln -s /srv/nwn2-home "$HOME/Neverwinter Nights 2" \
    && mkdir -p "$HOME/.wine/drive_c/users/nwnx4/Temp/NWN2/LOGS.0" \
    && ln -s /srv/nwn2-logs "$HOME/.wine/drive_c/users/nwnx4/Temp/NWN2/LOGS.0" \
    \
    && xvfb-run wineboot \
    && xvfb-run winetricks -q sound=disabled \
    && xvfb-run wine regedit /C /opt/nwn2.reg \
    && wineserver --wait \
    \
    && rm -rf /tmp/wine*

# Add NWN2 stage folder to image
COPY --chown=nwnx4:nwnx4 docker/nwnx4/nwn2-stage /opt/nwn2-stage

# Associated NWN2/NWNX4 volume bindings
VOLUME ["/srv/nwnx4-user", "/srv/nwn2", "/srv/nwn2-home", "/srv/nwn2-logs"]

# Expose UDP port 5121 for port access to NWN2 server
EXPOSE 5121/udp

# Set default directory as NWNX4 user directory
WORKDIR /srv/nwnx4-user

# Run entrypoint as root to properly establish base permissions
USER root

COPY docker/nwnx4/supervisord.conf /etc/
COPY docker/nwnx4/docker-entrypoint.sh /

ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["supervisord"]
