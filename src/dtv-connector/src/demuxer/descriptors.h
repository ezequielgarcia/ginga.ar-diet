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

#include "../types.h"
#include <util/any.h>
#include <map>

namespace tuner {
namespace desc {

//  Map of descriptors (Section 2.6 ISO)
typedef std::pair<util::BYTE,util::Any> DescriptorItem;
typedef std::map<util::BYTE,util::Any> MapOfDescriptors;


namespace ident {

enum type {
	carousel_id                =0x13,
	association_tag            =0x14,
	network_name               =0x40,
	service_list               =0x41,
	service                    =0x48,
	short_event                =0x4d,
	stream_identifier          =0x52,
	content                    =0x54,
	parental_rating            =0x55,
	local_time_offset          =0x58,
	application_signalling     =0x6f,
	audio_component            =0xc4,
	video_decode               =0xc8,
	ts_information             =0xcd,
	terrestrial_delivery_system=0xfa,
	parcial_reception          =0xfb,
	data_component             =0xfd,
	system_management          =0xfe
};

}

void show( const MapOfDescriptors &desc );

inline bool findDesc( const MapOfDescriptors &gDesc, const MapOfDescriptors &aDesc, BYTE tag, MapOfDescriptors::const_iterator &it ) {
	bool result=true;
	MapOfDescriptors::const_iterator gIt = gDesc.find( tag );
	MapOfDescriptors::const_iterator aIt = aDesc.find( tag );

	if (aIt != aDesc.end()) {
		it=aIt;
	}
	else if (gIt != gDesc.end()) {
		it=gIt;
	}
	else {
		result=false;
	}
	return result;
}

}
}

