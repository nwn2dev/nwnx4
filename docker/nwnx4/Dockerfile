FROM debian:bullseye

# Copy the NWNX4 release into the image
ARG NWNX4_DIST_DIR=dist/

# Install requirements
RUN dpkg --add-architecture i386 \
    && apt-get update \
    && apt-get install -y wget cabextract wine32 xvfb \
    && wget https://raw.githubusercontent.com/Winetricks/winetricks/master/src/winetricks -O /usr/local/bin/winetricks \
    && chmod +x /usr/local/bin/winetricks \
    \
    && apt-get clean \
    && apt-get autoclean \
    && apt-get autoremove \
    && rm -rf /var/lib/apt/lists/*

# Setup Wine/Xvfb
ENV WINEPREFIX="/root/.wine32"
ENV WINEARCH="win32"
ENV WINEDLLOVERRIDES="mshtml=;devenum,dxdiagn,granny2=n"
ENV WINEDEBUG="fixme-all"
ENV WINE_NO_AUDIO=1
ENV AUDIODRIVER="none"
ENV DISPLAY=":0"

RUN ln -s /srv/nwn2-home "$HOME/Neverwinter Nights 2" \
    && mkdir -p "$WINEPREFIX/drive_c/users/root/Temp/NWN2/LOGS.0" \
    && ln -s /srv/nwn2-logs "$WINEPREFIX/drive_c/users/root/Temp/NWN2/LOGS.0" \
    \
    && wineboot --init \
    && winetricks -q dotnet472 \
    \
    && rm -rf /tmp/wine*

VOLUME ["/srv/nwn2-logs"]

COPY docker/nwnx4/nwn2.reg /opt/

# Setup NWN2 stage
COPY docker/nwnx4/nwn2-stage /opt/nwn2-stage

# Place the dist folder on the image
COPY $NWNX4_DIST_DIR /opt/nwnx4/

VOLUME ["/etc/nwnx4/plugins"]

# Expose UDP port 5121 for port access to NWN2 server
EXPOSE 5121/udp

# Setup entrypoint and command
WORKDIR /srv/nwnx4-user
COPY docker/nwnx4/docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["wine", "/opt/nwnx4/NWNX4_Controller.exe", "-interactive", "-verbose"]
