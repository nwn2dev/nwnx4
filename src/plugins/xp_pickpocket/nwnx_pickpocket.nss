// Pickpocketting Framework
// By: GodBeastX

void PickpocketCancel()
{
	NWNXGetInt("PICK", "1", "", 1);
}

void PickpocketContinue()
{
	NWNXGetInt("PICK", "1", "", 0);
}

object PickpocketGetTarget()
{
	int nOID = NWNXGetInt("PICK", "0", "", 0);
	
	return IntToObject(nOID);
}