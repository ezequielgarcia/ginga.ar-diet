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
#include "pmtdemuxer.h"
#include "psi.h"
#include "../descriptors/demuxer.h"

namespace tuner {

PMTDemuxer::PMTDemuxer( ID pid )
: PSIDemuxer( pid )
{
}

PMTDemuxer::~PMTDemuxer()
{
}

//	Getters
ID PMTDemuxer::tableID() const {
	return PSI_TID_PMT;
}

//	Signals
void PMTDemuxer::onParsed( const ParsedCallback &callback ) {
	_onParsed = callback;
}

//	TSSectionDemuxer Virtual method
DWORD PMTDemuxer::frequency() const {
	return 500;
}

void PMTDemuxer::onSection( BYTE *section, SIZE_T len ) {
	desc::Demuxer descDemuxer;
	desc::MapOfDescriptors piDescriptors;
	std::vector<Pmt::ElementaryInfo> elements;
	SSIZE_T offset = PSI_PAYLOAD_OFFSET;

	//	Program number ...
	ID programNumber = PSI_EXTENSION(section);

	//	PCR pid
	ID pcrPid  = GET_PID(section+offset);
	offset	  += 2;

	//	Program Information descriptors
	offset += descDemuxer.parse( piDescriptors, section+offset );

	//	Parse elementary pids
	while (offset < len) {
		Pmt::ElementaryInfo info;

		//	Get stream type
		info.streamType = RB(section,offset);

		//	Elementary PID
		info.pid = GET_PID(section+offset);
		offset += 2;

		//	Descriptors
		offset += descDemuxer.parse( info.descriptors, section+offset );

		elements.push_back( info );
	}

	Pmt *pmt = new Pmt( pid(), PSI_VERSION(section), programNumber, pcrPid, piDescriptors, elements );
	_onParsed( pmt );
}

Version PMTDemuxer::updateVersion( Version ver, BYTE *section, bool apply ) {
	PMTDemuxer::Subtable sub;	

	//	Subtable is: programID + version
	sub.programID = PSI_EXTENSION(section);	
	sub.version   = ver;
	return psi::chgVersion( _tables, sub, apply );
}

bool PMTDemuxer::supportMultipleSections( void ) const {
	return false;
}

}
