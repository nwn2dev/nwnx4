

void main()
{
	WriteTimestampedLogEntry("[DBG] exec gui_test_executescript");
	object oModule = GetModule();
	SetLocalString(oModule, "testexescript", "executed");
}