FROM alpine:3.16.0

# Set arch
RUN echo "x86" > /etc/apk/arch

# Install wine and tools
RUN apk add --no-cache xvfb wine wget cabextract && \
    wineboot && \
    wineserver -w && \
    cd "/usr/local/bin" && \
    wget https://raw.githubusercontent.com/Winetricks/winetricks/master/src/winetricks && \
    chmod +x winetricks

# Misc. NWNX4 cleanup
COPY ./dist /opt/nwnx4
RUN mv /opt/nwnx4/config.example /etc/nwnx4 && \
    mv -f /opt/nwnx4/plugins /etc/nwnx4/plugins
COPY ./docker/nwn2server/nwnx.ini /etc/nwnx4/nwnx.ini
RUN ln -s /etc/nwnx4/*.ini /opt/nwnx4 && \
    ln -s /etc/nwnx4/plugins /opt/nwnx4/plugins

# Post
WORKDIR /opt/nwnx4
EXPOSE 5121

COPY ./docker/nwn2server/docker-entrypoint.sh /docker-entrypoint.sh
ENTRYPOINT ["/bin/sh", "/docker-entrypoint.sh"]
CMD ["wine", "/opt/nwnx4/NWNX4_Controller.exe", "-interactive"]
