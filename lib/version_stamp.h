//
// UCSD p-System virtual machine
// Copyright (C) 2006 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at
// you option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>
//

#ifndef LIB_VERSION_STAMP_H
#define LIB_VERSION_STAMP_H

/**
  * The version_stamp function is used to obtain the version stamp
  * assigned to the change set when it was built.
  */
const char *version_stamp(void);

/**
  * The copyright_years function is used to obtain the list of copyright
  * years for this project.
  */
const char *copyright_years(void);

#endif // LIB_VERSION_STAMP_H
