FROM i386/alpine:3.16.0

ENV WINEDLLOVERRIDES="mscoree,mshtml="
ENV WINEPREFIX="/root/wine"

# Install wine and tools
RUN apk add --no-cache xvfb xvfb-run wine freetype && \
    apk add --no-cache -X http://dl-cdn.alpinelinux.org/alpine/edge/testing mono winetricks && \
    apk add --update wget && \
    rm -rf /var/cache/apk/* && \
    xvfb-run wineboot && \
    xvfb-run winetricks -q vcrun2005 vcrun2015 dotnet472 && \
    echo "Finished installing all requirements"

# NWNX4 setup
WORKDIR /opt/nwnx4
#COPY ./dist .
#RUN mkdir -p /etc/nwnx4 && \
#    mv ./plugins /etc/nwnx4/plugins && \
#    mv ./*.ini /etc/nwnx4
#COPY ./docker/nwn2server/nwnx.ini /etc/nwnx4/nwnx.ini
#RUN ln -s /etc/nwnx4/*.ini . && \
#    ln -s /etc/nwnx4/plugins ./plugins

# Final
COPY ./docker/nwn2server/docker-entrypoint.sh /
RUN chmod +x /docker-entrypoint.sh

ENTRYPOINT ["sh", "/docker-entrypoint.sh"]
CMD ["tail", "-f", "/dev/null"]
#CMD ["xvfb-run", "wine", "NWNX4_Controller.exe", "-interactive"]
EXPOSE 5121
