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

#include "stdwx.h"
#include "worker.h"

NWNXWorker::NWNXWorker(NWNXController* controller, wxFrame* mainFrame)
{
	m_controller = controller;
	m_mainFrame = mainFrame;
	m_action = ACTION_NONE;
	m_mutex = new wxMutex();
}

NWNXWorker::~NWNXWorker()
{
}

void *NWNXWorker::Entry()
{
	int i = 0;
	// wxLogTrace(TRACE_VERBOSE, wxT("Worker thread started."));

	while (!TestDestroy())
	{
		Sleep(250);

		switch(m_action)
		{
			case ACTION_START:
				wxLogMessage(wxT("* Starting the NWN Server."));
				m_controller->startServerProcess();
				wxPostEvent(m_mainFrame->GetEventHandler(), wxCommandEvent(wxEVT_SERVER_STARTED, m_mainFrame->GetId()));
				resetAction();
				break;

			case ACTION_STOP:
				wxLogMessage(wxT("* Stopping the NWN Server."));
				m_controller->killServerProcess(true);
				wxPostEvent(m_mainFrame->GetEventHandler(), wxCommandEvent(wxEVT_SERVER_STOPPED, m_mainFrame->GetId()));
				resetAction();
				break;

			case ACTION_RESTART:
				wxLogMessage(wxT("* Restarting the NWN Server."));
				m_controller->restartServerProcess();
				wxPostEvent(m_mainFrame->GetEventHandler(), wxCommandEvent(wxEVT_SERVER_STARTED, m_mainFrame->GetId()));
				resetAction();
				break;

		    case ACTION_KILL:
                wxLogMessage(wxT("* Killing the NWN Server."));
                m_controller->killServerProcess();
                wxPostEvent(m_mainFrame->GetEventHandler(), wxCommandEvent(wxEVT_SERVER_KILLED, m_mainFrame->GetId()));
                resetAction();
                break;
		}

		if (i % 4 == 0)
		{
			m_controller->ping();
			i = 0;
		}
		else
		{
			i++;
		}

	}

	return nullptr;
}

void NWNXWorker::startServer()
{
	wxMutexLocker lock(*m_mutex);
	m_action = ACTION_START;
}

void NWNXWorker::stopServer()
{
	wxMutexLocker lock(*m_mutex);
	m_action = ACTION_STOP;
}

void NWNXWorker::restartServer()
{
	wxMutexLocker lock(*m_mutex);
	m_action = ACTION_RESTART;
}

void NWNXWorker::killServer()
{
    if (m_action == ACTION_NONE) {
        wxMutexLocker lock(*m_mutex);
        m_action = ACTION_KILL;
    } else {
        m_controller->killServerProcess();
        wxPostEvent(m_mainFrame->GetEventHandler(), wxCommandEvent(wxEVT_SERVER_KILLED, m_mainFrame->GetId()));
    }
}

void NWNXWorker::resetAction()
{
	wxMutexLocker lock(*m_mutex);
	m_action = ACTION_NONE;
}
