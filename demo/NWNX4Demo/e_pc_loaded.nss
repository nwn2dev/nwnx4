#include "nwnx_include"

// Creates the portal and sets it up for the specified plugin
void createPortal(object oPC, object oTarget, int nPluginId)
{
	object oPortal = GetNearestObjectByTag("module_portal", oTarget, nPluginId);
	if (oPortal == OBJECT_INVALID)  // We ran out of portals.
		return;	
	location l = GetLocation(oPortal);
    object o = GetAreaFromLocation(l);
    vector v = GetPositionFromLocation(l);
    float f = GetFacingFromLocation(l);	
	v.z = 1.92; // set to appropriate starting height to fit inside the portal placeable.
	location dest = Location(o, v, f);
	string class = NWNXGetPluginClass(nPluginId);
	string subclass = NWNXGetPluginSubClass(class);
	string version = NWNXGetPluginVersion(class);
	string description = 
		"Class: " + class + "\n" + 
		"Subclass: " + subclass + "\n" + 
		"Version: " + version + "\n\n" + 
		NWNXGetPluginDescription(class);
		
		
	SetFirstName(oPortal, subclass);
	SetLastName(oPortal, "");
	SetDescription(oPortal, description);
	SetLocalString(oPortal, "mod", "nwnx_demo_" + GetStringLowerCase(subclass));
	
	// Create the portal place effect at specified location... 
	object visual = CreateObject(OBJECT_TYPE_PLACED_EFFECT, "nwnx_portal", dest, FALSE);
	if (!GetIsObjectValid(visual)) { 
		SendMessageToPC(oPC, "Could not create portal effect for " + class + " - " + subclass);
	}
}

void main()
{
	object oPC = GetEnteringObject();
    if (!NWNXInstalled()) { 
		SendMessageToPC(oPC, "NWNX is not installed.  This module will not work.");
		return;
	}	
 	int count = NWNXGetPluginCount();
    if (count == 0) { 
		SendMessageToPC(oPC, "There are no plugins installed. This module will not work.");
		return;
	}
	// All portals will be created relative to WP_START.  Using this 
	// instead of oPC, because it's a fixed point, whereas oPC might move 
	// before this script executes.
	object oTarget = GetNearestObjectByTag("WP_START", oPC); 
	int i;
	for (i = 1; i <= count; i++) { 
		createPortal(oPC, oTarget, i);
	}
				
}