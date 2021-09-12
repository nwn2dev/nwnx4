#include "nwnx_sql"
#include "nwnx_funcs"
#include "nwnx_time"

object oModule = GetModule();
location lStart = GetStartingLocation();
object oArea = GetAreaFromLocation(lStart);
object oContainer = CreateObject(OBJECT_TYPE_PLACEABLE, "plc_mc_lbox01", lStart);

void Shutdown(string sReason){
	WriteTimestampedLogEntry("!! Shutting down server because of " + sReason);
	NWNXSetString("SRVADMIN", "SHUTDOWNNWN2SERVER", "", 0, "");
}

void Assert(int bCond, string sFunction, int nLine, string sMessage=""){
	if(!bCond){
		WriteTimestampedLogEntry(sFunction + ":" + IntToString(nLine) + ": Assert failed" + (sMessage != "" ? ": " + sMessage : ""));
		if(!bIsShuttingDown){
			// Shut down the server
			bIsShuttingDown = TRUE;
			float fDelay = 6.0 - GetLocalFloat(oModule, "__timer");
			if(fDelay < 0.0)
				fDelay = 0.0;
			DelayCommand(fDelay, Shutdown("failed assert(s)"));
		}
	}
}
int ApproxEquals(float a, float b, float delta=0.01){
	return fabs(a - b) < delta;
}
//===================================================================





void xp_sql(){
	StartTimer(oModule, "nwnxtest_sqlperf");

	SQLCreateTables();

	// Simple var storage
	SetPersistentString(oModule, "nwnxtest_string", "Café");
	Assert(GetPersistentString(oModule, "nwnxtest_string") == "Café", __FUNCTION__, __LINE__);
	SetPersistentInt(oModule, "nwnxtest_int", 42);
	Assert(GetPersistentInt(oModule, "nwnxtest_int") == 42, __FUNCTION__, __LINE__);
	SetPersistentFloat(oModule, "nwnxtest_float", 13.37);
	Assert(GetPersistentFloat(oModule, "nwnxtest_float") == 13.37, __FUNCTION__, __LINE__);
	SetPersistentVector(oModule, "nwnxtest_vector", Vector(1.0, 2.0, 3.0));
	Assert(GetPersistentVector(oModule, "nwnxtest_vector") == Vector(1.0, 2.0, 3.0), __FUNCTION__, __LINE__);
	SetPersistentLocation(oModule, "nwnxtest_location", lStart);
	Assert(GetPersistentLocation(oModule, "nwnxtest_location") == lStart, __FUNCTION__, __LINE__);

	// SCORCO
	object oItem = CreateItemOnObject("nw_wswss001", oContainer);
	Assert(GetIsObjectValid(oItem), __FUNCTION__, __LINE__);
	SetPersistentObject(oModule, "nwnxtest_object", oItem);
	object oRetrieved = GetPersistentObject(oModule, "nwnxtest_object", oContainer);
	Assert(GetIsObjectValid(oRetrieved) && GetName(oRetrieved) == GetName(oItem), __FUNCTION__, __LINE__);
	DestroyObject(oItem);
	DestroyObject(oRetrieved);

	// Passthrough conventional campaign functions
	SetCampaignString("nwnxtest_cam", "string", "Café");
	Assert(GetCampaignString("nwnxtest_cam", "string") == "Café", __FUNCTION__, __LINE__);
	SetCampaignInt("nwnxtest_cam", "int", 42);
	Assert(GetCampaignInt("nwnxtest_cam", "int") == 42, __FUNCTION__, __LINE__);
	SetCampaignFloat("nwnxtest_cam", "float", 13.37);
	Assert(GetCampaignFloat("nwnxtest_cam", "float") == 13.37, __FUNCTION__, __LINE__);
	SetCampaignVector("nwnxtest_cam", "vector", Vector(1.0, 2.0, 3.0));
	Assert(GetCampaignVector("nwnxtest_cam", "vector") == Vector(1.0, 2.0, 3.0), __FUNCTION__, __LINE__);
	SetCampaignLocation("nwnxtest_cam", "location", lStart);
	Assert(GetCampaignLocation("nwnxtest_cam", "location") == lStart, __FUNCTION__, __LINE__);

	oItem = CreateItemOnObject("nw_wswss001", oContainer);
	Assert(GetIsObjectValid(oItem), __FUNCTION__, __LINE__);
	StoreCampaignObject("nwnxtest_cam", "object", oItem);
	oRetrieved = RetrieveCampaignObject("nwnxtest_cam", "object", GetLocation(oContainer), oContainer);
	Assert(GetIsObjectValid(oRetrieved) && GetName(oRetrieved) == GetName(oItem), __FUNCTION__, __LINE__);
	DestroyObject(oItem);
	DestroyObject(oRetrieved);

	WriteTimestampedLogEntry("[PERF] xp_mysql: " + FloatToString(StringToFloat(QueryTimer(oModule, "nwnxtest_sqlperf")) / 1000.0, 0, 2) + "ms");
}

void xp_time(){
	Assert(QueryTimer(oModule, "nwnxtest_timer") == "0", __FUNCTION__, __LINE__);
	StartTimer(oModule, "nwnxtest_timer");
	DelayCommand(1.0, Assert(ApproxEquals(StringToFloat(QueryTimer(oModule, "nwnxtest_timer")) / 1000000.0, 1.0, 0.1), __FUNCTION__, __LINE__));
	DelayCommand(2.0, Assert(ApproxEquals(StringToFloat(QueryTimer(oModule, "nwnxtest_timer")) / 1000000.0, 2.0, 0.1), __FUNCTION__, __LINE__));
	DelayCommand(3.0, Assert(ApproxEquals(StringToFloat(StopTimer(oModule, "nwnxtest_timer")) / 1000000.0, 3.0, 0.1), __FUNCTION__, __LINE__));
	DelayCommand(4.0, Assert(QueryTimer(oModule, "nwnxtest_timer") == "0", __FUNCTION__, __LINE__));
}

void xp_funcs(){
	object oCreature = CreateObject(OBJECT_TYPE_CREATURE, "c_bear", lStart);
	Assert(GetIsObjectValid(oCreature), __FUNCTION__, __LINE__);

	Assert(GetCreatureSoundSet(oCreature) == 7, __FUNCTION__, __LINE__);

	DestroyObject(oCreature);
}







//===================================================================
int bIsShuttingDown = FALSE;
void timer(){
	SetLocalFloat(oModule, "__timer", GetLocalFloat(oModule, "__timer") + 0.1);
	DelayCommand(0.1, timer());
}
void main()
{
	WriteTimestampedLogEntry("Execute " + __FILE__ + "================================");

	Assert(GetIsObjectValid(oContainer), __FUNCTION__, __LINE__);
	DelayCommand(0.1, timer());

	// Unit tests
	xp_sql();
	DelayCommand(1.0, xp_time());
	xp_funcs();

	WriteTimestampedLogEntry("Finished " + __FILE__ + "================================");
}