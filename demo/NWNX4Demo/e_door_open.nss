// Name     : door_return
// Purpose  : on used script that prompts to ensure that the user wants to return to the NWNX4Demo.mod
// Authors  : Grinning Fool [Marc Paradise]
// Modified : December 3rd, 2006

void main()
{

	DisplayMessageBox(
	 	GetLastOpenedBy(), // User to show message to
		 -1, // message strref
		"Are you sure you wish to return to NWNX4Demo?", // message tex
		"gui_door_return_ok",  // script to invok on Yes processed
		"", // Cancel script callback
		TRUE, // show cancel button
		"SCREEN_MESSAGEBOX_DEFAULT", // screen to display
		-1, // OK button text strref
		"Yes", // text to put in OK button
		-1, // Cancel button text strref
		"No"  // cancel button text
	);
	
	

		
		
}