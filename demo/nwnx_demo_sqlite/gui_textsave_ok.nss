// Name     : gui_textsave_ok
// Purpose  : INvoked by the GUI when the user presses OK after entering their text to save
// Authors  : Grinning Fool [Marc Paradise]
// Modified : December 5rd, 2006

#include "nwnx_sql"

void main(string value)
{
	if (value == "") { 
		SendMessageToPC(OBJECT_SELF, "No text being saved -- you didn't type anything!");
		return;
	}
	SendMessageToPC(OBJECT_SELF, "Saving text to database: " + value);
	SetPersistentString(OBJECT_SELF, "demoName", value);

}