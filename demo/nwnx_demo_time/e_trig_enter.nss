// Name     : e_trig_enter
// Purpose  : Fires when the PC enters the trigger. 
//
// Authors  : Grinning Fool [Marc Paradise]
// Modified : December 6th, 2006

const string DONE = "dn"; 
const string TIMER1 = "t1";
const string TIMER2 = "t2";

#include "nwnx_time"
void main()
{
	int id = GetLocalInt(OBJECT_SELF, "id"); 
	object oPC = GetEnteringObject();
	// TODO: Any way to query if a timer is valid? What happens if a QueryTimer is called on a stopped timer? 
	if (!GetIsObjectValid(oPC) || !GetIsPC(oPC)) 
		return;
	string time; 
	string time2; 
	int bTimer1 = GetLocalInt(oPC, TIMER1);
	int bTimer2 = GetLocalInt(oPC, TIMER2); 
	int bDone   = GetLocalInt(oPC, DONE);
	// TODO: If I want to get tricky, determine where they're going by getting facing.. 
	switch (id) { 
		case 1: 
			if (bDone) { 
				DeleteLocalInt(oPC, TIMER2); 
				DeleteLocalInt(oPC, TIMER1); 
				DeleteLocalInt(oPC, DONE); 
				SendMessageToPC(oPC, "Timers have been reset.  Go catch your breath, then come back through these markers to start again.");
			} else if (bTimer1) { 
				SendMessageToPC(oPC, "You're running in circles! Go to the next markers!");
			} else {
				StartTimer(oPC, TIMER1);
				SetLocalInt(oPC, TIMER1, TRUE);
				SendMessageToPC(oPC, "The timer is now running! Go go go!");
			}
			break;
		case 2: 
			if (bDone) {
				return; 
			}
			
			if (bTimer1) { 
				if (bTimer2) { 
					SendMessageToPC(oPC, "You're running in circles! Go to the next markers!");
				} else { 
					time = QueryTimer(oPC, TIMER1); 
					StartTimer(oPC, TIMER2);
					SetLocalInt(oPC, TIMER2, TRUE);
					SendMessageToPC(oPC, "Your time since the first marker is " + time + " microseconds");
				}
			} else { 
				SendMessageToPC(oPC, "You didn't pass through the first marker! Go back and do it now!");
			}
			break;
			
		case 3: 
			if (bDone) {
				SendMessageToPC(oPC, "What are you still doing around here?  You've finished already!");
				return;
			}
			if (bTimer1) { 
				if (bTimer2) { 
					time = StopTimer(oPC, TIMER1);
					time2 = StopTimer(oPC, TIMER2);
					SetLocalInt(oPC, DONE, TRUE);
					SendMessageToPC(oPC, "Nice one! Your total time since the first marker is " + time + " microseconds, and your time since the second marker is " + time2 + " microseconds.  Go back through the first markers if you want to reset and try again. ");
				} else { 
					SendMessageToPC(oPC, "Oops! You missed the second markers -- hurry up and get to them!");
				}
			} else { 
				SendMessageToPC(oPC, "What are you doing here?! You haven't even passed through the first markers yet!"); 
			}
	}
	
}