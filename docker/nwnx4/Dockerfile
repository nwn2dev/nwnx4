FROM debian:bullseye

ARG RELEASE_URL="https://github.com/nwn2dev/nwnx4/releases/latest/download/nwnx4.zip"
ARG NWNX4_USER_HOME="/home/nxnx4"

# Install requirements
RUN dpkg --add-architecture i386 \
    && apt-get update \
    && apt-get install -y gosu wget cabextract xvfb openssl \
    && apt-get install -y gnupg2 software-properties-common \
    \
    && wget -qO- https://dl.winehq.org/wine-builds/winehq.key | apt-key add - \
    && apt-add-repository https://dl.winehq.org/wine-builds/debian/ \
    && apt-get update \
    && apt-get install -y --install-recommends winehq-stable \
    \
    && wget https://raw.githubusercontent.com/Winetricks/winetricks/master/src/winetricks -O /usr/local/bin/winetricks \
    && chmod +x /usr/local/bin/winetricks \
    \
    && apt-get install -y curl unzip \
    \
    && apt-get clean \
    && apt-get autoclean \
    && apt-get autoremove \
    && rm -rf /var/lib/apt/lists/*

# Setup nwnx4 user
RUN useradd -u 1000 -d $NWNX4_USER_HOME --create-home -ms /bin/bash nwnx4

# Give access to NWNX4 etc folder to nwnx4 user
RUN mkdir -p /etc/nwnx4/plugins \
    && chown nwnx4:nwnx4 -R /etc/nwnx4

# Set to nwnx4 user; allows wine to be configured correctly
USER nwnx4:nwnx4

# Setup Wine/Xvfb
ENV WINEPREFIX="$NWNX4_USER_HOME/.wine32"
ENV WINEARCH="win32"
ENV WINEDLLOVERRIDES="mshtml=;devenum,dxdiagn,granny2=n"
ENV WINEDEBUG="fixme-all"
ENV WINE_NO_AUDIO=1
ENV AUDIODRIVER="none"
ENV DISPLAY=":0"

RUN ln -s /srv/nwn2-home "$NWNX4_USER_HOME/Neverwinter Nights 2" \
    && mkdir -p "$WINEPREFIX/drive_c/users/nwnx4/Temp/NWN2" \
    && ln -s /srv/nwn2-logs "$WINEPREFIX/drive_c/users/nwnx4/Temp/NWN2/LOGS.0" \
    \
    && wineboot --init \
    && xvfb-run winetricks -q dotnet48 vcrun2005 vcrun2015 \
    \
    && rm -rf /tmp/wine*

VOLUME ["/srv/nwn2-logs"]

# Setup NWN2 stage and registry overrides
COPY --chown=nwnx4:nwnx4 docker/nwnx4/nwn2-stage /opt/nwn2-stage
COPY --chown=nwnx4:nwnx4 docker/nwnx4/nwn2.reg /opt/

# Add temp. folder for X11
RUN mkdir -p /tmp/.X11-unix \
    && chmod 1777 /tmp/.X11-unix

# Fix to ALSA configuration
COPY --chown=nwnx4:nwnx4 docker/nwnx4/.asoundrc $NWNX4_USER_HOME

# Expose UDP port 5121 for port access to NWN2 server
EXPOSE 5121/udp

# Setup entrypoint and command
USER root

# Setup release
RUN curl -LJO $RELEASE_URL \
    && unzip nwnx4.zip -d /opt \
    && chown -R nwnx4:nwnx4 /opt/nwnx4

WORKDIR /srv/nwnx4-user
COPY docker/nwnx4/docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["wine", "/opt/nwnx4/NWNX4_Controller.exe", "-interactive", "-verbose"]
