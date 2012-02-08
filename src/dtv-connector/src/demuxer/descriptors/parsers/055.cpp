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
#include <string>
#include <vector>
#include <stdio.h>

namespace tuner {
namespace desc {

bool fnc055Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len ) {
    SSIZE_T offset = 0;
	ParentalRatingDescriptor desc;
	ParentalRating elem;

    while (offset < len) {
		DESC_LANGUAGE(elem.language,descPayload,offset);
        elem.rating = RB(descPayload,offset);
        desc.push_back( elem );
    }

    descriptors[0x55] = desc;
	show( desc );

    return true;
}

void show( const ParentalRatingDescriptor &desc ) {
	printf( "[TAG::55] Parental Rating\n" );
	BOOST_FOREACH( const ParentalRating &elem, desc ) {
		printf( "[TAG::55]\t language=%s, rating=%02x\n", elem.language.c_str(), elem.rating );
	}
}

}
}
