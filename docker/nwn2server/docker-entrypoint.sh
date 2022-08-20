#!/usr/bin/env bash

set -xe

if [ ! -f /opt/nwn2_stage/.staged ]
then
  pushd /opt/nwn2
  ls ./
  cp --parents -v \
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
  	Modules/*.mod \
  	\
  	dialog*.TLK \
  	\
  	mss32.dll \
  	NWN2_MemoryMgr.dll \
  	NWN2_MemoryMgr_amdxp.dll \
  	\
  	nwn?.ini \
  	nwn?player.ini \
  	nwnpatch.ini \
  	\
  	nwn2server.exe \
  	\
  	/opt/nwn2_stage/
  touch /opt/nwn2_stage/.staged
  popd
fi

exec "$@"
