#!/usr/bin/env bash

set -xe

if [ ! -f /opt/nwn2_stage/.staged ]
then
  touch /opt/nwn2_stage/.staged
  cp -nr /opt/neverwinter_nights_2/* /opt/nwn2_stage/
fi

exec "$@"
