#!/usr/bin/env bash

pushd /opt/nwn2

# Stage data on startup; overwrite if different
cp --parents -r -u \
  Campaigns \
  Data \
  Modules \
  UI \
  \
  dmvault \
  Servervault \
  \
  dialog*.TLK \
  \
  mss32.dll \
  NWN2_MemoryMgr.dll \
  NWN2_MemoryMgr_amdxp.dll \
  nwn2server.exe \
  \
  nwn?.ini \
  nwn?player.ini \
  nwnpatch.ini \
  /opt/nwn2_stage/

popd
