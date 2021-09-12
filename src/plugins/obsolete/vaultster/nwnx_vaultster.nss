// Name     : VaultSTER portal system include
// Purpose  : Various VaultSTER related functions
// Authors  : Jeroen Broekhuizen, Ignmar Stieger, Patrice Torguet (Hialmar)
// Modified : January 12, 2005, June 8, 2008

// Updates:
//  7-2-2005 - fixed bug in the PortalStatus script
//  6-8-2008 - ported to NWN2 / NWNX4


// This file is licensed under the terms of the
// GNU GENERAL PUBLIC LICENSE (GPL) Version 2

// Error codes returned by VaultSter
const int VAULTSTER_OK = 0;
const int VAULTSTER_FAILED = -1;
const int VAULTSTER_SERVERBUSY = -2;
const int VAULTSTER_NOPLAYER = -3;
const int VAULTSTER_INTERNALERROR = -4;

// Status messages
const int VAULTSTER_STATUS_OK   = 0;
const int VAULTSTER_STATUS_BUSY = 2;
const int VAULTSTER_STATUS_ERROR= 3;

// Start portaling a player to the requested server. Use PortalStatus
// to keep track of the status of the portal for this player.
// returns: VAULTSTER_OK portal was successfully initiated
//          VAULTSTER_FAILED internal failure
//          VAULTSTER_SERVERBUSY too much players portalling already
//          VAULTSTER_NOPLAYER object was not a player
int PortalPC(object oPC, string Server);

// Checks the current status of portalling for the specified player
// returns: VAULTSTER_STATUS_OK transmission was successfull
//          VAULTSTER_STATUS_BUSY transmission is not yet finished
//          VAULTSTER_STATUS_ERROR there was an error during the transmission
int PortalStatus(object oPC);

//////////////////////////////
// Implementation
//////////////////////////////

// Convers a player name to it's filename
// returns: filename of the character
string convertToFilename(string sPlayerName)
{
    string sRes;
    string sChar;
    int i, j;

    int iLen = GetStringLength(sPlayerName);
    if (iLen > 16)
        iLen = 16;

    for (i = 0; i < iLen; i++) {
        sChar = GetSubString(sPlayerName, i + j, 1);
        if ((sChar != " ") && (sChar != "."))
            sRes += sChar;
        else {
            i--;
            j++;
        }
    }
    return sRes;// + ".bic";
}

int PortalPC(object oPC, string Server)
{
    // make sure it is a PC object
    if (!GetIsPC (oPC)) return VAULTSTER_NOPLAYER;

    // make sure that the latest changes are saved too
    ExportSingleCharacter (oPC);

    // fetch the required information
    string player = GetPCPlayerName (oPC);
    string character = convertToFilename (GetName (oPC));
	// start the portalling
    // build up the command string
    string command = Server + "|" + player + "|" + character;
    int id =  NWNXGetInt("VAULTSTER", "SEND", command, 0);

    // check the result
    if (id < 0) {
		if(id == VAULTSTER_FAILED) {
			WriteTimestampedLogEntry("Vaulster: Internal failure for character: "+character+" player: "+player+" server:"+Server);
		} else if(id == VAULTSTER_SERVERBUSY) {
			WriteTimestampedLogEntry("Vaulster: Too many players are portalling already, wait a little and try again. Character: "+character+" player: "+player+" server:"+Server);
		} else if(id == VAULTSTER_NOPLAYER) {
			WriteTimestampedLogEntry("Vaulster: Object was not a player. Object name: "+character+" server:"+Server);
		}
        // return the error message
        return id;
    } else {
        // save the id on the player (+1 as id is zero based what is
        // problematic with retreiving the id in PortalStatus, as on
        // error GetLocalString results also 0).
        SetLocalInt (oPC, "VAULTSTER_ID", id+1);
        return VAULTSTER_OK;
    }
}

int PortalStatus(object oPC)
{
    // get the portal id from the player
    int id = GetLocalInt (oPC, "VAULTSTER_ID");
    if (id == 0) return VAULTSTER_STATUS_ERROR;

    // get the status from Vaultster
    int status =  NWNXGetInt("VAULTSTER", "STATUS", "", id-1);

    // delete if neccessary the vaultster id from the player
    if (status == VAULTSTER_STATUS_OK || status == VAULTSTER_STATUS_ERROR) {
        DeleteLocalInt (oPC, "VAULTSTER_ID");
    }
    return status;
}
