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
    xvfb-run wineboot && \
    xvfb-run winetricks -q sound=disabled && \
    wget -qO - https://artifacts.elastic.co/GPG-KEY-elasticsearch | apt-key add - && \
    apt-get install -y apt-transport-https && \
    echo "deb https://artifacts.elastic.co/packages/8.x/apt stable main" | tee -a /etc/apt/sources.list.d/elastic-8.x.list && \
    apt-get update && apt-get install -y logstash && \
    ln -s /usr/share/logstash/bin/logstash /usr/local/bin/logstash && \
    apt-get clean && \
    apt-get autoclean && \
    apt-get autoremove && \
    rm -rf /var/lib/apt/lists/*

# NWN2 setup
RUN mkdir -p /tmp/nwn2
COPY docker/nwn2server/granny2.dll /opt/nwn2_stage/

# NWNX4 setup
COPY NWNX4_Controller.exe /opt/nwnx4/
COPY NWNX4_Hook.dll /opt/nwnx4/
COPY nwn2server-dll/*.dll /opt/nwnx4/nwn2server-dll/
COPY plugins/*.dll /opt/nwnx4/plugins/
COPY *.ini /opt/nwnx4/
COPY docker/nwn2server/nwnx.ini /opt/nwnx4/
COPY docker/nwn2server/logstash.conf /opt/nwnx4/
COPY docker/nwn2server/init.d/ /etc/nwn2server/init.d/

# Final setup
COPY docker/nwn2server/supervisord.conf /etc/
COPY docker/nwn2server/docker-entrypoint.sh /
WORKDIR /opt/nwnx4
RUN chmod +x /docker-entrypoint.sh

# Establish volumes (folders outside context directory)
VOLUME /opt/nwn2_stage
VOLUME /root/Neverwinter\ Nights\ 2

# Expose UDP port 5121 for port access
EXPOSE 5121/udp

# Establish entrypoint; run supervisord for tasks
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["supervisord"]
