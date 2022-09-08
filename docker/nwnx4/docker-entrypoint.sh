#!/usr/bin/env bash

set -euo pipefail

# Set the ownership of all core folders to nwnx4 user/group
for file in /srv/{nwnx4-user,nwn2-home,nwn2-logs}; do
  chown nwnx4:nwnx4 "$file"
done

# If the NWN2 stage file does not exist in NWN2 install directory, drop
for file in $(ls /opt/nwn2-stage); do
  if [[ ! -e "/srv/nwn2/$file" ]]; then
    rm -f "/opt/nwn2-stage/$file"
  fi
done

# If the NWN2 install file does not exist in NWN2 stage directory, add and set ownership
for file in $(ls /srv/nwn2); do
  if [[ ! -e "/opt/nwn2-stage/$file" ]]; then
    ln -s "/srv/nwn2/$file" /opt/nwn2-stage/
    chown nwnx4:nwnx4 -h "/opt/nwn2-stage/$file"
  fi
done

exec "$@"