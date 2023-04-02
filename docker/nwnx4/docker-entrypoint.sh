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
    ln -s "/srv/nwn2/$file" /opt/nwn2-stage/ && chown nwnx4:nwnx4 -h "/opt/nwn2-stage/$file"
  fi
done

# Clear all files in plugin folder; do this every startup
rm -Rf /etc/nwnx4/plugins/*;

# Copy a plugin if it exists; prefer /srv/nwnx4-user plugins
for file in $(ls /srv/nwnx4-user/plugins/*.dll | xargs -n 1 basename); do
  cp "/opt/nwnx4/plugins/$file" /etc/nwnx4/plugins/ && chown nwnx4:nwnx4 "/etc/nwnx4/plugins/$file"
done

# Copy a plugin if it exists; only add /opt/nwnx4 plugins if it doesn't exist in /srv/nwnx4-user
for file in $(ls /opt/nwnx4/plugins/*.dll | xargs -n 1 basename); do
  if [[ ! -e "/etc/nwnx4/plugins/$file" ]]; then
    cp "/opt/nwnx4/plugins/$file" /etc/nwnx4/plugins/ && chown nwnx4:nwnx4 "/etc/nwnx4/plugins/$file"
  fi
done

# All files in the /srv/nwnx4-user folder must be owned by the nwnx4 user
chown -R nwnx4:nwnx4 /srv/nwnx4-user

# .X11-unix must be created by root
mkdir -p /tmp/.X11-unix
chmod 1777 /tmp/.X11-unix

# Setup Xvfb; execute command
gosu nwnx4 bash <<-EOF
  Xvfb $DISPLAY -screen 0 1024x768x16 &
  wine reg import /opt/nwn2.reg
EOF

exec gosu nwnx4 "$@"
