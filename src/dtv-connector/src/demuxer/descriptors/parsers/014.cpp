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

namespace tuner {
namespace desc {

bool fnc014Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T /*len*/ ) {
    SSIZE_T offset=0;
	AssociationTagDescriptor desc;

    desc.tag = RW(descPayload,offset);
    desc.use = RW(descPayload,offset);

	//BYTE selectorLen = RB(descPayload,offset);
	//      Documented in TR101202
	//      0x0000           DSI with IOR of SGW
	//      0x0100 - 0x1FFF  DVB reserved
	//      0x2000 - 0xFFFF  User private
	// switch(_use) {
	//      case 0:
	//              DWORD transactionID = RDW(data,offset);
	//              DWORD timeout = RDW(data,offset);
	//              break;
	//      case 1:
	//              break;
	//      case default:
	//              offset += selectorLen;
	//              break;
	// }
	
	descriptors[0x14] = desc;
	show( desc );
	
    return true;
}

void show( const AssociationTagDescriptor &desc ) {
	printf( "[TAG::14] Association Tag: tag=%02x, use=%02x\n", desc.tag, desc.use );
}

}
}
