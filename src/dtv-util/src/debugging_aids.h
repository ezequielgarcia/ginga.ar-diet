/*******************************************************************************

  Copyright (C) 2010, 2011 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata

********************************************************************************

  This file is part of DTV-util implementation.

    DTV-util is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation, either version 2 of the License.

    DTV-util is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
  this program. If not, see <http://www.gnu.org/licenses/>.

********************************************************************************

  Este archivo es parte de la implementación de DTV-util.

    DTV-util es Software Libre: Ud. puede redistribuirlo y/o modificarlo
  bajo los términos de la Licencia Pública General Reducida GNU como es publicada por la
  Free Software Foundation, según la versión 2 de la licencia.

    DTV-util se distribuye esperando que resulte de utilidad, pero SIN NINGUNA
  GARANTÍA; ni siquiera la garantía implícita de COMERCIALIZACIÓN o ADECUACIÓN
  PARA ALGÚN PROPÓSITO PARTICULAR. Para más detalles, revise la Licencia Pública
  General Reducida GNU.

    Ud. debería haber recibido una copia de la Licencia Pública General Reducida GNU
  junto a este programa. Si no, puede verla en <http://www.gnu.org/licenses/>.

*******************************************************************************/
#ifndef GINGAUTIL_DEBUGGING_AIDS_H
#define GINGAUTIL_DEBUGGING_AIDS_H

#include <execinfo.h>
#include <cstdlib>
#include <cstdio>

#define BACKTRACE_FRAMES_MAX (20)

namespace util {
	
void custom_terminate()
{
	void * array[BACKTRACE_FRAMES_MAX];
	int size = backtrace(array, BACKTRACE_FRAMES_MAX);
	
	fprintf(stderr, "[ GINGAUTIL - terminate ] Dumping backtrace of %d frames\n", size);
	backtrace_symbols_fd (array, size, fileno(stderr));
	fprintf(stderr, "[ GINGAUTIL - terminate ] calling abort()");
	
	abort();
}

} /* namespace util */

#endif /* GINGAUTIL_DEBUGGING_AIDS_H */

