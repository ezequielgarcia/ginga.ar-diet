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
#include "psifilter.h"
#include "../../demuxer/ts.h"
#include "../../demuxer/psi/psi.h"
#include <stdio.h>

namespace tuner {

PSIFilter::PSIFilter( ID pid, const SectionCallback &callback )
	: PSIDemuxer( pid )
{
	_onSection = callback;
	_continuityCounter = TS_INVALID_CONTINUITY;
}

PSIFilter::~PSIFilter( void )
{
}

void PSIFilter::onSection( BYTE *section, SIZE_T len ) {
	if (!_onSection.empty()) {
		_onSection( pid(), section, len+4 );
	}
}

bool PSIFilter::useStandardVersion() const {
	return false;
}

bool PSIFilter::checkContinuity( bool tsStart, util::BYTE actualCC ) {
	bool result=true;
	bool begin;

	if (_continuityCounter != TS_INVALID_CONTINUITY) {
		begin = false;
			
		//	Get next continuity counter
		_continuityCounter++;
		_continuityCounter = (_continuityCounter % 0x10);
		if (_continuityCounter != actualCC) {
			//printf( "[TSDemuxer] actual=%02x, next=%02x\n", actualCC, continuityCounter );
			//	Mark invalid
			_continuityCounter = TS_INVALID_CONTINUITY;
		}
	}
	else {
		//	Begin of a PID
		begin = true;
	}

	//	Recheck if continuity is invalid
	if (_continuityCounter == TS_INVALID_CONTINUITY) {
		if (tsStart) {
			_continuityCounter = actualCC;
		}
		else {
			result=false;
			if (!begin) {
				printf( "[PSIFilter] Warning: Dropped by continuity counter\n" );
			}
		}
	}
	return result;
}

}
