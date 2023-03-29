#!/usr/bin/env bash

set -euxo pipefail

# If the NWN2 stage file does not exist in NWN2 install directory, drop
for file in $(ls /opt/nwn2-stage); do
  if [[ ! -e "/srv/nwn2/$file" ]]; then
    rm -f "/opt/nwn2-stage/$file"
  fi
done

# If the NWN2 install file does not exist in NWN2 stage directory, add
for file in $(ls /srv/nwn2); do
  if [[ ! -e "/opt/nwn2-stage/$file" ]]; then
    ln -s "/srv/nwn2/$file" /opt/nwn2-stage/
  fi
done

# Setup all the /srv/nwn* folders and files as owned by root user
#for file in $(find /srv/nwnx4-user \! -user root); do
#  chown -f root $file
#done

Xvfb $DISPLAY -screen 0 1024x768x16 &
wine reg import /opt/nwn2.reg
exec "$@"

