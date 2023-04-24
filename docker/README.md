## NWNX4 Docker

NWNX4 Docker is a way to deploy NWNX4 in a Docker Linux container.

## How-to

There is a docker-compose.yml file available in this folder that helps to build and lift 
your copy. You will need to buildthe distribution of the NWNX4 application first.

NOTE: Make sure the build directory is using --buildtype=release.

1. Run `meson install --destdir=../dist` from your NWNX4 build directory in a VS 2019 
   x86 Dev. Command Prompt. If your destdir is within a dist folder at the root of the 
   repository as in this instruction, you will not need to set the Docker argument 
   `NWNX4_DIST_DIR` during your `docker-compose build` step.
3. Set the following environment variables:
   1. `NWN2_HOME_DIR` i.e. C:\Users\youruser\OneDrive\Documents\Neverwinter Nights 2
   2. `NWN2_INSTALL_DIR` i.e. C:\Program Files (x86)\Steam\steamapps\common\Neverwinter Nights 2
   3. `NWNX4_USER_DIR` i.e. C:\nwnx4-user
4. Run `docker-compose build` from this directory (./docker).
5. Run `docker-compose up -d` to start the service daemon. By default, the server will be accessible from port 5121.
