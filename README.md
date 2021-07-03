
# NWNX4

NWNX4 is a launcher for NWN2Server that injects code into the server process, in order
to provide additional features and bug fixes.

## Usage

1. Extract the [NWNX4 zip file](https://github.com/nwn2dev/nwnx4/releases) in any directory
2. Edit `nwnx.ini`, at least:
    + `nwn2`: full or relative path to the NWN2 install folder
    + `parameters`: nwn2server command line arguments. Examples:
        * `-module YourModuleName` if your module is a .mod file
        * `-moduledir YourModuleName` if your module is a directory
3. Delete or rename either `xp_mysql.dll` or `xp_sqlite.dll`
4. Configure your plugins (`xp_*.ini` files). Most plugins come with convenient defaults, but you may need to tweak some of them.
5. Start NWNX4:
    + Run `NWNX4_GUI.exe` for the GUI version
    + Run `NWNX4_Controller.exe -interactive` in a shell for the command-line version.


## About
NWNX4 was originally written by Virusman, Grinning Fool, McKillroy, Skywing,
Papillon and many contributors. This repository is based on the original
codebase, but with modern build tools and new maintainers.

The original source code can be found here: https://github.com/NWNX/nwnx4

# Build

## Requirements

- [Meson](https://github.com/mesonbuild/meson/releases)
- [Microsoft Visual Studio 2019
  Community](https://visualstudio.microsoft.com/downloads/#visual-studio-community-2019)
  or [MSBuild
  tools](https://visualstudio.microsoft.com/fr/downloads/?q=build+tools)

## Building

### Initialize your environment

```powershell
# Initialize git submodules (if you did not clone this repo with `--recurse`)
git submodule init
git submodule update

# Bootstrap vcpkg
vcpkg\bootstrap-vcpkg.sh

# Install dependencies (can take a while)
vcpkg\vcpkg.exe install --triplet=x86-windows-static-md
```

### Build NWNX4

From a x86 MSBuild prompt (i.e. `x86 Native Tools Command Prompt for VS 2019` if you installed Visual Studio 2019):
```powershell
# Setup build
meson builddir

# Build project
cd builddir
meson compile

# Install nwnx4 at a given location
meson install --destdir=%cd%\..\nwnx4-install
```


## Developing with Visual Studio 2019

Meson can generate solutions for Visual Studio. The following command will
create a `NWNX4.sln` that you can open with Visual Studio:
```ps
meson setup --backend=vs2019 vsbuild
```
