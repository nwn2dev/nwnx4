FROM i386/alpine:3.16.0

ENV WINEARCH=win32
ENV WINEPREFIX=/root/.wine32
ENV WINEDEBUG=fixme-all

# Install wine and tools
WORKDIR /usr/local/bin
RUN apk add --no-cache xvfb xvfb-run wine && \
    apk add --no-cache --repository=http://dl-cdn.alpinelinux.org/alpine/edge/testing mono winetricks && \
    apk add --update wget
RUN xvfb-run winecfg && \
    winetricks -q vcrun2005 && \
    winetricks -q vcrun2015 && \
    winetricks -q dotnet472

# NWNX4 setup
WORKDIR /opt/nwnx4
COPY ./dist .
RUN mkdir -p /etc/nwnx4 && \
    mv ./plugins /etc/nwnx4/plugins && \
    mv ./*.ini /etc/nwnx4
COPY ./docker/nwn2server/nwnx.ini /etc/nwnx4/nwnx.ini
RUN ln -s /etc/nwnx4/*.ini . && \
    ln -s /etc/nwnx4/plugins ./plugins

# Cleanup
COPY ./docker/nwn2server/docker-entrypoint.sh /
RUN chmod +x
ENTRYPOINT ["sh", "/docker-entrypoint.sh"]
CMD ["xvfb-run", "wine", "NWNX4_Controller.exe", "-interactive"]
EXPOSE 5121
