// Name     : e_mod_load
// Purpose  : This script will attempt to automatically set up the database when the module first loads. 
//
// Authors  : Grinning Fool [Marc Paradise]
// Modified : December 3rd, 2006
#include "nwnx_sql"
#include "nwnx_include"
int doesTableExist(string tablename)
{
	SQLExecDirect("SELECT count(*) FROM " + tablename);
	if (SQLFetch() != SQL_SUCCESS) 
		return FALSE;	
	string val = SQLGetData(1);
	if (val == "0" || StringToInt(val) > 0) 
		return TRUE; 
	return FALSE;

}
void main()
{
	object oMod = GetModule();
	DeleteLocalString(oMod, "ERROR");
	if (!NWNXInstalled()) { 
		SetLocalString(oMod, "ERROR", "NWNX4 has not been installed.  Before proceeding with SQLite installation, you'll need to download and install NWNX4.");
		PrintString("No NWNX4");
		return;
	}
	if (GetStringUpperCase(NWNXGetPluginSubClass("SQL")) != "SQLITE") { 
		SetLocalString(oMod, "ERROR", "The SQLite plugin is not available.  If you have both xp_mysql.dll and xp_sqlite.dll in your NWN2 folder, please remove xp_mysql.dll. \n\nIf you don't have xp_sqlite.dll in your NWN2 install directory, please copy it there and reload NWNX4Demo.");
		PrintString("e_mod_load: Wrong Plugin Subclass");
		return;
	}
	/*
	SQLExecDirect("SHOW DATABASES");
	if (SQLFetch() != SQL_SUCCESS) { 
		SetLocalString(oMod, "ERROR", "The SQLite plugin is not available.  ");
		PrintString("e_mod_load: SHOW DATABASES failed.");
		return;
	}
	*/
	if (!doesTableExist("pwdata")) { 
	    SQLExecDirect("CREATE TABLE pwdata (" +
	        "player varchar(64) NOT NULL default '~'," +
	        "tag varchar(64) NOT NULL default '~'," +
	        "name varchar(64) NOT NULL default '~'," +
	        "val text," +
	        "expire int(11) default NULL," +
	        "last timestamp NOT NULL default current_timestamp," +
	        "PRIMARY KEY (player,tag,name)" +
	        ")"
		);
		if (!doesTableExist("pwdata")) { 
			SetLocalString(oMod, "ERROR", "The SQLite plugin could not set up the table 'pwdata'.  Please make sure that all settings in xp_sqlite.ini are correct.");	
			PrintString("e_mod_load: CREATE pwdata failed.");
			return;
		}
		
	}
	if (!doesTableExist("pwobjdata")) { 
	    SQLExecDirect("CREATE TABLE pwobjdata (" +
	        "player varchar(64) NOT NULL default '~'," +
	        "tag varchar(64) NOT NULL default '~'," +
	        "name varchar(64) NOT NULL default '~'," +
	        "val blob," +
	        "expire int(11) default NULL," +
	        "last timestamp NOT NULL default current_timestamp," +
	        "PRIMARY KEY (player,tag,name)" +
	        ")"
		);
		if (!doesTableExist("pwobjdata")) { 
			SetLocalString(oMod, "ERROR", "The SQLite plugin could not set up the table 'pwobjdata'.  Please make sure that the mysql userid in xp_mysql.ini has sufficient permissions to create a new table.");	
			PrintString("e_mod_load: CREATE pwobjdata failed.");
			return;
		}
	}
}