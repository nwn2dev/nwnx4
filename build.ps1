meson setup meson-build-debug --buildtype=debug
#meson setup meson-build-release --buildtype=release

pushd meson-build-debug
ninja
popd

#pushd meson-build-release
#ninja
#popd
