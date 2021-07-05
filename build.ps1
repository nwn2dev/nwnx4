meson setup meson-build-debug --buildtype=debug
#meson setup meson-build-release --buildtype=release

pushd meson-build-debug
ninja
cp ./src/hook/NWNX4_Hook.dll ./src/gui/
cp ./src/hook/NWNX4_Hook.dll ./src/controller/
popd

#pushd meson-build-release
#ninja
#popd
