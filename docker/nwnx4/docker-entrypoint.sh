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

# Clear all files in plugin folder; do this every startup
rm -Rf /etc/nwnx4/plugins/*;

# Copy a plugin if it exists in the /srv/nwnx4-user/plugins folders
for file in $(ls /srv/nwnx4-user/plugins/*.dll); do
  cp $file /etc/nwnx4/plugins/
done

for file in $(ls /opt/nwnx4/plugins/*.dll | xargs -n 1 basename); do
  if [[ ! -e "/etc/nwnx4/plugins/$file" ]]; then
    cp "/opt/nwnx4/plugins/$file" /etc/nwnx4/plugins/
  fi
done

Xvfb $DISPLAY -screen 0 1024x768x16 &
wine reg import /opt/nwn2.reg
exec "$@"

