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
#include "tot.h"
#include <stdio.h>

#define BCD2BIN(val) (((val) & 0xF) + ((val)>>4)*10) //	Convert from BCD to Binary

namespace tuner {

Tot::Tot( WORD mjd, DWORD lsb, const desc::MapOfDescriptors &descs )
{
	int year  = (mjd-15078.2)/365.25;
	int month = (mjd-14956.1-int(year*365.25))/30.6001;
	_day      = mjd-14956 - int(year*365.25) - int(month*30.6001);
	
	int k  = (month == 14 || month == 15) ? 1 : 0;

	_year  =  year+k+1900;
	_month = month-1-(k*12);
	_lsb   = lsb;
	_descs = descs;
}

Tot::~Tot( void )
{
}

int Tot::year() const {
	return _year;
}

int Tot::month() const {
	return _month;
}

int Tot::day() const {
	return _day;
}

int Tot::hours() const {
	return BCD2BIN((_lsb & 0x00FF0000) >> 16);
}

int Tot::minutes() const {
	return BCD2BIN((_lsb & 0x0000FF00) >> 8);
}

int Tot::seconds() const {
	return BCD2BIN(_lsb & 0x000000FF);
}

const desc::MapOfDescriptors &Tot::descriptors() const {
	return _descs;
}

void Tot::show() const {
	printf( "[Tot] %d/%d/%d %d:%d:%d, descs=%d\n",
		_day, _month, _year, hours(), minutes(), seconds(), _descs.size() );
}

}

