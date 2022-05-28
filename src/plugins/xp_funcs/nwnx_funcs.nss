
// Returns the soundset ID of a given creature, as listed in soundset.2da
int GetCreatureSoundSet(object oCreature){
	return NWNXGetInt("FUNCS", "GETSOUNDSET", "", ObjectToInt(oCreature));
}