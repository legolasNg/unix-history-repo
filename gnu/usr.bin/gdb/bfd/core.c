/* Core file generic interface routines for BFD.
   Copyright (C) 1990-1991 Free Software Foundation, Inc.
   Written by Cygnus Support.

This file is part of BFD, the Binary File Descriptor library.

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
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/*
SECTION
	Core files

DESCRIPTION
	Buff output this facinating topic
*/

#include "bfd.h"
#include "sysdep.h"
#include "libbfd.h"


/*
FUNCTION
	bfd_core_file_failing_command

SYNOPSIS
	CONST char *bfd_core_file_failing_command(bfd *);

DESCRIPTION
	Returns a read-only string explaining what program was running
	when it failed and produced the core file being read

*/

CONST char *
DEFUN(bfd_core_file_failing_command,(abfd),
      bfd *abfd)
{
  if (abfd->format != bfd_core) {
    bfd_error = invalid_operation;
    return NULL;
  }
  return BFD_SEND (abfd, _core_file_failing_command, (abfd));
}

/*
FUNCTION
	bfd_core_file_failing_signal

SYNOPSIS
	int bfd_core_file_failing_signal(bfd *);

DESCRIPTION
	Returns the signal number which caused the core dump which
	generated the file the BFD is attached to.
*/

int
bfd_core_file_failing_signal (abfd)
     bfd *abfd;
{
  if (abfd->format != bfd_core) {
    bfd_error = invalid_operation;
    return 0;
  }
  return BFD_SEND (abfd, _core_file_failing_signal, (abfd));
}


/*
FUNCTION
	core_file_matches_executable_p

SYNOPSIS
	boolean core_file_matches_executable_p
		(bfd *core_bfd, bfd *exec_bfd);

DESCRIPTION
	Returns <<true>> if the core file attached to @var{core_bfd}
	was generated by a run of the executable file attached to
	@var{exec_bfd}, or else <<false>>.
*/
boolean
core_file_matches_executable_p (core_bfd, exec_bfd)
     bfd *core_bfd, *exec_bfd;
{
    if ((core_bfd->format != bfd_core) || (exec_bfd->format != bfd_object)) {
	    bfd_error = wrong_format;
	    return false;
	}

    return BFD_SEND (core_bfd, _core_file_matches_executable_p,
		     (core_bfd, exec_bfd));
}
