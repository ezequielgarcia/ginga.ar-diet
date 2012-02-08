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
#include <stdio.h>

namespace tuner {
namespace desc {

bool fnc058Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len ) {
	SSIZE_T offset=0;
	LocalTimeOffsetDescriptor desc;

	while (offset < len) {
		LocalTimeOffsetStruct off;

		DWORD tmp = RDW(descPayload,offset);

		off.countryCode   = tmp >> 8;
		off.countryRegion = (tmp & 0x0000000F) >> 2;
		off.localTimeOffsetPolarity = (tmp & 0x00000001) ? true : false;

		off.localTimeOffset = RW(descPayload,offset);

		off.timeOfChange   = RDW(descPayload,offset);
		off.timeOfChange <<= 8;
		off.timeOfChange   = RB(descPayload,offset);

		off.nextTimeOffset = RW(descPayload,offset);

		desc.push_back( off );
	}

	descriptors[0x58] = desc;
	show( desc );

	return true;
}

void show( const LocalTimeOffsetDescriptor &desc ) {
	BOOST_FOREACH( const LocalTimeOffset &lt, desc ) {
		printf( "[0x58] Country=%08lx, Region=%02x, Polarity=%d, Offset=%04x, TimeOfChange=%010llx, Next=%04x\n",
			lt.countryCode, lt.countryRegion, lt.localTimeOffsetPolarity, lt.localTimeOffset,
			lt.timeOfChange, lt.nextTimeOffset );
	}
}

}
}

