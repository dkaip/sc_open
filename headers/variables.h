/************************************************************************
sc - Station Controller
Copyright (C) 2005 
Written by John F. Poirier DBA Edge Integration

Contact information:
    john@edgeintegration.com

    Edge Integration
    885 North Main Street
    Raynham MA, 02767

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
************************************************************************/

#ifndef VARIABLES_H_
#define VARIABLES_H_

enum var_types
{
    VT_USER_DEFINE, 
    VT_USER_GLOBAL, 
    VT_USER_LOCAL,
    VT_USER, 
    VT_USER_REF, 

    VT_SC, 
    VT_LABEL 
};

/* Variables are prefixed with these.  Keep sc variables separate from program variables.
   All sc internal variables begin with a #, while the user's or program variables don't 
*/

extern char *var_prefix[];

#endif /* VARIABLES_H_ */
