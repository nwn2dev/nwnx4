/***************************************************************************
    Patch - Runtime patch wrapper classes
    Copyright (C) 2000 Andreas Hansson (adron@valhallalegends.com)

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

#include <windows.h>

#include "Patch.h"

Patch::~Patch()
{
	if (orig)
		delete[] orig;
	if (repl)
		delete[] repl;
}

Patch::Patch(DWORD offset, char* replace, int l, Relocation* nreloc)
{
	offs = reinterpret_cast<char*>(offset);
	repl = new char[l];
	memcpy(repl, replace, l);
	reloc     = nreloc;
	relocdata = false;
	enabled   = false;
	orig      = 0;
	len       = l;
}

Patch::Patch(DWORD offset, relativefunc func, Relocation* nreloc)
{
	offs          = reinterpret_cast<char*>(offset);
	repl          = new char[4];
	*(DWORD*)repl = reinterpret_cast<DWORD>(func) - offset - 4;
	reloc         = nreloc;
	relocdata     = false;
	enabled       = false;
	orig          = 0;
	len           = 4;
}

Patch::Patch(DWORD offset, absolutefunc func, Relocation* nreloc)
{
	offs          = reinterpret_cast<char*>(offset);
	repl          = new char[4];
	*(DWORD*)repl = reinterpret_cast<DWORD>(func);
	reloc         = nreloc;
	relocdata     = false;
	enabled       = false;
	orig          = 0;
	len           = 4;
}

Patch::Patch()
{
	offs = repl = orig = 0;
	reloc              = 0;
	len                = 0;
	enabled            = false;
}

int Patch::Apply(void)
{
	if (enabled)
		return 0;
	if (!offs)
		return 0;
	DWORD oldprot;
	char* trueoffs = offs;
	if (reloc) {
		trueoffs += *reloc;
		if (relocdata)
			*(DWORD*)repl -= *reloc;
	}
	VirtualProtect(trueoffs, len, PAGE_EXECUTE_READWRITE, &oldprot);
	if (!orig) {
		orig = new char[len];
		memcpy(orig, trueoffs, len);
	}
	memcpy(trueoffs, repl, len);
	VirtualProtect(trueoffs, len, oldprot, &oldprot);
	enabled = true;
	return 1;
}

int Patch::Remove(void)
{
	if (!enabled)
		return 0;
	if (!offs)
		return 0;
	DWORD oldprot;
	char* trueoffs = offs;
	if (reloc) {
		trueoffs += *reloc;
		if (relocdata)
			*(DWORD*)repl += *reloc;
	}
	VirtualProtect(trueoffs, len, PAGE_EXECUTE_READWRITE, &oldprot);
	if (orig) {
		memcpy(trueoffs, orig, len);
	}
	VirtualProtect(trueoffs, len, oldprot, &oldprot);
	enabled = false;
	return 1;
}

void Relocation::Relocate()
{
	HINSTANCE hLib = LoadLibraryA(dll);
	if (hLib != 0) {
		base = reinterpret_cast<DWORD>(GetProcAddress(hLib, func)) - offs;
		FreeLibrary(hLib);
	} else {
		base = 0x10000000;
	}
}
