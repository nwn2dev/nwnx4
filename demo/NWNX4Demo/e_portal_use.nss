// Name     : e_portal_use
// Purpose  : on used script that prompts to ensure that the user wants to use the portal 
// Authors  : Grinning Fool [Marc Paradise]
// Modified : December 5th, 2006

void main()
{
	object oPC = GetLastUsedBy();
	string name = GetLocalString(OBJECT_SELF, "mod");
	if (name == "") { 
		SendMessageToPC(oPC, "There is no plugin attached to this gateway.");
		return;
	}
	SetLocalObject(oPC, "portal", OBJECT_SELF);
	DisplayMessageBox(
	 	GetLastUsedBy(), // User to show message to
		 -1, // message strref
		"Are you sure you wish to proceed to the '" + name + "' demo and setup module?", // message tex
		"gui_portal_ok",  // script to invoke on Yes processed
		"", // Cancel script callback
		TRUE, // show cancel button
		"SCREEN_MESSAGEBOX_DEFAULT", // screen to display
		-1, // OK button text strref
		"Yes", // text to put in OK button
		-1, // Cancel button text strref
		"No"  // cancel button text
	);
	
	

		
		
}