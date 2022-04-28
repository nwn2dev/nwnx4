// Name     : NWNX include
// Purpose  : Functions for querying NWNX status and installed plugins
// Author   : Ingmar Stieger
// Modified : 04/29/2007
// Copyright: This file is licensed under the terms of the
//            GNU GENERAL PUBLIC LICENSE (GPL) Version 2
/**/

// Returns TRUE if NWNX is installed
int NWNXInstalled()
{
    return NWNXGetInt("NWNX", "INSTALLED", "", 0);
}

// Returns number of registered plugins / function classes
int NWNXGetPluginCount()
{
    return NWNXGetInt("NWNX", "GET PLUGIN COUNT", "", 0);    
}

// Returns the index of the plugin with the class name sPluginClassName
int NWNXFindPluginByClass(string sPluginClassName)
{
    return NWNXGetInt("NWNX", "FIND PLUGIN CLASS", sPluginClassName, 0);
}

// Returns function class specified by parameter nPlugin
string NWNXGetPluginClass(int nPlugin)
{
    return NWNXGetString("NWNX", "GET PLUGIN CLASS", "", nPlugin);
}

// Returns the plugin user-readable information.
string NWNXGetPluginInfo(int nPlugin)
{
    return NWNXGetString("NWNX", "GET PLUGIN INFO", "", nPlugin);
}

// Returns the plugin version as a string
string NWNXGetPluginSemVer(int nPlugin)
{
    return NWNXGetString("NWNX", "GET PLUGIN VERSION", "", nPlugin);
}


struct NWNXVersion {
    int major;
    int minor;
    int patch;
};

// Converts a version string like "v1.2.3" into a struct containing version int numbers
struct NWNXVersion NWNXExtractVersionNumbers(string sVersion)
{
    int nStart = 0;
    if(GetStringLowerCase(GetStringLeft(sVersion, 1)) == "v")
        nStart = 1;

    struct NWNXVersion version;

    int nEnd = FindSubString(sVersion, ".", nStart);
    version.major = StringToInt(GetSubString(sVersion, nStart, nEnd - nStart));

    if(nEnd < 0)
        return version;

    nStart = nEnd + 1;
    nEnd = FindSubString(sVersion, ".", nStart);
    version.minor = StringToInt(GetSubString(sVersion, nStart, nEnd - nStart));

    if(nEnd < 0)
        return version;

    nStart = nEnd + 1;
    version.patch = StringToInt(GetSubString(sVersion, nStart, -1));

    return version;
}



string NWNXGetPluginSubClass(string sClass)
{
    return NWNXGetString(sClass, "GET SUBCLASS", "", 0);
}

/// Deprecated function, only works with legacy plugins. Use NWNXGetPluginInfo instead
string NWNXGetPluginDescription(string sClass)
{
    return NWNXGetString(sClass, "GET DESCRIPTION", "", 0);
}

// Deprecated function, only works with legacy plugins. Use NWNXGetPluginSemVer instead
string NWNXGetPluginVersion(string sClass)
{
    return NWNXGetString(sClass, "GET VERSION", "", 0);
}