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
#include "nitdemuxer.h"
#include "psi.h"
#include "../descriptors/demuxer.h"
#include <stdio.h>

//	Implemented based on
//		- ETSI EN 300 468 v1.5.1

namespace tuner {

NITDemuxer::NITDemuxer( ID pid )
: PSIDemuxer( pid )
{
	_onlyActual = false;
}

NITDemuxer::NITDemuxer( ID pid, bool onlyActual )
: PSIDemuxer( pid )
{
	_onlyActual = onlyActual;
}

NITDemuxer::~NITDemuxer()
{
}

//	Getters
ID NITDemuxer::tableID() const {
	return _onlyActual ? PSI_TID_NIT_ACTUAL : PSI_TID_FORBIDDEN;
}

//  Signals
void NITDemuxer::onParsed( const ParsedCallback &callback ) {
    _onParsed = callback;
}

//  TSSectionDemuxer Virtual method
DWORD NITDemuxer::frequency() const {
	return 2000;
}

void NITDemuxer::onSection( BYTE *section, SIZE_T len ) {
    desc::Demuxer descDemuxer;
    desc::MapOfDescriptors netDescriptors;
    std::vector<Nit::ElementaryInfo> elements;
    SSIZE_T offset = PSI_PAYLOAD_OFFSET;

    bool isActualNetwork = (PSI_TABLE(section) == PSI_TID_NIT_ACTUAL);
    WORD networkID  = PSI_EXTENSION(section);
	Version version = PSI_VERSION(section);

    //  Network descriptors
    offset += descDemuxer.parse( netDescriptors, section+offset );
    offset += 2;    //  Transport stream loop length

    while (offset < len) {
		Nit::ElementaryInfo info;

        info.tsID  = RW(section,offset);
        info.nitID = RW(section,offset);

        //  Transport Stream Descriptors
        offset += descDemuxer.parse( info.descriptors, section+offset );

        elements.push_back( info );
    }

    Nit *nit = new Nit( version, networkID, isActualNetwork, netDescriptors, elements );
    _onParsed( nit );
}

Version NITDemuxer::updateVersion( Version ver, BYTE *section, bool apply ) {
	NITDemuxer::Subtable sub;	

	//	Subtable is: tableID + originalNetworkID + version
	sub.tableID   = PSI_TABLE(section);
    sub.networkID = PSI_EXTENSION(section);	
    sub.version   = ver;
	return psi::chgVersion( _tables, sub, apply );
}

bool NITDemuxer::supportMultipleSections( void ) const {
	//	TODO: NIT must be support multiple sections .... need complete!
	return false;
}

}
