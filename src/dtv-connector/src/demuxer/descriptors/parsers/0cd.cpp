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

//	Information of TS
bool fnc0cdParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T /*len*/ ) {
	TransportStreamInformationDescriptor desc;
	SSIZE_T offset=0;
	BYTE data;

	//	Remote control key id
	desc.remoteControlKeyID = RB(descPayload,offset);

	data = RB(descPayload,offset);

	//	TS name
	BYTE tsNameLen = (data >> 2);
	desc.name = std::string( (char *)(descPayload+offset), tsNameLen );
	offset += tsNameLen;

	//	Transmision type count
	BYTE typeCount = (data & 0x3);
	for (int type=0; type<typeCount; type++) {
		TransmissionTypeStruct tType;
		
		tType.type = RB(descPayload,offset);
		data       = RB(descPayload,offset);
		for (BYTE srv=0; srv<data; srv++) {
			WORD serviceID = RW(descPayload,offset);
			tType.services.push_back( serviceID );
		}
		desc.transmissions.push_back( tType );
	}
	
	show( desc );
	descriptors[0xcd] = desc;
    return true;
}

void show( const TransportStreamInformationDescriptor &desc ) {
	printf( "[TAG::CD] Transport Stream information: name=%s, remote control=%02x\n", desc.name.c_str(), desc.remoteControlKeyID );
	BOOST_FOREACH( const TransmissionTypeInfo &tType, desc.transmissions ) {
		printf( "[TAG::CD]\ttransmisionType=%02x\n", tType.type );
		BOOST_FOREACH( WORD serviceID, tType.services ) {
			printf( "[TAG::CD]\t\tserviceID=%04x\n", serviceID );
		}
	}
}

}
}
