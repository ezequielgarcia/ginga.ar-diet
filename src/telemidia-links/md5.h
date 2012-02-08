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
/* md5.h
 * This file is a part of the Links program, released under GPL.
 */

#ifndef _MD5_H_
#define _MD5_H_

/*#include <sys/types.h>*/

#define MD5_HASHBYTES 16

#ifndef u_int32_t
#define u_int32_t unsigned
#endif

typedef struct MD5Context {
	u_int32_t buf[4];
	u_int32_t bits[2];
	unsigned char in[64];
} JS_MD5_CTX;

extern void   MD5Init(JS_MD5_CTX *context);
extern void   MD5Update(JS_MD5_CTX *context, unsigned char const *buf,
	       unsigned len);
extern void   MD5Final(unsigned char digest[MD5_HASHBYTES], JS_MD5_CTX *context);
extern void   MD5Transform(u_int32_t buf[4], u_int32_t const in[16]);
extern char * MD5End(JS_MD5_CTX *, char *);
extern char * MD5File(const char *, char *);
extern char * MD5Data (const unsigned char *, unsigned int, char *);

#endif /* !_MD5_H_ */
