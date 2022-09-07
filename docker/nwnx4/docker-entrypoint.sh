#!/usr/bin/env bash

set -euo pipefail

for file in /srv/{nwnx4-user,nwn2-home,nwn2-logs}; do
  chown nwnx4:nwnx4 "$file"
done

exec "$@"