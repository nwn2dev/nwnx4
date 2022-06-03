FROM alpine:edge

ENV WINEDEBUG=fixme-all
ENV WINEARCH=win32
ENV WINEPREFIX=/root/.wine32

# Set arch
RUN echo "x86" > /etc/apk/arch

# Install wine and tools
WORKDIR /usr/local/bin
RUN apk add --no-cache xvfb xvfb-run wine && \
    apk add --no-cache --repository=http://dl-cdn.alpinelinux.org/alpine/edge/testing winetricks && \
    apk add --update wget
    # xvfb-run winetricks -q vcrun2005 vcrun2015 dotnet472

# Misc. NWNX4 setup
WORKDIR /opt/nwnx4
COPY ./dist .

WORKDIR /etc/nwnx4
COPY ./dist/config.example/*.ini .
COPY ./dist/plugins/*.dll ./plugins
COPY ./docker/nwn2server/nwnx.ini /etc/nwnx4/nwnx.ini

WORKDIR /opt/nwnx4
RUN ln -s /etc/nwnx4/*.ini . && \
    ln -s /etc/nwnx4/plugins ./plugins

# Post
CMD ["tail", "-f", "/dev/null"]
#ENTRYPOINT ["xvfb-run", "wine"]
#CMD ["NWNX4_Controller.exe", "-interactive"]

EXPOSE 5121
