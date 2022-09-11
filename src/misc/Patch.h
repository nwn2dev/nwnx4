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

#ifndef _PATCH_H
#define _PATCH_H

// DLL relocation class
class Relocation {
	char* dll;
	char* func;
	DWORD base;
	DWORD offs;
	Relocation();

public:
	~Relocation() {};
	Relocation(char* ndll, char* nfunc, DWORD noffs)
	    : dll(ndll)
	    , func(nfunc)
	    , offs(noffs) {};
	operator DWORD() { return base; };
	void Relocate();
};

typedef void (*relativefunc)();
typedef struct _TAGABSOLUTEFUNC* absolutefunc;

// Memory patch class
class Patch {
	char* orig;
	char* repl;
	char* offs;
	int len;
	Relocation* reloc;

public:
	~Patch();
	Patch(DWORD offset, char* replace, int l, Relocation* nreloc = 0);
	Patch(DWORD offset, relativefunc func, Relocation* nreloc = 0);
	Patch(DWORD offset, absolutefunc func, Relocation* nreloc = 0);
	Patch();
	int Apply(void);
	int Remove(void);

private:
	bool relocdata;
	bool enabled;
};

#endif
