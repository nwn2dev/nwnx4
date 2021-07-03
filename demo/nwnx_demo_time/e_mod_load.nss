// Name     : e_mod_load
// Purpose  : This script just validates that the TIME plugin is properly loaded. 
// Authors  : Grinning Fool [Marc Paradise]
// Modified : December 6th, 2006
#include "nwnx_include"
#include "nwnx_time"

void main()
{
	object oMod = GetModule();
	DeleteLocalString(oMod, "ERROR");
	if (!NWNXInstalled()) { 
		SetLocalString(oMod, "ERROR", "NWNX4 has not been installed.  Before proceeding with mysql installation, you'll need to download and install NWNX4.");
		PrintString("No NWNX4");
		return;
	}
	if (GetStringUpperCase(NWNXGetPluginSubClass("TIME")) != "TIME") { 
		SetLocalString(oMod, "ERROR", "The TIME plugin is not available.  Please ensure that xp_time.dll is present; if not, please copy it there and restart the NWN2 server.");
		PrintString("e_mod_load: Wrong Plugin Subclass");
		return;
	}
	
	// restore timer state if the user is going back and forth between mods. 
	// TODO: Restore the trigger timers too? 	
	// update: ah, well. Doesn't look like the timers are preserved between module reloads.  
	object timer2 = GetObjectByTag("timer2"); 
	object timer1 = GetObjectByTag("timer1");
	// Yeah, I know this should be a function but I'm feeling lazy... 
	if (QueryTimer(timer1, "Timer1") != "0") { 
		SetFirstName(timer1, "Stop Timer1");
		SetLocalInt(timer1, "state", TRUE);
	}
	if (QueryTimer(timer2, "Timer2") != "0") { 
		SetFirstName(timer2, "Stop Timer2");
		SetLocalInt(timer2, "state", TRUE);
	}
	
	
}