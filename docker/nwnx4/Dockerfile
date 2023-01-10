FROM debian:bullseye

# Location to NWNX4 distribution
ARG NWNX4_DIST_DIR=dist/

# Install requirements
RUN dpkg --add-architecture i386 \
    && apt-get update \
    && apt-get install -y supervisor software-properties-common gnupg gnupg1 gnupg2 \
    \
    && apt-get install -y wget \
    && wget -O- https://dl.winehq.org/wine-builds/winehq.key | gpg --dearmor | tee /usr/share/keyrings/winehq.gpg \
    && apt-key add /usr/share/keyrings/winehq.gpg \
    && add-apt-repository 'deb https://dl.winehq.org/wine-builds/debian/ bullseye main' \
    && apt-get update \
    && apt-get install -y --install-recommends winehq-stable:i386 \
    && apt-get install -y xvfb \
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

# Configure Wine/Xvfb
ENV WINEARCH=win32
ENV WINEDLLOVERRIDES="mscoree,mshtml="
ENV WINEDEBUG="fixme-all"
ENV DISPLAY=":0"

RUN ln -s /srv/nwn2-home "$HOME/Neverwinter Nights 2" \
    && mkdir -p "$HOME/.wine/drive_c/users/nwnx4/Temp/NWN2/LOGS.0" \
    && ln -s /srv/nwn2-logs "$HOME/.wine/drive_c/users/nwnx4/Temp/NWN2/LOGS.0" \
    \
    && wineboot --init \
    && wine reg import /opt/nwn2.reg

# Add NWN2 DLL folder to image
COPY --chown=nwnx4:nwnx4 docker/nwnx4/nwn2-stage /opt/nwn2-stage

# Drop all Wine tmp files
USER root
RUN rm -rf /tmp/wine*

# Associated NWN2/NWNX4 volume bindings
VOLUME ["/srv/nwnx4-user", "/srv/nwn2", "/srv/nwn2-home", "/srv/nwn2-logs"]

# Expose UDP port 5121 for port access to NWN2 server
EXPOSE 5121/udp

# Set default directory as NWNX4 user directory
WORKDIR /srv/nwnx4-user

# Run entrypoint as root to properly establish base permissions
COPY docker/nwnx4/docker-entrypoint.sh /
COPY docker/nwnx4/supervisord.conf /etc/

ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["supervisord", "-c", "/etc/supervisord.conf"]
