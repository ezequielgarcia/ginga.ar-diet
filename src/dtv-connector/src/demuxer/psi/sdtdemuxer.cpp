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
#include "sdtdemuxer.h"
#include "psi.h"
#include "../../service/types.h"
#include "../descriptors/demuxer.h"
#include <stdio.h>

//	Implemented based on
//		- ETSI EN 300 468 v1.5.1

//	EN 300 468 v1.5.1
#define TABLE_ACTUAL_TS 0x42
#define TABLE_OTHER_TS 0x46

namespace tuner {

SDTDemuxer::SDTDemuxer( ID pid )
  : PSIDemuxer( pid )
{
}

SDTDemuxer::~SDTDemuxer()
{
}

//  Signals
void SDTDemuxer::onParsed( const SDTDemuxer::ParsedCallback &callback ) {
	_onParsed = callback;
}

//  TSSectionDemuxer Virtual method
DWORD SDTDemuxer::frequency() const {
	return 2000;
}

void SDTDemuxer::onSection( BYTE *section, SIZE_T len ) {
	std::vector<Sdt::Service> services;
	
    SSIZE_T offset = PSI_PAYLOAD_OFFSET;

	Version version = PSI_VERSION(section);
	bool actualTS   = (PSI_TABLE(section) == TABLE_ACTUAL_TS) ? true :false;
	ID tsID         = PSI_EXTENSION(section);
	ID networkID    = RW(section,offset);
	offset += 1;	//	Reserved

	while (offset < len) {
		Sdt::Service srv;
		desc::Demuxer descDemuxer;
		BYTE tmp;

		//	ServiceID
		srv.serviceID = RW(section,offset);

		//	EIT flags
		tmp = RB(section,offset);
		srv.eitSchedule = tmp & 0x02;
		srv.eitPresentFollowing = tmp & 0x01;

		//	Running status & free CA mode
		tmp = RB(section,offset);
		
		//	Sanity the run status
		srv.status = service::status::type(tmp & 0xE0);
		if (srv.status > service::status::running) {
			srv.status = service::status::running;
		}
		
		srv.caMode = (tmp & 0x10) ? true : false;

		//  Descriptors
		offset -= 1;
		offset += descDemuxer.parse( srv.descriptors, section+offset );

		services.push_back( srv );
	}
	
    Sdt *sdt = new Sdt( version, tsID, networkID, actualTS, services );
    _onParsed( sdt );
}

Version SDTDemuxer::updateVersion( Version ver, BYTE *section, bool apply ) {
 	Subtable sub;
    SSIZE_T offset = PSI_PAYLOAD_OFFSET;	

	//	Subtable is: tableID + tsID + originalNetworkID + version
	sub.tableID   = PSI_TABLE(section);
	sub.tsID      = PSI_EXTENSION(section);
	sub.networkID = RW(section,offset);
    sub.version   = ver;
	return psi::chgVersion( _tables, sub, apply );
}

bool SDTDemuxer::supportMultipleSections( void ) const {
	//	TODO: Need support!!
	return false;
}

}
