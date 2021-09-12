VaultSTER plugin for NWNX2
==========================

  
2005 by Jeroen Broekhuizen  
Email: [jeroen@nwnx.org](mailto:jeroen@nwnx.org)  
Website: [nwnx.org](http://nwnx.org)  
Windows Version: 1.4.6  
Linux version: 0.3.0

Documentation and program are © by Jeroen Broekhuizen 2005

### Contents:

1.  [Welcome](#chap1)
    1.  [New features](#chap11)
2.  [Upgrade](#chap2)
3.  [Full install](#chap3)
4.  [Configuration](#chap4)
5.  [Scripting](#chap5)
6.  [Security](#chap6)
7.  [End marks](#chap7)
8.  [Todo](#chap8)
9.  [Faq](#chap9)

**IMPORTANT!**  
This version is NOT compatible with previous version due to some coding differences on Windows and Linux. So be sure to update all your servers with this new version when you deside to upgrade to this version.

1\. Welcome
-----------

Vaultster can be described as a character transfer software, which makes it easy to link up multiple (autonomous) worlds to increase player base and space. Using traditional ways of sharing servervaults over the internet using distributed systems can be unreliable or have other side effects. To circumvent this I designed and implemented the Vaultster software based on the standard TCP/IP protocol available on every platform including Windows and Linux. Via compression using the well known zip method (rar is not free to use) high transmission speeds are ensured and by using a well known encryption method the security of the system is increased.

### 1.a New in this release

The most important new aspect of this release is that it is now available for Linux distributions too without any limitations or what soever. Other features include:

*   Automatic detection of an already running Vaultster server on the same machine (only under Windows). This prefents binding to the same port errors, etc.
*   A new security feature: DNS or IP validation. When this feature is turned on every client is validated against a list of known DNS names. If the client is not known the connection will be terminated. So, only known servers may transmit their character files to your server.

I have rewritten the complete software from scratch resulting in better manageble, readable and stable (and perhaps slightly faster) code.

The latest version of this document is available on the documentation page of [http://www.nwnx.org](http://www.nwnx.org).

2\. Upgrade
-----------

Upgrading VaultSter is a fairly simple task as it is still compatible with your NWN scripts with exception of the STARTSERVER function (see below). Start with removing the 'nwnx\_send.dll' file from your Neverwinter Nights directory. When you have done so place the new 'nwnx\_vaultster.dll' file in the NWN directory and rename it to nwnx\_send.dll. See the Configuration section about setting up this new Vaultster.

After configuring the plugin there is still one thing to do. Remove any calls to the STARTSERVER function from your modules. This new version of Vaultster has a building check to make sure that only one Vaultster Server is running on that machine. When you have finished this step you are ready for using Vaultster.

3\. Full install
----------------

Copy the nwnx\_vaultster.dll and nwnx\_vaultster.ini files to your Neverwinter Nights directory. Now follow the sections Configuration and Scripting to set up Vaultster with your module. When using linux make sure that you have installed the zlib library. For windows also copy the zlib.dll to the Neverwinter Nights directory.

4\. Configuration
-----------------

This step will lead you through the configuration of the Vaultster plugin. This is done entirely in the nwnx2.ini file.

Create a new section in the configuration file called VAULTSTER. This section will contain both server settings as the known server list with DNS names or IP addresses (prior versions had different sections for this, so now take those in one section).

The Vaultster section contains the following options:

*   port : the TCP/IP port to which Vaultster should bind.
*   path : full path to the servervault of your server.
*   startserver : when 1 the VaultSter Server will be started, on 0 it will not.
*   key : the encryption method needs a key of characters (see Security section)
*   password : character string (see Security section)
*   count : the number of servers that are known
*   serverN : server DNS name, where N is an increasing number starting at 1.

Now both Windows and Linux users should add the options to the configuration file manually. Take a look at the nwnx\_vaultster.ini for an example.

5\. Scripting
-------------

I included an erf in the package (build with version 1.64) which includes an includable script vaulster\_inc. Import it via File -> Import and make sure it gets imported correctly ;). The functions described below only portal the files to another server not the player himself. You must call the NWN portal function yourself in a script (see below for an example).

You must include the file as usual (#include "vaultster\_inc") before you can use the functions or constants declared in it. Before starting with coding I recommend you to read the comments of the functions first. They exactly describe what results you can expect from the functions.

The first function is called _PortalPC_ and takes two parameters. The first parameter is the player object that should be transfered to the server which is given as the second parameter. Before anything it transferred the script exports the character to ensure that any latest changes are saved to file too. See the comments for descriptions of the return values.

The second function returns the status for a player object. Transmission of the file takes place on the background so the NWN server can continue working on other things while Vaultster takes care of the transmission. With this function you can see what the current status is of the transmission. In case it returns the busy value, you can wait a second or two and call this function again, till you get a success or failure message. On a success message the file was transferred correctly and the player can be portalled to the other server. In case of a failure you can allow the player to run around again as the file transfer did not succeed. You should not portal the player over to the other server.

The following example shows you how you can do implement a script to portal a player:

```cpp
void main ()
{
   object oPC = GetEnteringObject();
   int ret = PortalPC  (oPC, "nwn.someserver.org");
   if (ret == VAULTSTER_OK) {
      SendMessageToPC (oPC, "Portalling...");
      DelayCommand (1.0f, ExecuteScript ("CheckStatus", oPC));
   }
   else
       SendMessageToPC (oPC, "Failure with portalling.");
}
```

The code above calls the PortalPC function with the object that entered a transition. When the function returns successfull the CheckStatus script will be called after one second, which is given below.

```cpp
void main ()
{
   object oPC = OBJECT_SELF;
   int status = PortalStatus (oPC);
   if (status == VAULTSTER_STATUS_OK)
      // file was transferred successfully, so now portal the player
      ActivatePortal (oPC, "nwn.someserver.com", "", "", TRUE);
   else if (status == VAULTSTER_STATUS_BUSY) {
      // vaultster is still busy with the transmission
      SendMessageToPC (oPC, "Hold on..");
      DelayCommand (1.0f, ExecuteScript ("CheckStatus", oPC));
   }
   else {
      // failure during transmission or not able to connect (see log file)
      SendMessageToPC (oPC, "Failed to portal the character file.");
   }
}
```

This function is slightly more difficult to read, but I will explain it now. First it calls the status function available from the vaultster\_inc file. Depending on the status that is returned we do either 1) on success we send the player to the other server, 2) transmission is not finished, so we must wait a little longer and 3) an error occured, you should take a look in the log file and see if there is an error message there.

6\. Security
------------

Security is an important aspect for Vaultster. Currently there are two mechanisms available to protect your servervault against possible cheaters. The first and original mechanism is based on a secret password which should only be known between known servers. This password is should be given in the configuration file as 'password'. This password is send over the internet during the handshaking process.

Before it actually will be transferred it is first encrypted with the BlowFish algorithm. For this algorithm there is also a key needed to able to encrypt anything which also should be kept private between your known servers. You can supply the key in the configuration file in the 'key' option.

Both the password and the key can have an arbitrary length. For the password it recommended to limit to 84 characters so there will be added enough noice.

7\. End marks
-------------

All named products (NWScript, NWServer, ect) are NOT written by me and their developer has all rights. Also we are not responsible for any damage/loss of data this NWNX Vaultster plugin, or any library that it depends on, does to your system. You use this software on your own risc. The source code is made publicly under the GNU license as the rest of NWNX is. See licence at [here](licence.txt).

Todo
----

*   Public key encryption

FAQ
---

**1\. Why can't I connect to Vaultster?**

Make sure that Vaultster is loaded and that the Vaultster server is started. This should be listed in the nwnx\_vaultster.txt log file. If you are behind a router or firewall, make sure that the port you listed in the configuration file is open and directed to the correct machine.

**2\. Is Vaultster also available for Linux?**

Read the documentation! ;) Yes, from this version Linux is also supported and you can now link Windows versions to Linux versions easily.