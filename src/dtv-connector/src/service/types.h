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

namespace tuner {
namespace service {

typedef util::BYTE Type;

namespace type {

enum type {
	reserved=0x0,
	digital_television=0x01,
	digital_radio=0x02,
	teletext=0x03,
	nvod_reference=0x04,
	nvod_time_shiftted=0x05,
	mosaic=0x06,
	pal=0x07,
	secam=0x08,
	d_d2_mac=0x09,
	fm_radio=0x0a,
	ntsc=0x0b,
	data_broadcast=0x0c,
	ciu=0x0d,
	rcs_map=0x0e,
	rcs_fls=0x0f,
	dvb_mhp=0x10,
	engineering=0xa4,
	one_seg=0xc0
};

}	//	namespace type


namespace status {

enum type {
	undefined=0x0,
	not_running=0x1,
	starting=0x2,
	pausing=0x3,
	running=0x4
};
	
}	//	namespace status

//	State of a service in the service provider
namespace state {

enum type {
	present,
	complete,
	expired,
	timeout,
	ready,
	running
};
	
}

}	//	namespace service
}
