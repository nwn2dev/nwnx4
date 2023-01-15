/***************************************************************************
    IATHook - Image import table hook.
    Copyright (C) 2008 Skywing (skywing@valhallalegends.com)

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

#include "IATHook.h"
#include <windows.h>

bool RedirectImageImports(__in HMODULE Module,
                          __in PVOID* Imports,
                          __in PVOID* DesiredAddresses,
                          __out_opt PULONG PatchCounts,
                          __in SIZE_T Count)
{
	PIMAGE_DATA_DIRECTORY ImportDirectory;
	PIMAGE_IMPORT_DESCRIPTOR ImportDescriptor;
	PIMAGE_DOS_HEADER DosHeader;
	PIMAGE_NT_HEADERS NtHeaders;
	ULONG_PTR ModuleBase;
	PIMAGE_IMPORT_DESCRIPTOR LastImport;
	PVOID* CurrentImport;
	SIZE_T i;
	bool Success;

	Success    = false;
	ModuleBase = reinterpret_cast<ULONG_PTR>(Module);
	DosHeader  = reinterpret_cast<PIMAGE_DOS_HEADER>(ModuleBase);
	NtHeaders  = reinterpret_cast<PIMAGE_NT_HEADERS>(ModuleBase + DosHeader->e_lfanew);

	ImportDirectory = &NtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

	if (PatchCounts) {
		for (i = 0; i < Count; i += 1) {
			PatchCounts[i] = 0;
		}
	}

	for (;;) {
		//
		// Ensure that we've actually got an import directory.
		//

		if (ImportDirectory->Size < sizeof(IMAGE_IMPORT_DESCRIPTOR)) {
			//			DebugPrint(
			//				"ImportDirectorySize %lu is too small.\n",
			//				ImportDirectory->Size
			//				);
			break;
		}

		//
		// Calculate the last import descriptor so we don't run off the end of
		// the array.
		//

		LastImport = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(
		    ModuleBase + ImportDirectory->VirtualAddress + ImportDirectory->Size);

		//		DebugPrint(
		//			"Last import descriptor @ %p\n", LastImport
		//			);

		//
		// Iterate over each import descriptor so that we can enumerate all
		// imports associated with that descriptor.
		//

		for (ImportDescriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(
		         ModuleBase + ImportDirectory->VirtualAddress);
		     ImportDescriptor != LastImport; ImportDescriptor += 1) {
			//			DebugPrint(
			//				"Inspecting import descriptor @ %p\n",
			//				ImportDescriptor
			//				);

			//
			// An empty import descriptor terminates the array, so we shall
			// stop if we encounter one.
			//

			if ((!ImportDescriptor->Name) || (!ImportDescriptor->FirstThunk)) {
				//				DebugPrint( "Reached null import descriptor.\n" );
				break;
			}

			//
			// Iterate through the imports associated with this import
			// descriptor.  We assume that they have all been snapped, and thus
			// will contain the address we're looking for, if this module does
			// in fact actually import it.
			//

			for (CurrentImport
			     = reinterpret_cast<PVOID*>(ModuleBase + ImportDescriptor->FirstThunk);
			     *CurrentImport; CurrentImport += 1) {
				DWORD OldProtection;

				//				DebugPrint(
				//					"Inspecting import thunk %p [@%p]...\n",
				//					*CurrentImport,
				//					CurrentImport
				//					);

				//
				// If we're not at a desired import, then we'll continue on to
				// the next snapped import thunk.
				//

				for (i = 0; i < Count; i += 1) {
					if (*CurrentImport == Imports[i])
						break;
				}

				if (i == Count)
					continue;

				//
				// Okay, we've got a match.  Let's perform our interception
				// patching now.  Note that we must use PAGE_EXECUTE_*, as if
				// the import directory happens to be on the same page as code,
				// we might accidentally make that code non-executable for the
				// duration of the patch.
				//

				if (!VirtualProtect(CurrentImport, sizeof(*CurrentImport), PAGE_EXECUTE_READWRITE,
				                    &OldProtection))
					break;

				//
				// Rewrite the snapped import thunk address.
				//

				*CurrentImport = DesiredAddresses[i];

				//
				// Restore old memory protection.
				//

				VirtualProtect(CurrentImport, sizeof(*CurrentImport), OldProtection,
				               &OldProtection);

				//
				// If the caller is keeping track of what we're actually
				// successfully patched then update the information.
				//

				if (PatchCounts)
					PatchCounts[i] += 1;

				//
				// Set the general success flag if we've got at least one
				// patch completed.  For multiple patches the caller needs to
				// supply a PatchCount array and inspect it.
				//

				Success = true;
			}
		}

		break;
	}

	return Success;
}
