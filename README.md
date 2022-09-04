
# NWNX4

NWNX4 is a launcher for NWN2Server that injects code into the server process,
in order to provide additional features and bug fixes.

### About

NWNX4 was originally written by Virusman, Grinning Fool, McKillroy, Skywing,
Papillon and many contributors. This repository is based on the original
codebase, but with modern build tools and new maintainers.

The original source code can be found here: https://github.com/NWNX/nwnx4

## Usage

### Requirements
You must install:
- [Visual C++ 2005 x86 Runtime](https://download.microsoft.com/download/8/B/4/8B42259F-5D70-43F4-AC2E-4B208FD8D66A/vcredist_x86.EXE) <!-- xp_bugfix -->
- [Visual C++ 2015 x86 Runtime](https://download.microsoft.com/download/9/3/F/93FCF1E7-E6A4-478B-96E7-D4B285925B00/vc_redist.x86.exe) <!-- msvc 2019 -->
- [.NET Framework 4.7.2](https://download.visualstudio.microsoft.com/download/pr/1f5af042-d0e4-4002-9c59-9ba66bcf15f6/124d2afe5c8f67dfa910da5f9e3db9c1/ndp472-kb4054531-web.exe) or above <!-- xp_bugfix -->

### Installation


1. Download and extract the [NWNX4 zip
   file](https://github.com/nwn2dev/nwnx4/releases) in `C:\Program Files
   (x86)\nwnx4` (recommended). This folder is referenced as the "_Install
   dir_"
2. Create a new empty folder somewhere in your user profile, like
   `%USERPROFILE%\Documents\nwnx4`. This folder is referenced as the "_User
   dir_".
3. Copy all files from `C:\Program Files (x86)\nwnx4\config.example\` to your
   _User dir_ (`%USERPROFILE%\Documents\nwnx4`)
4. Customize the configuration files you just copied to suit your needs:
    - `nwnx.ini`:
        + `plugin_list`: the list of all nwnx4 plugins that will be loaded.
          You may want to add or remove some of them. Additional plugins
          should be installed in the `plugins` folder in _User dir_ (you may
          need to create it).
        + `parameters`: nwn2server command line arguments. Examples:
            * `-module YourModuleName` to automatically load a module as a
              .mod file
            * `-moduledir YourModuleName` if your module is a directory
    - `xp_*.ini`: these are the plugins configuration files. Most plugins are
      shipped with convenient defaults, but you may need to tweak some of
      them. Note that the presence of a plugin configuration file does not
      mean the plugin is loaded (see `plugin_list` in `nwnx.ini`).

There are 3 methods for launching the server:
- Using `NWNX4_GUI.exe` (graphical interface)
    + Create a shortcut to `NWNX4_GUI.exe`, open its properties and edit the
      "Run in" field to set to your _User dir_.
    + Then Double click the shortcut to launch the server
- Using `NWNX4_Controller.exe` (command-line)
    + Run `NWNX4_Controller.exe -userdir "%USERPROFILE%\Documents\nwnx4"
      -interactive` in a cmd window. If `-userdir` is not provided, the
      current working directory will be used instead.
- Using a Windows service:
    + Run `NWNX4_Controller.exe -userdir "%USERPROFILE%\Documents\nwnx4"
      -installservice` in a cmd window to register the service. If `-userdir`
      is not provided, the _User dir_ will be the current working directory.
    + Run `NWNX4_Controller.exe -startservice` to start the service and launch
      the server


#### Alternative install: single-folder

Before NWNX4 v1.2.0, the _Install dir_ and _User dir_ were the same folder.
This behavior is still supported but discouraged for server setups, as it
makes updating NWNX4 harder.

Double-clicking `NWNX4_GUI.exe` will launch NWNX4 with merged folder.


### Updating NWNX4

1. Delete or rename your current NWNX4 _Install dir_
2. Replace it with the latest [NWNX4 zip
   file](https://github.com/nwn2dev/nwnx4/releases)
3. Compare the new config files in `config.example/` with your existing config
   files in the NWNX4 _User dir_. Read the release notes to see exactly what
   changed.
4. Copy the `.nss` files from the `nwscript/` folder into your module folder,
   **or** import `nwscript/nwnx.erf` into your module using the NWN2 toolset.
   Overwrite existing files if prompted.


# Building from sources

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

# Bootstrap vcpkg (prepare dependencies)
vcpkg\bootstrap-vcpkg.bat

# Compile and install dependencies
vcpkg\vcpkg.exe install --triplet=x86-windows-static-md --clean-after-build
```

### Build NWNX4

From a x86 MSBuild prompt (i.e. `x86 Native Tools Command Prompt for VS 2019` if you installed Visual Studio 2019):
```powershell
# Setup build files
meson builddir

# Compile project
cd builddir
meson compile

# Install nwnx4 at a given location
meson install --destdir=%cd%\..\nwnx4-dev
```


## Developing with Visual Studio 2019

Meson can generate solutions for Visual Studio. The following command will
create a `NWNX4.sln` that you can open with Visual Studio:
```ps
meson setup --backend=vs2019 vsbuild
```


# How to: Distribute nwnx4 with your module

This section will guide you through the process of creating a folder / zip
file containing everything players will need to quickly run a NWN2 server with
your custom content, for single-player or multi-player usage.

### Skeleton

Download this repository and copy the `package-skel` directory to any
location. This folder contains some handy scripts, and the base structure for
your package, that you will need to configure and fill with your content.

```
package-skel\
├── ClientExtension\  Contains Skywing's Client Extension
├── home\             Replaces Documents\Neverwinter Nights 2.
├── nwnx4\            Contains NWNX4 files and config
├── start-game.bat    Launcher for the game (with the client extension)
└── start-server.bat  Launcher for the server (with nwnx4)
```

### NWNX4

Download the [nwnx4 zip file](https://github.com/nwn2dev/nwnx4/releases), and
extract it into `package-skel\nwnx4\`.

You must configure NWNX4 as explained in nwnx4's
[README.md](https://github.com/nwn2dev/nwnx4#first-installation).

In order to run nwnx4 in a portable manner, you must add `-home
"%NWNX4_DIR%\..\home"` to the `parameters` option in `nwnx.ini`. Other
arguments are recommended but not required:

```ini
# Configure the parameters to auto-start your module
# -home <DIR>           The provided path will replace the Documents\Neverwinter Nights 2\ folder
# -moduledir <MODNAME>  Your module name (if you're saving in directory mode)
# -module <MODNAME>     Your module name without the .mod extension (if you're not in directory mode)
# -publicserver         Disable server online advertising
# -maxclients           Maximum number of connected players
# -servervault 1        Makes the server automatically save the character in the servervault folder
parameters = -home "%NWNX4_DIR%\..\home" -moduledir YourModule -publicserver 0 -maxclients 1 -servervault 1
```

### Game launcher

Skywing's Client Extension can automatically detect NWN2 installation
directory and provides a better NWN2 player experience. [Download it from the
NWVault](https://neverwintervault.org/project/nwn2/other/nwn2-client-extension)
and extract the zip into the `package-skel\ClientExtension\` folder.

As is, `start-game.bat` will:
1. Copy `nwn2.ini` and `nwn2player.ini` from `Documents\Neverwinter Nights 2\`
   to `package-skel\home\` (so player game settings are kept). If these files
   are already present in the home folder, they will not be overwritten. You
   can use this behavior to provide your own custom `nwn2player.ini`.
2. Launch the game using the Client Extension

Additionnally, `start-game-autoconnect.bat` will also automatically connect to
the server at `127.0.0.1:5121`.


## Custom content

You must install your custom content (modules, campaigns, haks, tlks,
override, ...) inside the `package-skel\home\` folder. This custom content
will be used by both the server and the game client.

## Docker setup

Running NWN2 server in Linux is now no longer a pipe dream through Docker.

| Env. Variable      | Definition                                                                          | Default? |
|--------------------|-------------------------------------------------------------------------------------|----------|
| NWN2_INSTALL_DIR   | The full path to your Neverwinter Nights 2 installation directory path on the host. | N/A      |
| NWN2_HOME_DIR      | The full path to your Neverwinter Nights 2 home directory, on your user account.    | N/A      |
| SERVER_PORT        | External server port used for your NWN2 server.                                     | 5121     |

The only required environment variables are NWN2_INSTALL_DIR and NWN_HOME_DIR. Once defined, lifting the server is a 
single command ran from the distribution:

```powershell
docker-compose up -d
```

There are environment variables used on the nwnx.ini file that pertain to setting the configuration. In most cases,
you will not have to modify any of the environment variables except NWNX4_PLUGIN_LIST and NWNX4_PARAMETERS.

| Env. Variable                        | Definition                                                                       | Default?                                                                  |
|--------------------------------------|----------------------------------------------------------------------------------|---------------------------------------------------------------------------|
| NWNX4_PLUGIN_LIST                    | List of all plugins to load when starting the NWN2Server process                 | xp_bugfix, xp_sqlite, xp_funcs, xp_time, xp_srvadmin, xp_objectattributes |
| NWNX4_NWN2                           | Path to NWN2 installation (where nwn2server.exe is located)                      | /opt/nwn2                                                                 |
| NWNX4_NWN2TEMP                       | You can override the tempory path NWN2 uses                                      | /tmp/nwn2                                                                 |
| NWNX4_PARAMETERS                     | Command line parameters to start the server with                                 | -module 0_Tutorial                                                        |
| NWNX4_PROCESS_WATCHDOG               | Should NWNX restart the server if the server process has gone away ?             | 1                                                                         |
| NWNX4_GAMESPY_WATCHDOG               | Should NWNX restart the server if it locks up ?                                  | 1                                                                         |
| NWNX4_GAMESPY_INTERVAL               | How often should the gamespy watchdog query the server ?                         | 30                                                                        |
| NWNX4_GAMESPY_TOLERANCE              | How often may the server not react to gamespy queries ?                          | 4                                                                         |
| NWNX4_GAMESPY_DELAY                  | How long should NWNX wait before the gamespy watchdog kick in ?                  | 30                                                                        | 
| NWNX4_RESTART_DELAY                  | If the server crashes, how long should NWNX wait before it restarts the server ? | 5                                                                         |
| NWNX4_NO_GP_FAULT_ERROR_BOX          | Should NWNX disable the general protection fault error dialog ?                  | 1                                                                         |
| NWNX4_RESTART_CMD                    | If you want NWNX to execute a CMD or BAT file before restarting the server after | ""                                                                        |
| NWNX4_GRACEFUL_SHUTDOWN_TIMEOUT      | How long should NWNX wait for the server to shutdown gracefully                  | 10                                                                        |
| NWNX4_GRACEFUL_SHUTDOWN_MESSAGE      | You can enter a server message that will be sent to all players                  | Server is shutting down NOW!                                              |
| NWNX4_GRACEFUL_SHUTDOWN_MESSAGE_WAIT | How long should the shutdown server message be displayed ?                       | 5                                                                         |
