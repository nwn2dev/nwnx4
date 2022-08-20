#!/usr/bin/env bash

set -xe

pushd /opt/nwn2

# Stage data on startup; do not overwrite
cp --parents -v -n \
  Campaigns/Neverwinter\ Nights\ 2\ Campaign*/Campaign.cam \
  \
  Data/2DA*.zip \
  Data/convo*.zip \
  Data/Ini*.zip \
  Data/lod-merged*.zip \
  Data/NWN2_Models*.zip \
  Data/scripts*.zip \
  Data/SpeedTree*.zip \
  Data/Templates*.zip \
  Data/walkmesh*.zip \
  \
  dialog*.TLK \
  \
  mss32.dll \
  NWN2_MemoryMgr.dll \
  NWN2_MemoryMgr_amdxp.dll \
  nwn2server.exe \
  \
  /opt/nwn2_stage/

# Stage all modules and configuration files if there are changes
cp --parents -v -u \
  Modules/*.mod \
  \
  nwn?.ini \
  nwn?player.ini \
  nwnpatch.ini \
  /opt/nwn2_stage/
popd

exec "$@"
