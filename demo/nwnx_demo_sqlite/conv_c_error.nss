// Name     : conv_c_error
// Purpose  : conversation conditional checks for any error received on module load. 
//            If an error is received, it sets custom token 1000 and returns TRUE. 
// Authors  : Grinning Fool [Marc Paradise]
// Modified : December 3rd, 2006

int StartingConditional()
{
	string error = GetLocalString(GetModule(), "ERROR");
	if (GetStringLength(error) == 0) { 
		return FALSE;
	}
	SetCustomToken(1000, error);
	return TRUE;
}