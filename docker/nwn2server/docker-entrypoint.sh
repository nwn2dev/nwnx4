#!/usr/bin/env bash

set -xe

if [ ! -f /opt/nwn2_stage/.staged ]
then
  cp -nr /opt/nwn2/* /opt/nwn2_stage/
  touch /opt/nwn2_stage/.staged
fi

exec "$@"
