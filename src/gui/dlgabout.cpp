/***************************************************************************
    NWNX Dlg About - Simple "About" dialog
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
#include "dlgabout.h"
#include "res/nwnx4_logo.xpm"
#include "../nwnx_version.h"

#include "wx/statbmp.h"

AboutDialog::AboutDialog(
	wxWindow *parent,
	wxWindowID id,
	const wxString &title,
	const wxPoint &position,
	const wxSize& size,
	long style)
	: wxDialog(parent, id, title, position, size, style)
{
	wxString aboutText;

    aboutText.append(wxT("NWNX4 GUI Version " NWNX_VERSION_INFO "\n"));
    aboutText.append(wxT("Neverwinter Nights Extender for NWN2\n\n"));
    aboutText.append(wxT("Copyright (C) 2021 Crom, ihatemundays (Scott Munday)\n\n"));

	aboutText.append(wxT("Built (respectfully) from NWNX4 GUI Version 0.0.9\n"));
	aboutText.append(wxT("Neverwinter Nights Extender for NWN2\n\n"));
	aboutText.append(wxT("Copyright (C) 2008 Ingmar Stieger (Papillon)\n"));
	aboutText.append(wxT("Visit us at http://www.nwnx.org/\n\n"));
	aboutText.append(wxT("Contributors:\n\n"));
	aboutText.append(wxT("Virusman, Grinning Fool, McKillroy, Skywing,\n"));
	aboutText.append(wxT("and countless others: Tips, motivation, bugfixes, gripes... THANKS!"));

	wxBoxSizer *vsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hsizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticBitmap *logo = new wxStaticBitmap(this, wxID_ANY, wxIcon(nwnx4_logo_xpm));
	hsizer->Add(logo, 0, wxALL, 10);

	wxStaticText *text = new wxStaticText(this, wxID_ANY, aboutText);
	hsizer->Add(text, 0, wxALL, 10);

	vsizer->Add(hsizer);
	vsizer->Add(new wxButton(this, wxID_OK, wxT("OK")), 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 10);
  
	SetAutoLayout(TRUE);
	SetSizer(vsizer);

	vsizer->Fit(this);
	vsizer->SetSizeHints(this);
}  
 
BEGIN_EVENT_TABLE(AboutDialog, wxDialog)
	EVT_BUTTON( wxID_OK, AboutDialog::OnOk )
END_EVENT_TABLE()

void AboutDialog::OnOk(wxCommandEvent &event)
{
    event.Skip();
}
