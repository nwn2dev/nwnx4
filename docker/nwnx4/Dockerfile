FROM debian:bullseye

# Location to NWNX4 distribution
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

# Copy the NWNX4 release into the image
COPY $NWNX4_DIST_DIR /opt/nwnx4/

# Add the key registry for NWN2 setup
COPY docker/nwnx4/nwn2.reg /opt/

# Configure Wine/Xvfb
ENV WINEARCH=win32
ENV WINEPREFIX=/root/.wine32
ENV WINEDLLOVERRIDES="mshtml=;devenum,dxdiagn,granny2=n"
ENV WINEDEBUG="fixme-all"
ENV DISPLAY=":0"

RUN ln -s /srv/nwn2-home "$HOME/Neverwinter Nights 2" \
    && mkdir -p "$HOME/.wine/drive_c/users/root/Temp/NWN2/LOGS.0" \
    && ln -s /srv/nwn2-logs "$HOME/.wine/drive_c/users/root/Temp/NWN2/LOGS.0" \
    \
    && wineboot --init \
    && wine reg import /opt/nwn2.reg \
    && winetricks -q dotnet472 \
    \
    && rm -rf /tmp/wine*

COPY docker/nwnx4/nwn2-stage /opt/nwn2-stage

# Volume bindings
VOLUME ["/srv/nwn2-logs"]

# Expose UDP port 5121 for port access to NWN2 server
EXPOSE 5121/udp

# Setup entry command
WORKDIR /srv/nwnx4-user
COPY docker/nwnx4/docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["wine", "/opt/nwnx4/NWNX4_Controller.exe", "-interactive", "-verbose"]
