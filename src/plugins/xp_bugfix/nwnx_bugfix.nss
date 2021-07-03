
// Returns TRUE if TLS encryption is enabled for a specific player (the player
// must be running a somewhat recent version of Skywing's Client extension).
// Returns FALSE if oPC is invalid / not a player character.
//
// oPC: Target player character object
int GetIsTLSEnabled(object oPC){
    return NWNXGetInt("BUGFIX", "GET PLAYER TLS ENABLED", "", ObjectToInt(oCreature));
}

// Changes the game object update time, as configured by GameObjUpdateTime
// value in xp_bugfix.ini
//
// nMicroseconds: Loop time in microseconds
void SetGameObjectUpdateTime(int nMicroseconds){
    return NWNXSetInt("BUGFIX", "GAMEOBJUPDATETIME", "", nMicroseconds);
}