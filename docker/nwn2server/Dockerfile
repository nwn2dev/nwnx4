FROM i386/alpine:3.16.0

ENV WINEARCH win32
ENV WINEPREFIX /root/wine
ENV DISPLAY :0

# Install requirements
RUN apk add --no-cache xvfb xvfb-run wine freetype python3 && \
    apk add --no-cache -X http://dl-cdn.alpinelinux.org/alpine/edge/testing mono winetricks && \
    apk add --update wget bash && \
    rm -rf /var/cache/apk/* && \
    python3 -m ensurepip && \
    pip3 install --no-cache --upgrade pip supervisor && \
    xvfb-run wineboot && \
    xvfb-run winetricks -q vcrun2005 vcrun2015 dotnet472 win98 sound=disabled && \
    echo "Installed requirements"

ENV WINEDEBUG -warn+all,fixme-all

# Use latest version of granny2.dll; version that comes with NWN2 doesn't work well with Wine
COPY ./docker/nwn2server/granny2.dll /opt/nwn2_stage/

# NWNX4 setup
COPY ./dist /opt/nwnx4
RUN mkdir -p /etc/nwnx4 && \
    mv /opt/nwnx4/plugins /etc/nwnx4/plugins && \
    find /opt/nwnx4 -name "*.ini" -exec mv '{}' /etc/nwnx4/ \;
COPY ./docker/nwn2server/nwnx.ini /etc/nwnx4/nwnx.ini
RUN ln -s /etc/nwnx4/*.ini /opt/nwnx4 && \
    ln -s /etc/nwnx4/plugins /opt/nwnx4/plugins

# Final
COPY ./docker/nwn2server/supervisord.conf /etc/supervisord.conf
COPY ./docker/nwn2server/docker-entrypoint.sh /
WORKDIR /opt/nwnx4
RUN chmod +x /docker-entrypoint.sh
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["supervisord"]
EXPOSE 5121
