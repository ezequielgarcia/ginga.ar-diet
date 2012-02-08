/*******************************************************************************

  Copyright (C) 2010, 2011 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata

********************************************************************************

  This file is part of DTV-connector implementation.

    DTV-connector is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation, either version 2 of the License.

    DTV-connector is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
  this program. If not, see <http://www.gnu.org/licenses/>.

********************************************************************************

  Este archivo es parte de la implementación de DTV-connector.

    DTV-connector es Software Libre: Ud. puede redistribuirlo y/o modificarlo
  bajo los términos de la Licencia Pública General Reducida GNU como es publicada por la
  Free Software Foundation, según la versión 2 de la licencia.

    DTV-connector se distribuye esperando que resulte de utilidad, pero SIN NINGUNA
  GARANTÍA; ni siquiera la garantía implícita de COMERCIALIZACIÓN o ADECUACIÓN
  PARA ALGÚN PROPÓSITO PARTICULAR. Para más detalles, revise la Licencia Pública
  General Reducida GNU.

    Ud. debería haber recibido una copia de la Licencia Pública General Reducida GNU
  junto a este programa. Si no, puede verla en <http://www.gnu.org/licenses/>.

*******************************************************************************/
#include "funcs.h"
#include "../generic.h"
#include <boost/foreach.hpp>

namespace tuner {
namespace desc {

bool fnc06fParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len ) {
	SSIZE_T offset = 0;
	ApplicationSignallingElementInfo app;
	ApplicationSignallingDescriptor desc;
	
	while (offset < len) {
		//	15 bits of type
		app.type = RW(descPayload,offset);
		app.type &= 0xEFFF;

		//	5 bits of vesrion
		app.version = RB(descPayload,offset);
		app.version &= 0x1F;

		//	Add application type
		desc.push_back( app );
	}

	descriptors[0x6f] = desc;
	show( desc );
	
    return true;
}

void show( const ApplicationSignallingDescriptor &desc ) {
	BOOST_FOREACH( const ApplicationSignallingElementInfo elem, desc ) {
		printf( "[TAG::6F] Application Signalling: type=%02x, version=%02x\n", elem.type, elem.type );
	}
}

}
}
