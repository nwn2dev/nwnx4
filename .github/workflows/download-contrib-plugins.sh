#!/bin/bash
set -euo pipefail

DEST=${1:-.}

# args
# 1: user/repo
function get_gh_latest_release() {
	curl -s "https://api.github.com/repos/$1/releases/latest" | jq -r .name
}

if [ ! -f "$DEST/plugins/xp_rpc.dll" ]; then
	RELEASE_NAME=$(get_gh_latest_release "nwn2dev/nwnx4-plugin-rpc")
	echo "====================> Installing xp_rpc $RELEASE_NAME"
	curl -L "https://github.com/nwn2dev/nwnx4-plugin-rpc/releases/download/$RELEASE_NAME/xp_rpc-$RELEASE_NAME.zip" -o /tmp/xp_rpc.zip

	unzip -nj /tmp/xp_rpc.zip xp_rpc.dll -d "$DEST/plugins/"
	unzip -nj /tmp/xp_rpc.zip include/nwnx_rpc.nss -d "$DEST/nwscript/"
	unzip -nj /tmp/xp_rpc.zip xp_rpc.yml -d "$DEST/config.example/"
	rm /tmp/xp_rpc.zip
	echo "DONE"
fi

if [ ! -f "$DEST/plugins/xp_tlk.dll" ]; then
	RELEASE_NAME=$(get_gh_latest_release "nwn2dev/nwnx4-plugin-tlk")
	echo "====================> Installing xp_tlk $RELEASE_NAME"
	curl -L "https://github.com/nwn2dev/nwnx4-plugin-tlk/releases/download/$RELEASE_NAME/nwnx4-plugin-tlk.zip" -o /tmp/nwnx4-plugin-tlk.zip

	unzip /tmp/nwnx4-plugin-tlk.zip -d /tmp
	mv /tmp/nwnx4-plugin-tlk/{,plugins/}xp_tlk.readme.txt
	cp -R --no-clobber /tmp/nwnx4-plugin-tlk/* "$DEST/"
	rm -rf /tmp/nwnx4-plugin-tlk.zip /tmp/nwnx4-plugin-tlk/
	echo "DONE"
fi

if [ ! -f "$DEST/plugins/xp_ServerExts.dll" ]; then
	RELEASE_NAME="R3"
	echo "====================> Installing AuroraServerExts $RELEASE_NAME"
	curl -L "https://nwn2dev.blob.core.windows.net/nwn2/NWNX4/AuroraServerExts_$RELEASE_NAME.zip" -o /tmp/AuroraServerExts.zip
	curl -L "https://nwn2dev.blob.core.windows.net/nwn2/NWNX4/AuroraServerExtsSrc_$RELEASE_NAME.zip" -o /tmp/AuroraServerExtsSrc.zip

	unzip -nj /tmp/AuroraServerExts.zip xp_ServerExts.dll AuroraServerExts.pdb -d "$DEST/plugins/"
	unzip -nj /tmp/AuroraServerExts.zip AuroraServerExts.ini -d "$DEST/config.example/"
	unzip -nj /tmp/AuroraServerExtsSrc.zip ServerExts.nss -d "$DEST/nwscript/"
	mv --no-clobber "$DEST/nwscript/ServerExts.nss" "$DEST/nwscript/nwnx_ServerExts.nss"
	rm /tmp/AuroraServerExts.zip /tmp/AuroraServerExtsSrc.zip
fi

if [ ! -f "$DEST/plugins/xp_AuroraServerNWScript.dll" ]; then
	RELEASE_NAME="R12"
	echo "====================> Installing AuroraServerNWScript $RELEASE_NAME"
	curl -L "https://nwn2dev.blob.core.windows.net/nwn2/NWNX4/AuroraServerNWScript_$RELEASE_NAME.zip" -o /tmp/AuroraServerNWScript.zip

	unzip -nj /tmp/AuroraServerNWScript.zip xp_AuroraServerNWScript.dll AuroraServerNWScript.pdb Readme.txt -d "$DEST/plugins/"
	mv --no-clobber "$DEST/plugins/Readme.txt" "$DEST/plugins/xp_AuroraServerNWScript.readme.txt"
	unzip -nj /tmp/AuroraServerNWScript.zip AuroraServerNWScript.ini -d "$DEST/config.example/"
	unzip -nj /tmp/AuroraServerNWScript.zip OptionalScripts/\*.nss -d "$DEST/nwscript/"
	unzip -nj /tmp/AuroraServerNWScript.zip NWNScriptJIT.{dll,pdb} -d "$DEST/nwn2server-dll/"

	# Note: currently those DLLs cannot be loaded from nwn2server-dll/. See https://github.com/nwn2dev/nwnx4/issues/35
	mkdir -p "$DEST/copy-to-nwn2-install-dir/"
	unzip -nj /tmp/AuroraServerNWScript.zip NWNScriptJITIntrinsics.dll -d "$DEST/copy-to-nwn2-install-dir/"
	rm /tmp/AuroraServerNWScript.zip
fi
