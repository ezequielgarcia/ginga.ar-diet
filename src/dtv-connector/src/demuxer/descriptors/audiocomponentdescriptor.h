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
#pragma once

#include "../descriptors.h"
#include <string>
#include <vector>

namespace tuner {
namespace desc {

typedef struct {
	BYTE streamContent;
	BYTE componentType;
	BYTE componentTag;
	BYTE streamType;
	BYTE simulcastGroupTag;
	bool multiLanguageFlag;
	bool mainComponentFlag;
	BYTE qualityIndicator;
	BYTE samplingRate;
	std::vector<std::string> languages;
	std::string text;	
} AudioComponentDescriptor;

inline bool operator==( const AudioComponentDescriptor &ac1, const AudioComponentDescriptor &ac2 ) {
	return ac1.streamContent == ac2.streamContent &&
	ac1.componentType == ac2.componentType &&
	ac1.componentTag == ac2.componentTag &&
	ac1.streamType == ac2.streamType &&
	ac1.simulcastGroupTag == ac2.simulcastGroupTag &&
	ac1.multiLanguageFlag == ac2.multiLanguageFlag &&
	ac1.mainComponentFlag == ac2.mainComponentFlag &&
	ac1.qualityIndicator == ac2.qualityIndicator &&
	ac1.samplingRate == ac2.samplingRate &&
	ac1.languages == ac2.languages &&
	ac1.text == ac2.text;
}

}
}
