if exist home\nwn2player.ini (
    echo nwn2player.ini is already present
) else (
    echo Copying nwn2player.ini to packaged home folder
    copy "%UserProfile%\Documents\Neverwinter Nights 2\nwn2player.ini" home\nwn2player.ini
)

if exist home\nwn2.ini (
    echo nwn2.ini is already present
) else (
    echo Copying nwn2.ini to packaged home folder
    copy "%UserProfile%\Documents\Neverwinter Nights 2\nwn2.ini" home\nwn2.ini
)

ClientExtension\NWLauncher.exe -home %CD%\home +connect localhost:5121