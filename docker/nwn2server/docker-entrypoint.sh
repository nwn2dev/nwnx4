Xvfb :0 -screen 0 1280x1024x8 &
winetricks -q allfonts vcrun2005 vcrun2015 dotnet472
DISPLAY=:0.0
exec "$@"