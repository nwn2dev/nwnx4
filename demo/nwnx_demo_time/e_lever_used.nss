// Name     : e_trig_enter
// Purpose  : Fires when the PC pulls the lever. 
// Authors  : Grinning Fool [Marc Paradise]
// Modified : December 6th, 2006
#include "nwnx_time"

void main()
{

	AssignCommand(OBJECT_SELF, ActionPlayAnimation(ANIMATION_PLACEABLE_ACTIVATE));
	AssignCommand(OBJECT_SELF, ActionPlayAnimation(ANIMATION_PLACEABLE_DEACTIVATE));
	
	object oPC = GetLastUsedBy();
	int id = GetLocalInt(OBJECT_SELF, "id");
	int state = GetLocalInt(OBJECT_SELF, "state");
	string name = "Timer" + IntToString(id);
	string time;
	if (state) { 
		time = StopTimer(OBJECT_SELF, name);
		SendMessageToPC(oPC, name + " has been stopped.");
		SendMessageToPC(oPC, "Time since you started " + name + ": " + time + " microseconds");
		SetFirstName(OBJECT_SELF, "Start " + name);
	} else { 
		SendMessageToPC(oPC, name + " has been started.");
		StartTimer(OBJECT_SELF, name);
		SetFirstName(OBJECT_SELF, "Stop " + name);
	}
	SetLocalInt(OBJECT_SELF, "state", !state);
	
	
}	