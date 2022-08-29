#!/usr/bin/env bash

set -xe

for file in /etc/nwn2server/init.d/*; do
  if [[ -f $file ]]; then
    source "$file"
  fi
done

exec "$@"
