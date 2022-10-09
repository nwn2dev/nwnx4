#!/bin/bash
set -euo pipefail

DEST=${1:-.}

# args
# 1: user/repo
function get_gh_latest_release() {
  curl -s "https://api.github.com/repos/$1/releases/latest" | jq -r .name
}


RELEASE_NAME=$(get_gh_latest_release "nwn2dev/nwnx4-plugin-rpc")
echo "====================> Installing xp_rpc $RELEASE_NAME"

curl -L "https://github.com/nwn2dev/nwnx4-plugin-rpc/releases/download/$RELEASE_NAME/xp_rpc-$RELEASE_NAME.zip" -o /tmp/xp_rpc.zip
unzip -nj /tmp/xp_rpc.zip xp_rpc.dll -d "$DEST/plugins/"
unzip -nj /tmp/xp_rpc.zip include/nwnx_rpc.nss -d "$DEST/nwscript/"
unzip -nj /tmp/xp_rpc.zip xp_rpc.yml -d "$DEST/config.example/"
rm /tmp/xp_rpc.zip
echo "DONE"


RELEASE_NAME=$(get_gh_latest_release "nwn2dev/nwnx4-plugin-tlk")
echo "====================> Installing xp_tlk $RELEASE_NAME"

curl -L "https://github.com/nwn2dev/nwnx4-plugin-tlk/releases/download/$RELEASE_NAME/nwnx4-plugin-tlk.zip" -o /tmp/nwnx4-plugin-tlk.zip
unzip /tmp/nwnx4-plugin-tlk.zip -d /tmp
mv /tmp/nwnx4-plugin-tlk/{,plugins/}xp_tlk.readme.txt
cp -R --no-clobber /tmp/nwnx4-plugin-tlk/* "$DEST/"
rm -rf /tmp/nwnx4-plugin-tlk.zip /tmp/nwnx4-plugin-tlk/
echo "DONE"
