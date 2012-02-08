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
#include "totdemuxer.h"
#include "psi.h"
#include "../descriptors/demuxer.h"
#include <stdio.h>

namespace tuner {

TOTDemuxer::TOTDemuxer( WORD pid )
	: PSIDemuxer( pid )
{
}

TOTDemuxer::~TOTDemuxer( void )
{
}

//	Getters
bool TOTDemuxer::syntax() const {
	return false;
}

ID TOTDemuxer::tableID() const {
	return PSI_TID_TOT;
}

//  Signal
void TOTDemuxer::onParsed( const ParsedCallback &callback ) {
	_onParsed = callback;
}

DWORD TOTDemuxer::frequency() const {
	return 30*1000;
}

void TOTDemuxer::onSection( BYTE *section, SIZE_T len ) {
	//	Check CRC
	if (checkCRC( section, len )) {
		desc::Demuxer descDemuxer;
		desc::MapOfDescriptors descs;
		SSIZE_T offset=3;
	
		WORD mjd = RW(section,offset);
		DWORD lsb = RW(section,offset)
		lsb <<= 8;
		lsb  |= RB(section,offset);

		offset += descDemuxer.parse( descs, section+offset );

		_onParsed( new Tot( mjd, lsb, descs ) );
	}
	else {
		printf( "[TOTDemuxer] Warning: CRC error!\n" );
	}
}

}
