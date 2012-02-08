/*******************************************************************************

  Copyright 2010 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata

********************************************************************************

  This file is part of Ginga implementation.

    This program is free software: you can redistribute it and/or modify it 
  under the terms of the GNU General Public License as published by the Free 
  Software Foundation, either version 2 of the License.

    Ginga is distributed in the hope that it will be useful, but WITHOUT ANY 
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with
  this program. If not, see <http://www.gnu.org/licenses/>.

********************************************************************************

  Este archivo es parte de la implementación de Ginga.

    Este programa es Software Libre: Ud. puede redistribuirlo y/o modificarlo 
  bajo los términos de la Licencia Pública General GNU como es publicada por la
  Free Software Foundation, según la versión 2 de la licencia.

    Ginga se distribuye esperando que resulte de utilidad, pero SIN NINGUNA 
  GARANTÍA; ni siquiera la garantía implícita de COMERCIALIZACIÓN o ADECUACIÓN 
  PARA ALGÚN PROPÓSITO PARTICULAR. Para más detalles, revise la Licencia Pública 
  General GNU.

    Ud. debería haber recibido una copia de la Licencia Pública General GNU 
  junto a este programa. Si no, puede verla en <http://www.gnu.org/licenses/>.

*******************************************************************************/
/* bits.h
 * (c) 2002 Karel 'Clock' Kulhavy
 * This file is a part of the Links program, released under GPL.
 */

/* t2c
 * Type that has exactly 2 chars.
 * If there is none, t2c is not defined
 * The type may be signed or unsigned, it doesn't matter
 */
#if SIZEOF_SHORT == 2
#define t2c short
#elif SIZEOF_UNSIGNED_SHORT == 2
#define t2c unsigned short
#elif SIZEOF_INT == 2
#define t2c int
#elif SIZEOF_UNSIGNED == 2
#define t2c unsigned
#elif SIZEOF_LONG == 2
#define t2c long
#elif SIZEOF_UNSIGNED_LONG == 2
#define t2c unsigned long
#endif /* #if sizeof(short) */

/* t4c
 * Type that has exactly 4 chars.
 * If there is none, t2c is not defined
 * The type may be signed or unsigned, it doesn't matter
 */
#if SIZEOF_SHORT == 4
#define t4c short
#elif SIZEOF_UNSIGNED_SHORT == 4
#define t4c unsigned short
#elif SIZEOF_INT == 4
#define t4c int
#elif SIZEOF_UNSIGNED == 4
#define t4c unsigned
#elif SIZEOF_LONG == 4
#define t4c long
#elif SIZEOF_UNSIGNED_LONG == 4
#define t4c unsigned long
#endif /* #if sizeof(short) */

