/***************************************************************************
    NWNX GUI - This class defines the wxWidgets application
    Copyright (C) 2007 Ingmar Stieger (Papillon, papillon@nwnx.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

***************************************************************************/

#include "stdwx.h"
#include "mainframe.h"
#include "nwnx.h"
#include "DlgAbout.h"

bool NWNXGUI::OnInit()
{
    USES_CONVERSION;

	// open ini file
	wxString nwn2Dir = this->argc > 1 ? this->argv[1] : "";
	wxString inifile(nwn2Dir + "\\nwnx.ini");
	auto path = A2T(inifile.c_str());
	wxFileConfig* config = new wxFileConfig(wxEmptyString, wxEmptyString,
		inifile, wxEmptyString, wxCONFIG_USE_RELATIVE_PATH|wxCONFIG_USE_NO_ESCAPE_CHARACTERS);

	// Setup temporary directories
	wxString tempPath;
	config->Read(wxT("nwn2temp"), &tempPath);
	if (tempPath != wxT(""))
	{
		SetEnvironmentVariable(wxT("TEMP"), A2T(tempPath.c_str()));
		SetEnvironmentVariable(wxT("TMP"), A2T(tempPath.c_str()));
	}

	m_locale.Init(wxLANGUAGE_DEFAULT, NULL);	
	MainFrame* frame = new MainFrame(NULL, ID_MAINFRAME, SYMBOL_MAINFRAME_TITLE, SYMBOL_MAINFRAME_POSITION, SYMBOL_MAINFRAME_SIZE, SYMBOL_MAINFRAME_STYLE, config);

	frame->Show(true);
	SetTopWindow(frame);
	return TRUE;	
}

DECLARE_APP(NWNXGUI)