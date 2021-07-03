// Name     : e_creature_perceive
// Purpose  : Fires when the creature perceives the player. 
//
// Authors  : Grinning Fool [Marc Paradise]
// Modified : December 6th, 2006


void main()
{
	object oPC = GetLastPerceived();
	if (!GetIsObjectValid(oPC) || !GetIsPC(oPC)) { 
		return;
	}
	if (GetLocalInt(OBJECT_SELF, "DemoGreetComplete")) { 
		return; 
	}
	SetLocalInt(OBJECT_SELF, "DemoGreetComplete", TRUE);
	AssignCommand(OBJECT_SELF, ActionMoveToObject(oPC, FALSE, 1.2));
	AssignCommand(OBJECT_SELF, ActionStartConversation(oPC));
}