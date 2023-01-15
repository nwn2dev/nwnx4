#pragma once

#include "../../controller/pch/pch.h"

#include <wx/fileconf.h>
#include <wx/frame.h>
#include <wx/image.h>
#include <wx/mimetype.h>
#include <wx/mstream.h>
#include <wx/statbmp.h>
#include <wx/thread.h>
#include <wx/wx.h>

// debug memory allocation enhancement (see next tip)
#ifdef _DEBUG
#include <crtdbg.h>
#define DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define DEBUG_NEW new
#endif