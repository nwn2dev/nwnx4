#include "nwnx_include"
#include "nwnx_sql"
#include "nwnx_funcs"
#include "nwnx_time"

object oModule = GetModule();
location lStart = GetStartingLocation();
object oArea = GetAreaFromLocation(lStart);
object oContainer = CreateObject(OBJECT_TYPE_PLACEABLE, "plc_mc_lbox01", lStart);

void ShutdownIfFailed(){
	if(GetLocalInt(GetModule(), "shutting_down")){
		WriteTimestampedLogEntry("ERROR: Shutting down server because of failed asserts");
		NWNXSetString("SRVADMIN", "SHUTDOWNNWN2SERVER", "", 0, "");
	}
}

void Assert(int bCond, string sFunction, int nLine, string sMessage=""){
	if(!bCond){
		WriteTimestampedLogEntry("ERROR:" + sFunction + ":" + IntToString(nLine) + ": Assert failed" + (sMessage != "" ? ": " + sMessage : ""));
		SetLocalInt(GetModule(), "shutting_down", TRUE);
	}
}
void AssertEqS(string sA, string sB, string sFunction, int nLine, string sMessage=""){
	if(sA != sB){
		WriteTimestampedLogEntry("ERROR:" + sFunction + ":" + IntToString(nLine) + ": Assert failed: '" + sA + "' != '" + sB + "'" + (sMessage != "" ? ": " + sMessage : ""));
		SetLocalInt(GetModule(), "shutting_down", TRUE);
	}
}
void AssertEqI(int nA, int nB, string sFunction, int nLine, string sMessage=""){
	if(nA != nB){
		WriteTimestampedLogEntry("ERROR:" + sFunction + ":" + IntToString(nLine) + ": Assert failed: " + IntToString(nA) + " != " + IntToString(nB) + (sMessage != "" ? ": " + sMessage : ""));
		SetLocalInt(GetModule(), "shutting_down", TRUE);
	}
}
void AssertEqF(float fA, float fB, float fPrecision, string sFunction, int nLine, string sMessage=""){
	if(fabs(fA - fB) > fPrecision){
		WriteTimestampedLogEntry("ERROR:" + sFunction + ":" + IntToString(nLine) + ": Assert failed: " + FloatToString(fA, 0) + " != " + FloatToString(fB, 0) + " (precision: " + FloatToString(fPrecision, 0) + ")" + (sMessage != "" ? ": " + sMessage : ""));
		SetLocalInt(GetModule(), "shutting_down", TRUE);
	}
}
//===================================================================


void xp_sql_bench(){

	SQLExecDirect("DROP TABLE IF EXISTS `benchtable`");
	SQLExecDirect(
		  "CREATE TABLE IF NOT EXISTS `benchtable` ("
	    + "  `key` VARCHAR(64) NOT NULL DEFAULT '~',"
	    + "  `valuestr` VARCHAR(64),"
	    + "  `valueint` INT,"
	    + "  `valuefloat` FLOAT,"
	    + "  PRIMARY KEY (`key`)"
	    + ") ENGINE=MyISAM DEFAULT CHARSET=utf8"
	);

	int nLoops = 200;

	int insertStmt = SQLPrepPrepareStatement("INSERT INTO `benchtable` (`key`,`valuestr`,`valueint`,`valuefloat`) VALUES (?,?,?,?)");
	int selectStmt = SQLPrepPrepareStatement("SELECT `key`,`valuestr`,`valueint`,`valuefloat` FROM `benchtable`");

	// ------------------------------------------
	StartTimer(oModule, "nwnxtest_sqlbench");

	int i;
	for(i = 0 ; i < nLoops ; i++){
		SQLPrepBindString(insertStmt, 1, "Key" + IntToString(i));
		SQLPrepBindString(insertStmt, 2, "Value=" + IntToString(i));
		SQLPrepBindInt(insertStmt, 3, i * i);
		SQLPrepBindFloat(insertStmt, 4, sqrt(1f * i));
		AssertEqI(SQLPrepExecute(insertStmt), TRUE, __FUNCTION__, __LINE__);
	}

	WriteTimestampedLogEntry("[SQLBenchPrep]   " + NWNXGetPluginSubClass("SQL") + ": inserts=" + FloatToString(StringToFloat(QueryTimer(oModule, "nwnxtest_sqlbench")) / (1.0 * nLoops), 0, 2) + "µs / iter");
	StopTimer(oModule, "nwnxtest_sqlbench");
	StartTimer(oModule, "nwnxtest_sqlbench");

	SQLPrepExecute(selectStmt);
	for(i = 0 ; i < nLoops ; i++){
		AssertEqI(SQLPrepFetch(selectStmt), 1, __FUNCTION__, __LINE__);

		AssertEqS(SQLPrepGetString(selectStmt, 1), "Key" + IntToString(i), __FUNCTION__, __LINE__);
		AssertEqS(SQLPrepGetString(selectStmt, 2), "Value=" + IntToString(i), __FUNCTION__, __LINE__);
		AssertEqI(SQLPrepGetInt(selectStmt, 3), i * i, __FUNCTION__, __LINE__);
		AssertEqF(SQLPrepGetFloat(selectStmt, 4), sqrt(1f * i), 0.001, __FUNCTION__, __LINE__);
	}

	WriteTimestampedLogEntry("[SQLBenchPrep]   " + NWNXGetPluginSubClass("SQL") + ": selects=" + FloatToString(StringToFloat(QueryTimer(oModule, "nwnxtest_sqlbench")) / (1.0 * nLoops), 0, 2) + "µs / iter");

	StopTimer(oModule, "nwnxtest_sqlbench");

	// ------------------------------------------
	StartTimer(oModule, "nwnxtest_sqlbench");

	for(i = 0 ; i < nLoops ; i++){
		AssertEqI(
			SQLExecDirect("INSERT INTO `benchtable` (`key`,`valuestr`,`valueint`,`valuefloat`) VALUES ('"
				+ SQLEncodeSpecialChars("Key" + IntToString(i)) + "','"
				+ SQLEncodeSpecialChars("Value=" + IntToString(i)) + "',"
				+ IntToString(i * i) + ","
				+ FloatToString(sqrt(1f * i), 0) + ")"
			),
			TRUE, __FUNCTION__, __LINE__
		);
	}

	WriteTimestampedLogEntry("[SQLBenchDirect] " + NWNXGetPluginSubClass("SQL") + ": inserts=" + FloatToString(StringToFloat(QueryTimer(oModule, "nwnxtest_sqlbench")) / (1.0 * nLoops), 0, 2) + "µs / iter");
	StopTimer(oModule, "nwnxtest_sqlbench");
	StartTimer(oModule, "nwnxtest_sqlbench");

	SQLExecDirect("SELECT `key`,`valuestr`,`valueint`,`valuefloat` FROM `benchtable`");
	for(i = 0 ; i < nLoops ; i++){
		AssertEqI(SQLFetch(), 1, __FUNCTION__, __LINE__);

		AssertEqS(SQLGetData(1), "Key" + IntToString(i), __FUNCTION__, __LINE__);
		AssertEqS(SQLGetData(2), "Value=" + IntToString(i), __FUNCTION__, __LINE__);
		AssertEqI(SQLGetDataInt(3), i * i, __FUNCTION__, __LINE__);
		AssertEqF(SQLGetDataFloat(4), sqrt(1f * i), 0.001, __FUNCTION__, __LINE__);
	}

	WriteTimestampedLogEntry("[SQLBenchDirect] " + NWNXGetPluginSubClass("SQL") + ": selects=" + FloatToString(StringToFloat(QueryTimer(oModule, "nwnxtest_sqlbench")) / (1.0 * nLoops), 0, 2) + "µs / iter");
	// ------------------------------------------
}

void SQLDisconnect()
{
	// Advanced hidden function
    NWNXGetInt("SQL", "DISCON", "", 0);
}

void xp_sql(){
	StartTimer(oModule, "nwnxtest_sqlperf");

	SQLExecDirect("DROP TABLE IF EXISTS pwdata");
	SQLExecDirect("DROP TABLE IF EXISTS pwobjdata");
	SQLCreateTables();

	// Simple var storage
	SetPersistentString(oModule, "nwnxtest_string", "Café");
	AssertEqS(GetPersistentString(oModule, "nwnxtest_string"), "Café", __FUNCTION__, __LINE__);
	SetPersistentInt(oModule, "nwnxtest_int", 42);
	AssertEqI(GetPersistentInt(oModule, "nwnxtest_int"), 42, __FUNCTION__, __LINE__);
	SetPersistentFloat(oModule, "nwnxtest_float", 13.37);
	AssertEqF(GetPersistentFloat(oModule, "nwnxtest_float"), 13.37, 0.0, __FUNCTION__, __LINE__);
	SetPersistentVector(oModule, "nwnxtest_vector", Vector(1.0, 2.0, 3.0));
	Assert(GetPersistentVector(oModule, "nwnxtest_vector") == Vector(1.0, 2.0, 3.0), __FUNCTION__, __LINE__);
	SetPersistentLocation(oModule, "nwnxtest_location", lStart);
	Assert(GetPersistentLocation(oModule, "nwnxtest_location") == lStart, __FUNCTION__, __LINE__);

	// Test server disconnection
	SQLDisconnect();
	AssertEqS(GetPersistentString(oModule, "nwnxtest_string"), "Café", __FUNCTION__, __LINE__);
	AssertEqI(GetPersistentInt(oModule, "nwnxtest_int"), 42, __FUNCTION__, __LINE__);
	AssertEqF(GetPersistentFloat(oModule, "nwnxtest_float"), 13.37, 0.0, __FUNCTION__, __LINE__);
	Assert(GetPersistentVector(oModule, "nwnxtest_vector") == Vector(1.0, 2.0, 3.0), __FUNCTION__, __LINE__);
	Assert(GetPersistentLocation(oModule, "nwnxtest_location") == lStart, __FUNCTION__, __LINE__);

	// SCORCO
	object oItem1 = CreateItemOnObject("nw_wswss001", oContainer);
	Assert(GetIsObjectValid(oItem1), __FUNCTION__, __LINE__);
	SetPersistentObject(oModule, "nwnxtest_object1", oItem1);

	object oItem2 = CreateItemOnObject("nw_it_gem001", oContainer);
	Assert(GetIsObjectValid(oItem2), __FUNCTION__, __LINE__);
	SetPersistentObject(oModule, "nwnxtest_object2", oItem2);

	object oRetrieved1 = GetPersistentObject(oModule, "nwnxtest_object1", oContainer);
	Assert(GetIsObjectValid(oRetrieved1), __FUNCTION__, __LINE__);
	AssertEqS(GetResRef(oRetrieved1), "nw_wswss001", __FUNCTION__, __LINE__);
	DestroyObject(oItem1);
	DestroyObject(oRetrieved1);

	object oRetrieved2 = GetPersistentObject(oModule, "nwnxtest_object2", oContainer);
	Assert(GetIsObjectValid(oRetrieved2), __FUNCTION__, __LINE__);
	AssertEqS(GetResRef(oRetrieved2), "nw_it_gem001", __FUNCTION__, __LINE__);
	DestroyObject(oItem2);
	DestroyObject(oRetrieved2);

	// Fetch loop
	int nCount = 0;
	if(GetLocalInt(GetModule(), "NWNXSQL_USEPREP")){
		int nSelectAllStmt = SQLPrepPrepareStatement("SELECT `player`, `expire` FROM pwdata WHERE player=?");
		SQLPrepBindString(nSelectAllStmt, 1, "~");
		SQLPrepExecute(nSelectAllStmt);
		while(SQLPrepFetch(nSelectAllStmt)){
			AssertEqS(SQLPrepGetString(nSelectAllStmt, 1), "~", __FUNCTION__, __LINE__);
			nCount++;
		}

		// Prepared statement without parameters, with sql conversion to string
		int test = SQLPrepPrepareStatement("SELECT `name`, `val` FROM pwdata");
		SQLPrepExecute(test);

		int i = 0;
		while(SQLPrepFetch(test)){
			string sName = SQLPrepGetString(test, 1);
			switch(i++){
				case 0:
					AssertEqS(SQLPrepGetString(test, 1), "nwnxtest_string", __FUNCTION__, __LINE__);
					AssertEqS(SQLPrepGetString(test, 2), "Café", __FUNCTION__, __LINE__);
					break;
				case 1:
					AssertEqS(SQLPrepGetString(test, 1), "nwnxtest_int", __FUNCTION__, __LINE__);
					AssertEqS(SQLPrepGetString(test, 2), "42", __FUNCTION__, __LINE__);
					break;
				case 2:
					AssertEqS(SQLPrepGetString(test, 1), "nwnxtest_float", __FUNCTION__, __LINE__);
					AssertEqF(StringToFloat(SQLPrepGetString(test, 2)), 13.37, 0.0001, __FUNCTION__, __LINE__);
					break;
				case 3:
					{
						AssertEqS(SQLPrepGetString(test, 1), "nwnxtest_vector", __FUNCTION__, __LINE__);
						vector vValue = SQLStringToVector(SQLPrepGetString(test, 2));
						AssertEqF(vValue.x, 1.0, 0.0001, __FUNCTION__, __LINE__);
						AssertEqF(vValue.y, 2.0, 0.0001, __FUNCTION__, __LINE__);
						AssertEqF(vValue.z, 3.0, 0.0001, __FUNCTION__, __LINE__);
					}
					break;
				case 4:
					{
						AssertEqS(SQLPrepGetString(test, 1), "nwnxtest_location", __FUNCTION__, __LINE__);
						location lValue = SQLStringToLocation(SQLPrepGetString(test, 2));

						object oArea = GetAreaFromLocation(lValue);
						Assert(oArea == GetFirstArea(), __FUNCTION__, __LINE__);

						vector vPos = GetPositionFromLocation(lValue);
						AssertEqF(vPos.x, 18.0, 0.0001, __FUNCTION__, __LINE__);
						AssertEqF(vPos.y, 18.0, 0.0001, __FUNCTION__, __LINE__);
						AssertEqF(vPos.z, 0.0, 0.0001, __FUNCTION__, __LINE__);

						float fFacing = GetFacingFromLocation(lValue);
						AssertEqF(fFacing, 270.0, 0.0001, __FUNCTION__, __LINE__);
					}
					break;
				default:
					Assert(FALSE, __FUNCTION__, __LINE__, "unexpected " + SQLPrepGetString(test, 1));
			}
			// WriteTimestampedLogEntry("    name=" + SQLPrepGetString(test, 1) + " val=" + SQLPrepGetString(test, 2));
		}
	}
	else{
		SQLExecDirect("SELECT `player`, `expire` FROM pwdata WHERE player='~'");
		while(SQLFetch()){
			Assert(SQLGetData(1) == "~", __FUNCTION__, __LINE__);
			nCount++;
		}

		int i = 0;
		while(SQLFetch()){
			string sName = SQLGetData(1);
			switch(i++){
				case 0:
					AssertEqS(SQLGetData(1), "nwnxtest_string", __FUNCTION__, __LINE__);
					AssertEqS(SQLGetData(2), "Café", __FUNCTION__, __LINE__);
					break;
				case 1:
					AssertEqS(SQLGetData(1), "nwnxtest_int", __FUNCTION__, __LINE__);
					AssertEqS(SQLGetData(2), "42", __FUNCTION__, __LINE__);
					break;
				case 2:
					AssertEqS(SQLGetData(1), "nwnxtest_float", __FUNCTION__, __LINE__);
					AssertEqF(StringToFloat(SQLGetData(2)), 13.37, 0.0001, __FUNCTION__, __LINE__);
					break;
				case 3:
					{
						AssertEqS(SQLGetData(1), "nwnxtest_vector", __FUNCTION__, __LINE__);
						vector vValue = SQLStringToVector(SQLGetData(2));
						AssertEqF(vValue.x, 1.0, 0.0001, __FUNCTION__, __LINE__);
						AssertEqF(vValue.y, 2.0, 0.0001, __FUNCTION__, __LINE__);
						AssertEqF(vValue.z, 3.0, 0.0001, __FUNCTION__, __LINE__);
					}
					break;
				case 4:
					{
						AssertEqS(SQLGetData(1), "nwnxtest_location", __FUNCTION__, __LINE__);
						location lValue = SQLStringToLocation(SQLGetData(2));

						object oArea = GetAreaFromLocation(lValue);
						Assert(oArea == GetFirstArea(), __FUNCTION__, __LINE__);

						vector vPos = GetPositionFromLocation(lValue);
						AssertEqF(vPos.x, 18.0, 0.0001, __FUNCTION__, __LINE__);
						AssertEqF(vPos.y, 18.0, 0.0001, __FUNCTION__, __LINE__);
						AssertEqF(vPos.z, 0.0, 0.0001, __FUNCTION__, __LINE__);

						float fFacing = GetFacingFromLocation(lValue);
						AssertEqF(fFacing, 270.0, 0.0001, __FUNCTION__, __LINE__);
					}
					break;
				default:
					Assert(FALSE, __FUNCTION__, __LINE__, "unexpected " + SQLGetData(1));
			}
			// WriteTimestampedLogEntry("    name=" + SQLGetData(1) + " val=" + SQLGetData(test, 2));
		}
	}
	Assert(nCount == 5, __FUNCTION__, __LINE__);

	// Passthrough conventional campaign functions
	SetCampaignString("nwnxtest_cam", "string", "Café");
	AssertEqS(GetCampaignString("nwnxtest_cam", "string"), "Café", __FUNCTION__, __LINE__);
	SetCampaignInt("nwnxtest_cam", "int", 42);
	AssertEqI(GetCampaignInt("nwnxtest_cam", "int"), 42, __FUNCTION__, __LINE__);
	SetCampaignFloat("nwnxtest_cam", "float", 13.37);
	AssertEqF(GetCampaignFloat("nwnxtest_cam", "float"), 13.37, 0.0, __FUNCTION__, __LINE__);
	SetCampaignVector("nwnxtest_cam", "vector", Vector(1.0, 2.0, 3.0));
	Assert(GetCampaignVector("nwnxtest_cam", "vector") == Vector(1.0, 2.0, 3.0), __FUNCTION__, __LINE__);
	SetCampaignLocation("nwnxtest_cam", "location", lStart);
	Assert(GetCampaignLocation("nwnxtest_cam", "location") == lStart, __FUNCTION__, __LINE__);

	object oItem = CreateItemOnObject("nw_wswss001", oContainer);
	Assert(GetIsObjectValid(oItem), __FUNCTION__, __LINE__);
	StoreCampaignObject("nwnxtest_cam", "object", oItem);
	object oRetrieved = RetrieveCampaignObject("nwnxtest_cam", "object", GetLocation(oContainer), oContainer);
	Assert(GetIsObjectValid(oRetrieved), __FUNCTION__, __LINE__);
	AssertEqS(GetName(oRetrieved), GetName(oItem), __FUNCTION__, __LINE__);
	DestroyObject(oItem);
	DestroyObject(oRetrieved);

	WriteTimestampedLogEntry("[PERF] " + NWNXGetPluginSubClass("SQL") + ": " + FloatToString(StringToFloat(QueryTimer(oModule, "nwnxtest_sqlperf")) / 1000.0, 0, 2) + "ms");

	if(NWNXGetPluginSubClass("SQL") == "MySQL"){
		DelayCommand(2.2, xp_sql_bench());
		DelayCommand(3.2, xp_sql_bench());
	}
}

void xp_time(){
	int nPluginIndex = NWNXFindPluginByClass("TIME");
	AssertEqS(NWNXGetPluginSubClass("TIME"), "TIME", __FUNCTION__, __LINE__);

	AssertEqS(NWNXGetPluginInfo(nPluginIndex), NWNXGetPluginDescription("TIME"), __FUNCTION__, __LINE__);
	AssertEqS(NWNXGetPluginSemVer(nPluginIndex), "0.0.2", __FUNCTION__, __LINE__);
	AssertEqS(NWNXGetPluginSemVer(nPluginIndex), NWNXGetPluginVersion("TIME"), __FUNCTION__, __LINE__);

	AssertEqS(QueryTimer(oModule, "nwnxtest_timer"), "0", __FUNCTION__, __LINE__);
	StartTimer(oModule, "nwnxtest_timer");
	DelayCommand(1.0, AssertEqF(StringToFloat(QueryTimer(oModule, "nwnxtest_timer")) / 1000000.0, 1.0, 0.1, __FUNCTION__, __LINE__));
	DelayCommand(1.5, AssertEqF(StringToFloat(QueryTimer(oModule, "nwnxtest_timer")) / 1000000.0, 1.5, 0.1, __FUNCTION__, __LINE__));
	DelayCommand(2.0, AssertEqF(StringToFloat(StopTimer(oModule, "nwnxtest_timer")) / 1000000.0, 2.0, 0.1, __FUNCTION__, __LINE__));
	DelayCommand(2.5, AssertEqS(QueryTimer(oModule, "nwnxtest_timer"), "0", __FUNCTION__, __LINE__));
}

void xp_funcs(){
	object oCreature = CreateObject(OBJECT_TYPE_CREATURE, "c_bear", lStart);
	Assert(GetIsObjectValid(oCreature), __FUNCTION__, __LINE__);

	AssertEqI(GetCreatureSoundSet(oCreature), 7, __FUNCTION__, __LINE__);

	DestroyObject(oCreature);
}

void xp_example_cplugin(){
	int nCount = NWNXGetPluginCount();
	Assert(nCount > 0, __FUNCTION__, __LINE__);

	int nPlugin = NWNXFindPluginByClass("CPluginExample");
	AssertEqS(NWNXGetPluginClass(nPlugin), "CPluginExample", __FUNCTION__, __LINE__);
	struct NWNXVersion version = NWNXExtractVersionNumbers(NWNXGetPluginSemVer(nPlugin));
	AssertEqI(version.major, 1, __FUNCTION__, __LINE__);
	AssertEqI(version.minor, 0, __FUNCTION__, __LINE__);
	AssertEqI(version.patch, 0, __FUNCTION__, __LINE__);

	AssertEqS(NWNXGetPluginInfo(nPlugin), "NWNX4 CPlugin Example - A C++ plugin showcasing C plugin ABI", __FUNCTION__, __LINE__);

	AssertEqI(NWNXGetInt("CPluginExample", "GET_COUNTER", "", 0), 0, __FUNCTION__, __LINE__);
	NWNXSetInt("CPluginExample", "SET_COUNTER", "", 0, 42);
	AssertEqI(NWNXGetInt("CPluginExample", "GET_COUNTER", "", 0), 42, __FUNCTION__, __LINE__);
	AssertEqI(NWNXGetInt("CPluginExample", "INC_COUNTER", "", 0), 43, __FUNCTION__, __LINE__);
	AssertEqI(NWNXGetInt("CPluginExample", "INC_COUNTER", "", 0), 44, __FUNCTION__, __LINE__);

	AssertEqF(NWNXGetFloat("CPluginExample", "", "", 0), 0.0, 0.0, __FUNCTION__, __LINE__);
	NWNXSetFloat("CPluginExample", "", "", 0, 13.37);
	AssertEqF(NWNXGetFloat("CPluginExample", "", "", 0), 13.37, 0.0, __FUNCTION__, __LINE__);

	AssertEqS(NWNXGetString("CPluginExample", "", "", 0), "", __FUNCTION__, __LINE__);
	NWNXSetString("CPluginExample", "", "", 0, "Hello world");
	AssertEqS(NWNXGetString("CPluginExample", "", "", 0), "Hello world", __FUNCTION__, __LINE__);

	object oItem1 = CreateItemOnObject("nw_wswss001", oContainer);
	StoreCampaignObject("NWNX.CPluginExample", "A", oItem1);
	object oItem2 = CreateItemOnObject("nw_it_gem001", oContainer);
	StoreCampaignObject("NWNX.CPluginExample", "A", oItem2);
	object oItem3 = CreateItemOnObject("nw_it_gem002", oContainer);
	Assert(GetIsObjectValid(oItem3), __FUNCTION__, __LINE__);
	StoreCampaignObject("NWNX.CPluginExample", "B", oItem3);

	object oRetrieved = RetrieveCampaignObject("NWNX.CPluginExample", "A", GetLocation(oContainer), oContainer);
	Assert(GetIsObjectValid(oRetrieved), __FUNCTION__, __LINE__);
	AssertEqS(GetResRef(oRetrieved), "nw_wswss001", __FUNCTION__, __LINE__);
	oRetrieved = RetrieveCampaignObject("NWNX.CPluginExample", "A", GetLocation(oContainer), oContainer);
	Assert(GetIsObjectValid(oRetrieved), __FUNCTION__, __LINE__);
	AssertEqS(GetResRef(oRetrieved), "nw_it_gem001", __FUNCTION__, __LINE__);
	oRetrieved = RetrieveCampaignObject("NWNX.CPluginExample", "B", GetLocation(oContainer), oContainer);
	Assert(GetIsObjectValid(oRetrieved), __FUNCTION__, __LINE__);
	AssertEqS(GetResRef(oRetrieved), "nw_it_gem002", __FUNCTION__, __LINE__);

	Assert(!GetIsObjectValid(RetrieveCampaignObject("NWNX.CPluginExample", "A", GetLocation(oContainer), oContainer)), __FUNCTION__, __LINE__);
	Assert(!GetIsObjectValid(RetrieveCampaignObject("NWNX.CPluginExample", "B", GetLocation(oContainer), oContainer)), __FUNCTION__, __LINE__);
}

void nwnx_general(){
	int nCount = NWNXGetPluginCount();
	Assert(nCount > 0, __FUNCTION__, __LINE__);
	WriteTimestampedLogEntry("[INFO] Plugin count: " + IntToString(nCount));

	int i;
	for(i = 0 ; i < nCount ; i++)
	{
		string sClass = NWNXGetPluginClass(i);
		Assert(NWNXFindPluginByClass(sClass) == i, __FUNCTION__, __LINE__, "plugin index " + IntToString(i));

		if(sClass == "SQL"){
			// This is not a CPlugin, and it implements get version and get description
			Assert(NWNXGetPluginSubClass("SQL") == "MySQL" || NWNXGetPluginSubClass("SQL") == "SQLite", __FUNCTION__, __LINE__);
			AssertEqS(NWNXGetPluginInfo(i), NWNXGetPluginDescription(sClass), __FUNCTION__, __LINE__);
			Assert(NWNXGetPluginInfo(i) != "", __FUNCTION__, __LINE__);
			AssertEqS(NWNXGetPluginInfo(i), NWNXGetPluginDescription(sClass), __FUNCTION__, __LINE__);
			Assert(NWNXGetPluginSemVer(i) != "", __FUNCTION__, __LINE__);
			AssertEqS(NWNXGetPluginSemVer(i), NWNXGetPluginVersion(sClass), __FUNCTION__, __LINE__);
		}
	}

	Assert(NWNXGetPluginClass(i) == "", __FUNCTION__, __LINE__);
	Assert(NWNXGetPluginInfo(i) == "", __FUNCTION__, __LINE__);
}


void legacy_compatibility(){
	// xp_ini
	int INI_FLAG_NONE = 0;
	int INI_FLAG_OPEN_FILE = 1;
	int INI_FLAG_SAVE_FILE = 2;
	int INI_FLAG_CLOSE_FILE = 3;
	int INI_FLAG_CREATE_FILE = 4;
	int INI_FLAG_DELETE_FILE = 5;
	int INI_FLAG_FILE_OPENED = 6;
	int INI_FLAG_GET_UNICODE = 7;
	int INI_FLAG_SET_UNICODE = 8;
	int INI_FLAG_GET_MULTIKEY = 9;
	int INI_FLAG_SET_MULTIKEY = 10;
	int INI_FLAG_GET_MULTILINE = 11;
	int INI_FLAG_SET_MULTILINE = 12;
	int INI_FLAG_GET_USESPACES = 13;
	int INI_FLAG_SET_USESPACES = 14;
	int INI_FLAG_GET_PATH = 15;
	int INI_FLAG_FILE_EMPTY = 16;
	int INI_FLAG_INVALID = 17;


	int nIniIndex = NWNXFindPluginByClass("INI");
	Assert(nIniIndex >= 0, __FILE__, __LINE__);
	// These two are not correctly implemented in the plugin, but it shouldn't segfault the nwn2server
	AssertEqS(NWNXGetPluginInfo(nIniIndex), "", __FILE__, __LINE__);
	AssertEqS(NWNXGetPluginSemVer(nIniIndex), "", __FILE__, __LINE__);

	Assert(NWNXGetInt("INI", "test.ini", "test.ini", INI_FLAG_OPEN_FILE), __FILE__, __LINE__);

	AssertEqI(NWNXGetInt("INI", "test.ini", "main|IntValue", INI_FLAG_NONE), 42, __FILE__, __LINE__);
	AssertEqF(NWNXGetFloat("INI", "test.ini", "main|FloatValue", INI_FLAG_NONE), 13.37, 0.0001, __FILE__, __LINE__);
	AssertEqS(NWNXGetString("INI", "test.ini", "main|StringValue", INI_FLAG_NONE), "hello world", __FILE__, __LINE__);

	NWNXSetInt("INI", "test.ini", "main|IntValue", INI_FLAG_NONE, 43);
	NWNXSetFloat("INI", "test.ini", "main|FloatValue", INI_FLAG_NONE, 24.48);
	NWNXSetString("INI", "test.ini", "main|StringValue", INI_FLAG_NONE, "Wow it worked :)");

	AssertEqI(NWNXGetInt("INI", "test.ini", "main|IntValue", INI_FLAG_NONE), 43, __FILE__, __LINE__);
	AssertEqF(NWNXGetFloat("INI", "test.ini", "main|FloatValue", INI_FLAG_NONE), 24.48, 0.0001, __FILE__, __LINE__);
	AssertEqS(NWNXGetString("INI", "test.ini", "main|StringValue", INI_FLAG_NONE), "Wow it worked :)", __FILE__, __LINE__);

	Assert(NWNXGetInt("INI", "test.ini", "", INI_FLAG_CLOSE_FILE), __FILE__, __LINE__);
}



void main()
{
	DelayCommand(6.0, ShutdownIfFailed());

	WriteTimestampedLogEntry("Execute " + __FILE__ + "================================");

	Assert(GetIsObjectValid(oContainer), __FUNCTION__, __LINE__);

	// Unit tests
	nwnx_general();
	if(NWNXFindPluginByClass("INI") >= 0)
		legacy_compatibility();
	else
		WriteTimestampedLogEntry("WARN: legacy_compatibility checks have been skipped. Load xp_ini plugin to run compatibility tests");
	xp_sql();
	DelayCommand(1.0, xp_time());
	xp_funcs();
	xp_example_cplugin();

	WriteTimestampedLogEntry("Finished " + __FILE__ + "================================");
}
