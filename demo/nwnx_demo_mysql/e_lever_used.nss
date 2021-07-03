

#include "nwnx_sql"

const int SAVE_VAR = 1;
const int GET_VAR = 2;
const int SAVE_OBJ = 3;
const int GET_OBJ = 4;

void main()
{
	object oPC = GetLastUsedBy();
	AssignCommand(OBJECT_SELF, ActionPlayAnimation(ANIMATION_PLACEABLE_ACTIVATE));
	AssignCommand(OBJECT_SELF, ActionPlayAnimation(ANIMATION_PLACEABLE_DEACTIVATE));
	int task = GetLocalInt(OBJECT_SELF, "action"); 
	int state = GetLocalInt(oPC, "state");
	string value = "";
	switch (task) {
		case SAVE_VAR:
			DisplayInputBox(oPC, -1, "Enter some text.  We'll save this into the database.  Even when you restart the server, it will still be saved!", "gui_textsave_ok", "", FALSE);
			break;
			
		case GET_VAR: {
			string data = GetPersistentString(oPC, "demoName");
			if (data == "") {
				data = "-- No text saved yet --";
			}
			
			DisplayMessageBox(oPC, -1, "Here's the text you saved to the database earlier: \n" + data);
			break;
		}						
		case SAVE_OBJ: {
			SendMessageToPC(oPC, "This function is not yet implemented under NWNX4.");
			/*
			int iItem;
			object oChest = GetObjectByTag("Chest1");
			object oItem = GetFirstItemInInventory(oChest);
			while (GetIsObjectValid(oItem)) { 
				SetPersistentObject(oChest, "Item_" + IntToString(iItem), oItem);
				oItem = GetNextItemInInventory(oChest);
				iItem++;
			}
			DisplayMessageBox(oPC, -1, "We have stored " + IntToString(iItem) + " items in the database.  These will still be saved even if you restart the server.");
			*/
			break;	
		}
			
		case GET_OBJ: {
			SendMessageToPC(oPC, "This function is not yet implemented under NWNX4.");
			/*
		    int iItem;
		    object oCreated;
		    object oChest1 = GetObjectByTag("Chest1");
		    object oChest2 = GetObjectByTag("Chest2");
			do { 
		        oCreated = GetPersistentObject(oChest1, "Item_" + IntToString(iItem), oChest2);
	            iItem++;
			} while (GetIsObjectValid(oCreated);
			
			DisplayMessageBox(oPC, -1, "We have restored " + IntToString(iItem - 1) + " objects from the database.  These will still be saved even if you restart the server.");
			*/
			break;
		}
		default: 
			SendMessageToPC(oPC, "Unknown action: " + IntToString(task));
	}
	
	
}	