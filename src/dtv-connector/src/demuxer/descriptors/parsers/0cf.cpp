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
#include <stdio.h>

namespace tuner {
namespace desc {

bool fnc0cfParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len ) {
	SSIZE_T offset=0;
	LogoTransmissionDescriptor desc;

	desc.type = RB(descPayload,offset);

	switch (desc.type) {
		case LOGO_TYPE1: {
			LogoTransmissionType1 logo;
			logo.id   = RW(descPayload,offset);
			logo.id  &= 0x01FF;
			logo.version  = RW(descPayload,offset);
			logo.version &= 0x0FFF;
			logo.downloadID = RW(descPayload,offset);
			desc.data = logo;
			break;
		}
		case LOGO_TYPE2: {
			LogoTransmissionType2 logo;
			logo  = RW(descPayload,offset);
			logo &= 0x01FF;
			desc.data = logo;
			break;
		}
		case LOGO_TYPE3: {
			LogoTransmissionType3 logo;
			DESC_STRING(logo,len-offset,descPayload,offset);
			desc.data = logo;
			break;
		}
		default:
			printf( "[TAG::CF] Warning: invalid type of logo transmission descriptor type=%02x\n", desc.type );
			break;		
	};

	descriptors[0xcf] = desc;
	show( desc );

	return true;
}

void show( const LogoTransmissionDescriptor &desc ) {
	switch (desc.type) {
		case LOGO_TYPE1: {
			const LogoTransmissionType1 &logo = desc.data.get<LogoTransmissionType1>();
			printf( "[TAG::CF] Logo transmission descriptor: type=01, id=%04x, ver=%04x, downloadID=%04x\n",
				logo.id, logo.version, logo.downloadID );
			break;
		}
		case LOGO_TYPE2: {
			const LogoTransmissionType2 &logo = desc.data.get<LogoTransmissionType2>();
			printf( "[TAG::CF] Logo transmission descriptor: type=02, id=%04x\n", logo );
			break;
		}
		case LOGO_TYPE3: {
			const LogoTransmissionType3 &logo = desc.data.get<LogoTransmissionType3>();
			printf( "[TAG::CF] Logo transmission descriptor: type=03, str=%s\n", logo.c_str() );			
			break;
		}
	};
}

}
}

