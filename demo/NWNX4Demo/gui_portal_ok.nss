// Name     : gui_portal_ok
// Purpose  : INvoked by the GUI when the user pressed "yes" on prompt to proceed to the demo mod
// Authors  : Grinning Fool [Marc Paradise]
// Modified : December 16th, 2006

void warning()
{
	DelayCommand(3.0, ActionSpeakString("It appears that there has been no demo module created for this plugin."));
}
void main()
{
	object oPortal = GetLocalObject(OBJECT_SELF, "portal");
	string name = GetLocalString(oPortal, "mod");
	if (name == "") { 
		SendMessageToPC(OBJECT_SELF, "There is no plugin attached to this gateway.");
	} else { 
		StartNewModule(name);
	}
	// If the module doesn't work, say something!
	AssignCommand(oPortal, warning());
}