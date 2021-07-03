// Name     : e_creature_perceive
// Purpose  : Fires when the creature perceives the player. 
//
// Authors  : Grinning Fool [Marc Paradise]
// Modified : December 3rd, 2006
#include "nwnx_sql"

void main()
{
	object oPC = GetLastPerceived();
	if (!GetIsObjectValid(oPC) || !GetIsPC(oPC)) { 
		return;
	}
	if (GetPersistentInt(oPC, "DemoGreetComplete")) { 
		return; 
	}
	SetPersistentInt(oPC, "DemoGreetComplete", TRUE);
	AssignCommand(OBJECT_SELF, ActionMoveToObject(oPC, FALSE, 1.2));
	AssignCommand(OBJECT_SELF, ActionStartConversation(oPC));
}