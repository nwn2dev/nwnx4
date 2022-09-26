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

#include "nwnx.h"
#include "DlgAbout.h"
#include "mainframe.h"

bool NWNXGUI::OnInit()
{
	m_locale.Init(wxLANGUAGE_DEFAULT, NULL);
	MainFrame* frame
	    = new MainFrame(nullptr, ID_MAINFRAME, SYMBOL_MAINFRAME_TITLE, SYMBOL_MAINFRAME_POSITION,
	                    SYMBOL_MAINFRAME_SIZE, SYMBOL_MAINFRAME_STYLE);

	frame->Show(true);
	SetTopWindow(frame);
	return TRUE;
}
