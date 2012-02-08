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
#include "../audiocomponentdescriptor.h"
#include <string>
#include <stdio.h>

namespace tuner {
namespace desc {

bool fnc0c4Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len ) {
	SSIZE_T offset=0;
	AudioComponentDescriptor desc;
	
	desc.streamContent = RB(descPayload,offset);
	desc.streamContent &= 0x0F;
	desc.componentType = RB(descPayload,offset);
	desc.componentTag  = RB(descPayload,offset);
	desc.streamType    = RB(descPayload,offset);
	desc.simulcastGroupTag = RB(descPayload,offset);
	
	BYTE data = RB(descPayload,offset);
	desc.multiLanguageFlag = (data & 0x80) ? true : false;
	desc.mainComponentFlag = (data & 0x40) ? true : false;
	desc.qualityIndicator  = (data & 0x30) >> 4;
	desc.samplingRate      = (data & 0x0E) >> 1;

	std::string lang;
	DESC_LANGUAGE(lang,descPayload,offset);
	desc.languages.push_back( lang );

	if (desc.multiLanguageFlag) {
		DESC_LANGUAGE(lang,descPayload,offset);
		desc.languages.push_back( lang );
	}
	if (len > offset) {
		desc.text.assign( (char *)(descPayload+offset), len-offset );
		offset += (len-offset);
	}
	descriptors[0xc4] = desc;
    return true;
}

}
}
