/*******************************************************************************

  Copyright (C) 2010, 2011 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata

********************************************************************************

  This file is part of DTV implementation.

    DTV is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation, either version 2 of the License.

    DTV is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
  this program. If not, see <http://www.gnu.org/licenses/>.

********************************************************************************

  Este archivo es parte de la implementación de DTV.

    DTV es Software Libre: Ud. puede redistribuirlo y/o modificarlo
  bajo los términos de la Licencia Pública General Reducida GNU como es publicada por la
  Free Software Foundation, según la versión 2 de la licencia.

    DTV se distribuye esperando que resulte de utilidad, pero SIN NINGUNA
  GARANTÍA; ni siquiera la garantía implícita de COMERCIALIZACIÓN o ADECUACIÓN
  PARA ALGÚN PROPÓSITO PARTICULAR. Para más detalles, revise la Licencia Pública
  General Reducida GNU.

    Ud. debería haber recibido una copia de la Licencia Pública General Reducida GNU
  junto a este programa. Si no, puede verla en <http://www.gnu.org/licenses/>.

*******************************************************************************/
#include "string.h"
#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <exception>
#include <typeinfo>

#define MAX_BUFFER_SIZE 1024

namespace util {

std::string format( const char *format, ... ) {
	char szBuffer[MAX_BUFFER_SIZE];
	if (!format) {
		throw std::bad_cast();//	TODO: mejor exception
	}
#ifdef _WIN32
	va_list arglist;
	va_start( arglist, format );
	int res=_vsnprintf_s( szBuffer, MAX_BUFFER_SIZE, _TRUNCATE, format, arglist );
	va_end( arglist );
	return std::string(szBuffer);
#else
	va_list arglist;
	va_start( arglist, format );
	int res=vsnprintf( szBuffer, MAX_BUFFER_SIZE, format, arglist );
	va_end( arglist );
	if (res >= 0) {
		return std::string(szBuffer);
	}
	else {
		return std::string( "" );
	}
#endif
}

}
