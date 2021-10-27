/***************************************************************************
    NWNX Main frame - The main window of the GUI client
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

// Generated by DialogBlocks (unregistered), 17/12/2006 17:50:01

#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "mainframe.h"
#endif

/*!
 * Includes
 */

#include "../controller/controller.h"
#include "worker.h"
#include "GuiLog.h"
#include "../../nwnx_version.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_MAINFRAME 10000
#define ID_PANEL1 10001
#define ID_TEXTCTRL1 10002
#define ID_TEXTCTRL2 10003
#define ID_PW_ENABLED 10004
#define ID_TEXTCTRL3 10005
#define ID_TEXTCTRL4 10006
#define ID_GW_ENABLED 10007
#define ID_TEXTCTRL 10008
#define ID_TEXTCTRL6 10010
#define ID_TEXTCTRL5 10009
#define ID_TEXTCTRL7 10011
#define ID_TEXTCTRL_LOG 10017
#define ID_BTNSTART 10012
#define ID_BTNSTOP 10013
#define ID_BTNRESTART 10014
#define ID_HELP 10015
#define ID_FORUM 10016
#define SYMBOL_MAINFRAME_STYLE wxCAPTION|wxSYSTEM_MENU|wxMINIMIZE_BOX|wxCLOSE_BOX
#define SYMBOL_MAINFRAME_TITLE _("NWNX4 " NWNX_VERSION )
#define SYMBOL_MAINFRAME_IDNAME ID_MAINFRAME
#define SYMBOL_MAINFRAME_SIZE wxSize(400, 260)
#define SYMBOL_MAINFRAME_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * MainFrame class declaration
 */

class MainFrame: public wxFrame
{    
    DECLARE_CLASS( MainFrame )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    MainFrame(wxWindow* parent, wxWindowID id = SYMBOL_MAINFRAME_IDNAME, const wxString& caption = SYMBOL_MAINFRAME_TITLE, const wxPoint& pos = SYMBOL_MAINFRAME_POSITION, const wxSize& size = SYMBOL_MAINFRAME_SIZE, long style = SYMBOL_MAINFRAME_STYLE);
    ~MainFrame();

    bool Create(wxWindow* parent, wxWindowID id = SYMBOL_MAINFRAME_IDNAME, const wxString& caption = SYMBOL_MAINFRAME_TITLE, const wxPoint& pos = SYMBOL_MAINFRAME_POSITION, const wxSize& size = SYMBOL_MAINFRAME_SIZE, long style = SYMBOL_MAINFRAME_STYLE);

    /// Initialises member variables
    void Init();

    void OnClose(wxCloseEvent& event);

    /// Creates the controls and sizers
    void CreateControls();

////@begin MainFrame event handler declarations

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_PW_ENABLED
    void OnPwEnabledClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_GW_ENABLED
    void OnGwEnabledClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BTNSTART
    void OnBtnstartClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BTNSTOP
    void OnBtnstopClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BTNRESTART
    void OnBtnrestartClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EXIT
    void OnEXITClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_HELP
    void OnHelpClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_FORUM
    void OnForumClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_ABOUT
    void OnABOUTClick( wxCommandEvent& event );

////@end MainFrame event handler declarations

	void OnServerStarted(wxCommandEvent &event);
	void OnServerStopped(wxCommandEvent &event);
    void OnServerKilled(wxCommandEvent &event);

////@begin MainFrame member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end MainFrame member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin MainFrame member variables
    wxTextCtrl* m_startedAt;
    wxTextCtrl* m_CmdLine;
    wxCheckBox* m_PWEnabled;
    wxTextCtrl* m_PWInterval;
    wxTextCtrl* m_PWRestarts;
    wxCheckBox* m_GWEnabled;
    wxTextCtrl* m_GWInterval;
    wxTextCtrl* m_GWRetries;
    wxTextCtrl* m_GWRestarts;
    wxTextCtrl* m_GWLockups;
    wxTextCtrl* m_log;
    wxButton* m_BtnStart;
    wxButton* m_BtnStop;
    wxButton* m_BtnRestart;
////@end MainFrame member variables

private:
	NWNXController* controller;
    SimpleIniConfig* configuration;
    wxLogTextCtrl* m_logger;
	wxTimer m_timer;
	NWNXWorker* worker;
};

#endif
    // _MAINFRAME_H_
