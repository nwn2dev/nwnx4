

int StartingConditional(string sString, float fFloat, int nInt, object oObject)
{
	WriteTimestampedLogEntry("[DBG] exec gui_test_executescriptenh");
	WriteTimestampedLogEntry("[DBG] testexescriptenh_sString=" + sString);
	WriteTimestampedLogEntry("[DBG] testexescriptenh_fFloat=" + FloatToString(fFloat));
	WriteTimestampedLogEntry("[DBG] testexescriptenh_nInt=" + IntToString(nInt));
	WriteTimestampedLogEntry("[DBG] testexescriptenh_oObject=" + ObjectToString(oObject));
	object oModule = GetModule();
	SetLocalString(oModule, "testexescriptenh_string", sString);
	SetLocalFloat(oModule, "testexescriptenh_float", fFloat);
	SetLocalInt(oModule, "testexescriptenh_int", nInt);
	SetLocalObject(oModule, "testexescriptenh_object", oObject);

	int nRet = GetLocalInt(oModule, "testexescriptenh_cnt") + 10;
	SetLocalInt(oModule, "testexescriptenh_cnt", nRet);
	return nRet;
}