/*

    This file is part of the Maude 2 interpreter.

    Copyright 1997-2003 SRI International, Menlo Park, CA 94025, USA.

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.

*/

//
//	Global variables
//
extern int lineNumber;
extern FileTable fileTable;
extern DirectoryManager directoryManager;
extern Interpreter& interpreter;

#include "IO_Manager.hh"  // HACK
extern IO_Manager ioManager;

extern string executableDirectory;

extern bool alwaysAdviseFlag;

#define PRELUDE_NAME	"prelude.maude"
#define MAUDEV_LIB	"MAUDEV_LIB"

bool
findPrelude(string& directory, string& fileName);

bool
findFile(const string& userFileName, string& directory, string& fileName, int lineNr);

bool findExecutableDirectory(string& directory, string& executable);
