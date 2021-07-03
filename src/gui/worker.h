/***************************************************************************
    NWNX GUI Worker - A thread that periodically pings the controller class
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

#include "../controller/controller.h"

#define ACTION_NONE 0
#define ACTION_START 10000
#define ACTION_STOP 10001
#define ACTION_RESTART 10002
#define ACTION_KILL 10003

DECLARE_EVENT_TYPE(wxEVT_SERVER_STARTED, -1)
DECLARE_EVENT_TYPE(wxEVT_SERVER_STOPPED, -1)
DECLARE_EVENT_TYPE(wxEVT_SERVER_KILLED, -1)

class NWNXWorker : public wxThread
{
  public:
	NWNXWorker(NWNXController* controller, wxFrame* mainFrame);
	~NWNXWorker();
	virtual void *Entry();
	void startServer();
	void stopServer();
	void restartServer();
	void killServer();

  private:
	void resetAction();

	NWNXController* m_controller;
	wxFrame* m_mainFrame;
	wxMutex* m_mutex;
	int m_action;
};