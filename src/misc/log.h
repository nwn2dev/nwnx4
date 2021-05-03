/***************************************************************************
    NWNX Log - Logging functions
    Copyright (C) 2006 Ingmar Stieger (Papillon, papillon@nwnx.org)

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

#if !defined(LOG_H_INCLUDED)
#define LOG_H_INCLUDED

#include "wx/log.h"
#include <atlbase.h>
#include <atlconv.h>

const wxString TRACE_NORMAL = wxT("TRACE_NORMAL");
const wxString TRACE_VERBOSE = wxT("TRACE_VERBOSE");

class wxLogNWNX : public wxLog
{
public:
    // redirect log output to a FILE
	wxLogNWNX(FILE *fp = (FILE *) NULL);
	wxLogNWNX(FILE *fp, wxString);
	wxLogNWNX(wxString);
	wxLogNWNX(wxString, wxString);
	wxLogNWNX(wxString, wxString, long);

protected:
    // implement sink function
    virtual void DoLogText(const wxString &msg);

	FILE* fFile;
	wxString fName;
	long maxSizeBytes;
	wxString header;

	void create_log_file();
	void set_trace_mask();

    DECLARE_NO_COPY_CLASS(wxLogNWNX)
};

/*
// debug examples :
wxLogError(wxT("wxLogError"));
wxLogWarning(wxT("wxLogWarning"));
wxLogMessage(wxT("wxLogMessage"));
wxLogVerbose(wxT("wxLogVerbose"));
wxLogStatus(wxT("wxLogStatus"));
wxLogSysError(wxT("wxLogSysError"));
wxLogDebug(wxT("wxLogDebug"));
wxLogFatalError(wxT("wxLogFatalError")); 
*/

#endif
