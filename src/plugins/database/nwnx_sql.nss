// Name     : NWNX SQL include
// Version  : 1.10-dev49
// Purpose  : Scripting functions for NWNX SQL plugins
// Author   : Ingmar Stieger
// Author   : virusman
// Modified : 16 Nov 2008
// Copyright: This file is licensed under the terms of the
//            GNU GENERAL PUBLIC LICENSE (GPL) Version 2
//            Based on aps_include by Ingmar Stieger, Adam Colon, Josh Simon

/************************************/
/* Constants                        */
/************************************/

const int SQL_ERROR = 0;
const int SQL_SUCCESS = 1;

/************************************/
/* Function prototypes              */
/************************************/

// Create the SQL tables required for the SetPersistentXXX and
// GetPersistentXXX functions, if they don't already exist. You can call this
// function in the OnModuleLoad script.
void SQLCreateTables();

// Execute statement in sSQL
// returns: SQL_SUCCESS if the query was successful
//          SQL_ERROR if an error has occurred
int SQLExecDirect(string sSQL);

// Position cursor on next row of the resultset
// Call this before using SQLGetData().
// - Leave the parameter empty to advance to the next row.
// - Pass "NEXT" as parameter to fetch the first row of the
//   next resultset (for statements that return multiple resultsets)
// returns: SQL_SUCCESS if there is a row
//          SQL_ERROR if there are no more rows
int SQLFetch(string mode = " ");

// Return value of column iCol in the current row of result set sResultSetName
// Maximum column size: 65KByte
string SQLGetData(int iCol);

int SQLGetDataInt(int iCol){        return StringToInt(SQLGetData(iCol));   }
float SQLGetDataFloat(int iCol){    return StringToFloat(SQLGetData(iCol)); }

// Deprecated.
// Return value of column iCol in the current row of result set sResultSetName
// Maximum column size: 65KByte
// Only for compability reasons. Use SQLGetData instead.
string SQLGetDataText(int iCol);

// Return the number of rows that were affected by the last
// INSERT, UPDATE, or DELETE operation.
int SQLGetAffectedRows();

// Returns the ID of the last inserted or updated row.
// The table must have an AUTOINCREMENT column.
int SQLGetLastInsertedID();

// Return the error code for the most recently invoked API function that can succeed or fail.
// A return value of zero means that no error occurred.
// Client error message numbers are listed in nwnx_mysql_errors.nss
int SQLGetErrno();

// Return a string containing the error message for the most recently invoked API function that failed
string SQLGetErrorMessage();

// Return a string value when given a location
string SQLLocationToString(location lLocation);

// Return a location value when given the string form of the location
location SQLStringToLocation(string sLocation);

// Return a string value when given a vector
string SQLVectorToString(vector vVector);

// Return a vector value when given the string form of the vector
vector SQLStringToVector(string sVector);

// Set oObject's persistent string variable sVarName to sValue
// Optional parameters:
//   iExpiration: Number of days the persistent variable should be kept in database (default: 0=forever)
//   sTable: Name of the table where variable should be stored (default: pwdata)
void SetPersistentString(object oObject, string sVarName, string sValue, int iExpiration =
                         0, string sTable = "pwdata");

// Set oObject's persistent integer variable sVarName to iValue
// Optional parameters:
//   iExpiration: Number of days the persistent variable should be kept in database (default: 0=forever)
//   sTable: Name of the table where variable should be stored (default: pwdata)
void SetPersistentInt(object oObject, string sVarName, int iValue, int iExpiration =
                      0, string sTable = "pwdata");

// Set oObject's persistent float variable sVarName to fValue
// Optional parameters:
//   iExpiration: Number of days the persistent variable should be kept in database (default: 0=forever)
//   sTable: Name of the table where variable should be stored (default: pwdata)
void SetPersistentFloat(object oObject, string sVarName, float fValue, int iExpiration =
                        0, string sTable = "pwdata");

// Set oObject's persistent location variable sVarName to lLocation
// Optional parameters:
//   iExpiration: Number of days the persistent variable should be kept in database (default: 0=forever)
//   sTable: Name of the table where variable should be stored (default: pwdata)
//   This function converts location to a string for storage in the database.
void SetPersistentLocation(object oObject, string sVarName, location lLocation, int iExpiration =
                           0, string sTable = "pwdata");

// Set oObject's persistent vector variable sVarName to vVector
// Optional parameters:
//   iExpiration: Number of days the persistent variable should be kept in database (default: 0=forever)
//   sTable: Name of the table where variable should be stored (default: pwdata)
//   This function converts vector to a string for storage in the database.
void SetPersistentVector(object oObject, string sVarName, vector vVector, int iExpiration =
                         0, string sTable = "pwdata");

// Set oObject's persistent object with sVarName to sValue
// Optional parameters:
//   iExpiration: Number of days the persistent variable should be kept in database (default: 0=forever)
//   sTable: Name of the table where variable should be stored (default: pwobjdata)
void SetPersistentObject(object oObject, string sVarName, object oObject2, int iExpiration =
                         0, string sTable = "pwobjdata");

// Get oObject's persistent string variable sVarName
// Optional parameters:
//   sTable: Name of the table where variable is stored (default: pwdata)
// * Return value on error: ""
string GetPersistentString(object oObject, string sVarName, string sTable = "pwdata");

// Get oObject's persistent integer variable sVarName
// Optional parameters:
//   sTable: Name of the table where variable is stored (default: pwdata)
// * Return value on error: 0
int GetPersistentInt(object oObject, string sVarName, string sTable = "pwdata");

// Get oObject's persistent float variable sVarName
// Optional parameters:
//   sTable: Name of the table where variable is stored (default: pwdata)
// * Return value on error: 0
float GetPersistentFloat(object oObject, string sVarName, string sTable = "pwdata");

// Get oObject's persistent location variable sVarName
// Optional parameters:
//   sTable: Name of the table where variable is stored (default: pwdata)
// * Return value on error: 0
location GetPersistentLocation(object oObject, string sVarname, string sTable = "pwdata");

// Get oObject's persistent vector variable sVarName
// Optional parameters:
//   sTable: Name of the table where variable is stored (default: pwdata)
// * Return value on error: 0
vector GetPersistentVector(object oObject, string sVarName, string sTable = "pwdata");

// Get oObject's persistent object sVarName
// Optional parameters:
//   sTable: Name of the table where object is stored (default: pwobjdata)
// * Return value on error: 0
object GetPersistentObject(object oObject, string sVarName, object oOwner = OBJECT_INVALID, string sTable = "pwobjdata");

// Delete persistent variable sVarName stored on oObject
// Optional parameters:
//   sTable: Name of the table where variable is stored (default: pwdata)
void DeletePersistentVariable(object oObject, string sVarName, string sTable = "pwdata");

// Replace special characters (like ') in a database compatible way
string SQLEncodeSpecialChars(string sString);

// Execute statement in sSQL using SCO/RCO
void SQLSCORCOExec(string sSQL);

//Store object in DB using SCO hook
void SQLStoreObject(object oObject);

//Retrieve object from DB using RCO hook
object SQLRetrieveObject(location lLocation, object oOwner = OBJECT_INVALID, string sMode="-");

/************************************/
/* Implementation                   */
/************************************/

void SQLCreateTables(){
    string sSubClass = NWNXGetString("SQL", "GET SUBCLASS", "", 0);

    if(sSubClass == "MySQL"){
        SQLExecDirect("CREATE TABLE IF NOT EXISTS `pwdata` ("
            + "  `player` varchar(64) NOT NULL DEFAULT '~',"
            + "  `tag` varchar(64) NOT NULL DEFAULT '~',"
            + "  `name` varchar(64) NOT NULL DEFAULT '~',"
            + "  `val` text DEFAULT NULL,"
            + "  `expire` int(11) DEFAULT NULL,"
            + "  `last` timestamp NOT NULL DEFAULT current_timestamp(),"
            + "  PRIMARY KEY (`player`,`tag`,`name`)"
            + ") ENGINE=MyISAM DEFAULT CHARSET=utf8"
        );
        SQLExecDirect("CREATE TABLE IF NOT EXISTS `pwobjdata` ("
            + "  `player` varchar(64) NOT NULL DEFAULT '~',"
            + "  `tag` varchar(64) NOT NULL DEFAULT '~',"
            + "  `name` varchar(64) NOT NULL DEFAULT '~',"
            + "  `val` blob DEFAULT NULL,"
            + "  `expire` int(11) DEFAULT NULL,"
            + "  `last` timestamp NOT NULL DEFAULT current_timestamp(),"
            + "  PRIMARY KEY (`player`,`tag`,`name`)"
            + ") ENGINE=MyISAM DEFAULT CHARSET=utf8"
        );
    }
    else if(sSubClass == "SQLite"){
        SQLExecDirect("CREATE TABLE IF NOT EXISTS `pwdata` ("
            + "  `player` varchar(64) NOT NULL DEFAULT '~',"
            + "  `tag` varchar(64) NOT NULL DEFAULT '~',"
            + "  `name` varchar(64) NOT NULL DEFAULT '~',"
            + "  `val` text DEFAULT NULL,"
            + "  `expire` int(11) DEFAULT NULL,"
            + "  `last` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            + "  PRIMARY KEY (`player`,`tag`,`name`)"
            + ")"
        );
        SQLExecDirect("CREATE TABLE IF NOT EXISTS `pwobjdata` ("
            + "  `player` varchar(64) NOT NULL DEFAULT '~',"
            + "  `tag` varchar(64) NOT NULL DEFAULT '~',"
            + "  `name` varchar(64) NOT NULL DEFAULT '~',"
            + "  `val` blob DEFAULT NULL,"
            + "  `expire` int(11) DEFAULT NULL,"
            + "  `last` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            + "  PRIMARY KEY (`player`,`tag`,`name`)"
            + ")"
        );
    }
}

int SQLExecDirect(string sSQL)
{
    return NWNXGetInt("SQL", "EXEC", sSQL, 0);
}

int SQLFetch(string mode = " ")
{
    return NWNXGetInt("SQL", "FETCH", mode, 0);
}

string SQLGetData(int iCol)
{
    return NWNXGetString("SQL", "GETDATA", "", iCol - 1);
}

string SQLGetDataText(int iCol)
{
    return NWNXGetString("SQL", "GETDATA", "", iCol - 1);
}

int SQLGetAffectedRows()
{
    return NWNXGetInt("SQL", "GET AFFECTED ROWS", "", 0);
}

int SQLGetLastInsertedID()
{
    return NWNXGetInt("SQL", "GET INSID", "", 0);
}

int SQLGetErrno()
{
    return NWNXGetInt("SQL", "GET AFFECTED ROWS", "", 0);
}

string SQLGetErrorMessage()
{
    return NWNXGetString("SQL", "GET ERROR MESSAGE", "", 0);
}

void SQLSCORCOExec(string sSQL)
{
    NWNXSetString("SQL", "SETSCORCOSQL", sSQL, 0, "");
}

void SQLStoreObject(object oObject)
{
    StoreCampaignObject("NWNX", "-", oObject);
}

object SQLRetrieveObject(location lLocation, object oOwner = OBJECT_INVALID, string sMode="-")
{
    return RetrieveCampaignObject("NWNX", sMode, lLocation, oOwner);
}

// These functions deal with various data types. Ultimately, all NWN specific
// information must be stored in the database as strings, and converted back to
// the proper form when retrieved.

string SQLVectorToString(vector vVector)
{
    return "#POSITION_X#" + FloatToString(vVector.x) + "#POSITION_Y#" + FloatToString(vVector.y) +
        "#POSITION_Z#" + FloatToString(vVector.z) + "#END#";
}

vector SQLStringToVector(string sVector)
{
    float fX, fY, fZ;
    int iPos, iCount;
    int iLen = GetStringLength(sVector);

    if (iLen > 0)
    {
        iPos = FindSubString(sVector, "#POSITION_X#") + 12;
        iCount = FindSubString(GetSubString(sVector, iPos, iLen - iPos), "#");
        fX = StringToFloat(GetSubString(sVector, iPos, iCount));

        iPos = FindSubString(sVector, "#POSITION_Y#") + 12;
        iCount = FindSubString(GetSubString(sVector, iPos, iLen - iPos), "#");
        fY = StringToFloat(GetSubString(sVector, iPos, iCount));

        iPos = FindSubString(sVector, "#POSITION_Z#") + 12;
        iCount = FindSubString(GetSubString(sVector, iPos, iLen - iPos), "#");
        fZ = StringToFloat(GetSubString(sVector, iPos, iCount));
    }

    return Vector(fX, fY, fZ);
}

string SQLLocationToString(location lLocation)
{
    object oArea = GetAreaFromLocation(lLocation);
    vector vPosition = GetPositionFromLocation(lLocation);
    float fOrientation = GetFacingFromLocation(lLocation);
    string sReturnValue;

    if (GetIsObjectValid(oArea))
        sReturnValue =
            "#AREA#" + GetTag(oArea) + "#POSITION_X#" + FloatToString(vPosition.x) +
            "#POSITION_Y#" + FloatToString(vPosition.y) + "#POSITION_Z#" +
            FloatToString(vPosition.z) + "#ORIENTATION#" + FloatToString(fOrientation) + "#END#";

    return sReturnValue;
}

location SQLStringToLocation(string sLocation)
{
    location lReturnValue;
    object oArea;
    vector vPosition;
    float fOrientation, fX, fY, fZ;

    int iPos, iCount;
    int iLen = GetStringLength(sLocation);

    if (iLen > 0)
    {
        iPos = FindSubString(sLocation, "#AREA#") + 6;
        iCount = FindSubString(GetSubString(sLocation, iPos, iLen - iPos), "#");
        oArea = GetObjectByTag(GetSubString(sLocation, iPos, iCount));

        iPos = FindSubString(sLocation, "#POSITION_X#") + 12;
        iCount = FindSubString(GetSubString(sLocation, iPos, iLen - iPos), "#");
        fX = StringToFloat(GetSubString(sLocation, iPos, iCount));

        iPos = FindSubString(sLocation, "#POSITION_Y#") + 12;
        iCount = FindSubString(GetSubString(sLocation, iPos, iLen - iPos), "#");
        fY = StringToFloat(GetSubString(sLocation, iPos, iCount));

        iPos = FindSubString(sLocation, "#POSITION_Z#") + 12;
        iCount = FindSubString(GetSubString(sLocation, iPos, iLen - iPos), "#");
        fZ = StringToFloat(GetSubString(sLocation, iPos, iCount));

        vPosition = Vector(fX, fY, fZ);

        iPos = FindSubString(sLocation, "#ORIENTATION#") + 13;
        iCount = FindSubString(GetSubString(sLocation, iPos, iLen - iPos), "#");
        fOrientation = StringToFloat(GetSubString(sLocation, iPos, iCount));

        lReturnValue = Location(oArea, vPosition, fOrientation);
    }

    return lReturnValue;
}

// These functions are responsible for transporting the various data types back
// and forth to the database.

void SetPersistentString(object oObject, string sVarName, string sValue, int iExpiration =
                         0, string sTable = "pwdata")
{
    string sPlayer;
    string sTag;

    if (GetIsPC(oObject))
    {
        sPlayer = SQLEncodeSpecialChars(GetPCPlayerName(oObject));
        sTag = SQLEncodeSpecialChars(GetName(oObject));
    }
    else
    {
        sPlayer = "~";
        sTag = GetTag(oObject);
    }

    sVarName = SQLEncodeSpecialChars(sVarName);
    sValue = SQLEncodeSpecialChars(sValue);

    string sSQL = "SELECT player FROM " + sTable + " WHERE player='" + sPlayer +
        "' AND tag='" + sTag + "' AND name='" + sVarName + "'";
    SQLExecDirect(sSQL);

    if (SQLFetch() == SQL_SUCCESS)
    {
        // row exists
        sSQL = "UPDATE " + sTable + " SET val='" + sValue +
            "',expire=" + IntToString(iExpiration) + " WHERE player='" + sPlayer +
            "' AND tag='" + sTag + "' AND name='" + sVarName + "'";
        SQLExecDirect(sSQL);
    }
    else
    {
        // row doesn't exist
        sSQL = "INSERT INTO " + sTable + " (player,tag,name,val,expire) VALUES" +
            "('" + sPlayer + "','" + sTag + "','" + sVarName + "','" +
            sValue + "'," + IntToString(iExpiration) + ")";
        SQLExecDirect(sSQL);
    }
}

string GetPersistentString(object oObject, string sVarName, string sTable = "pwdata")
{
    string sPlayer;
    string sTag;

    if (GetIsPC(oObject))
    {
        sPlayer = SQLEncodeSpecialChars(GetPCPlayerName(oObject));
        sTag = SQLEncodeSpecialChars(GetName(oObject));
    }
    else
    {
        sPlayer = "~";
        sTag = GetTag(oObject);
    }

    sVarName = SQLEncodeSpecialChars(sVarName);

    string sSQL = "SELECT val FROM " + sTable + " WHERE player='" + sPlayer +
        "' AND tag='" + sTag + "' AND name='" + sVarName + "'";
    SQLExecDirect(sSQL);

    if (SQLFetch() == SQL_SUCCESS)
        return SQLGetData(1);
    else
    {
        return "";
        // If you want to convert your existing persistent data to SQL, this
        // would be the place to do it. The requested variable was not found
        // in the database, you should
        // 1) query it's value using your existing persistence functions
        // 2) save the value to the database using SetPersistentString()
        // 3) return the string value here.
    }
}

void SetPersistentInt(object oObject, string sVarName, int iValue, int iExpiration =
                      0, string sTable = "pwdata")
{
    SetPersistentString(oObject, sVarName, IntToString(iValue), iExpiration, sTable);
}

int GetPersistentInt(object oObject, string sVarName, string sTable = "pwdata")
{
    string sPlayer;
    string sTag;

    if (GetIsPC(oObject))
    {
        sPlayer = SQLEncodeSpecialChars(GetPCPlayerName(oObject));
        sTag = SQLEncodeSpecialChars(GetName(oObject));
    }
    else
    {
        sPlayer = "~";
        sTag = GetTag(oObject);
    }

    sVarName = SQLEncodeSpecialChars(sVarName);

    string sSQL = "SELECT val FROM " + sTable + " WHERE player='" + sPlayer +
        "' AND tag='" + sTag + "' AND name='" + sVarName + "'";
    SQLExecDirect(sSQL);

    if (SQLFetch() == SQL_SUCCESS)
        return StringToInt(SQLGetData(1));
    else
        return 0;
}

void SetPersistentFloat(object oObject, string sVarName, float fValue, int iExpiration =
                        0, string sTable = "pwdata")
{
    SetPersistentString(oObject, sVarName, FloatToString(fValue), iExpiration, sTable);
}

float GetPersistentFloat(object oObject, string sVarName, string sTable = "pwdata")
{
    string sPlayer;
    string sTag;

    if (GetIsPC(oObject))
    {
        sPlayer = SQLEncodeSpecialChars(GetPCPlayerName(oObject));
        sTag = SQLEncodeSpecialChars(GetName(oObject));
    }
    else
    {
        sPlayer = "~";
        sTag = GetTag(oObject);
    }

    sVarName = SQLEncodeSpecialChars(sVarName);

    string sSQL = "SELECT val FROM " + sTable + " WHERE player='" + sPlayer +
        "' AND tag='" + sTag + "' AND name='" + sVarName + "'";
    SQLExecDirect(sSQL);

    if (SQLFetch() == SQL_SUCCESS)
        return StringToFloat(SQLGetData(1));
    else
        return 0.0f;
}

void SetPersistentLocation(object oObject, string sVarName, location lLocation, int iExpiration =
                           0, string sTable = "pwdata")
{
    SetPersistentString(oObject, sVarName, SQLLocationToString(lLocation), iExpiration, sTable);
}

location GetPersistentLocation(object oObject, string sVarName, string sTable = "pwdata")
{
    return SQLStringToLocation(GetPersistentString(oObject, sVarName, sTable));
}

void SetPersistentVector(object oObject, string sVarName, vector vVector, int iExpiration =
                         0, string sTable = "pwdata")
{
    SetPersistentString(oObject, sVarName, SQLVectorToString(vVector), iExpiration, sTable);
}

vector GetPersistentVector(object oObject, string sVarName, string sTable = "pwdata")
{
    return SQLStringToVector(GetPersistentString(oObject, sVarName, sTable));
}

void SetPersistentObject(object oOwner, string sVarName, object oObject, int iExpiration =
                         0, string sTable = "pwobjdata")
{
    string sPlayer;
    string sTag;

    if (GetIsPC(oOwner))
    {
        sPlayer = SQLEncodeSpecialChars(GetPCPlayerName(oOwner));
        sTag = SQLEncodeSpecialChars(GetName(oOwner));
    }
    else
    {
        sPlayer = "~";
        sTag = GetTag(oOwner);
    }
    sVarName = SQLEncodeSpecialChars(sVarName);

    string sSQL = "SELECT player FROM " + sTable + " WHERE player='" + sPlayer +
        "' AND tag='" + sTag + "' AND name='" + sVarName + "'";
    SQLExecDirect(sSQL);

    if (SQLFetch() == SQL_SUCCESS)
    {
        // row exists
        sSQL = "UPDATE " + sTable + " SET val=%s,expire=" + IntToString(iExpiration) +
            " WHERE player='" + sPlayer + "' AND tag='" + sTag + "' AND name='" + sVarName + "'";
        SQLSCORCOExec(sSQL);
        SQLStoreObject (oObject);
    }
    else
    {
        // row doesn't exist
        sSQL = "INSERT INTO " + sTable + " (player,tag,name,val,expire) VALUES" +
            "('" + sPlayer + "','" + sTag + "','" + sVarName + "',%s," + IntToString(iExpiration) + ")";
        SQLSCORCOExec(sSQL);
        SQLStoreObject(oObject);
    }
}

object GetPersistentObject(object oObject, string sVarName, object oOwner = OBJECT_INVALID, string sTable = "pwobjdata")
{
    string sPlayer;
    string sTag;
    object oModule;

    if (GetIsPC(oObject))
    {
        sPlayer = SQLEncodeSpecialChars(GetPCPlayerName(oObject));
        sTag = SQLEncodeSpecialChars(GetName(oObject));
    }
    else
    {
        sPlayer = "~";
        sTag = GetTag(oObject);
    }
    sVarName = SQLEncodeSpecialChars(sVarName);

    string sSQL = "SELECT val FROM " + sTable + " WHERE player='" + sPlayer +
        "' AND tag='" + sTag + "' AND name='" + sVarName + "'";
    SQLSCORCOExec(sSQL);

    if (!GetIsObjectValid(oOwner))
        oOwner = oObject;
    return SQLRetrieveObject(GetLocation(oOwner), oOwner);
}

void DeletePersistentVariable(object oObject, string sVarName, string sTable = "pwdata")
{
    string sPlayer;
    string sTag;

    if (GetIsPC(oObject))
    {
        sPlayer = SQLEncodeSpecialChars(GetPCPlayerName(oObject));
        sTag = SQLEncodeSpecialChars(GetName(oObject));
    }
    else
    {
        sPlayer = "~";
        sTag = GetTag(oObject);
    }

    sVarName = SQLEncodeSpecialChars(sVarName);
    string sSQL = "DELETE FROM " + sTable + " WHERE player='" + sPlayer +
        "' AND tag='" + sTag + "' AND name='" + sVarName + "'";
    SQLExecDirect(sSQL);
}

// Problems can arise with SQL commands if variables or values have single quotes
// in their names. This function encodes these quotes so the underlying database
// can safely store them.
string SQLEncodeSpecialChars(string sString)
{
    return NWNXGetString("SQL", "GET ESCAPE STRING", sString, 0);
}
