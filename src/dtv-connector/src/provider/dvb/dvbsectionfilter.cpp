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

#include "dvbsectionfilter.h"
#include "dvbprovider.h"
#include "../../demuxer/tssectiondemuxer.h"
#include "../../demuxer/psi/psi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/dvb/dmx.h>

namespace tuner {
namespace dvb {

DVBSectionFilter::DVBSectionFilter( TSSectionDemuxer *demux, DVBProvider *prov )
	: DVBFilter( prov ),
	  _io( prov->io() )
{
	//	Ignore CRC
	demux->checkCRC( false );
	_demux = demux;
}

DVBSectionFilter::~DVBSectionFilter( void )
{
	delete _demux;
}

bool DVBSectionFilter::start( void ) {
	bool result=false;
	struct dmx_sct_filter_params fil;
	
	//	Setup filter
	memset(&fil, 0, sizeof(fil));

	//	Setup PID, timeout
	fil.pid     = _demux->pid();
	fil.timeout = _demux->timeout();	//	In milliseconds
	fil.flags   = DMX_IMMEDIATE_START | DMX_CHECK_CRC;

	//	Set filter buffer
	DWORD freq = _demux->frequency();
	if (freq && freq < 500 && _demux->maxSectionBytes() == TSS_PRI_MAX_BYTES) {
		//	1MB for high bitrate (default 2*4096)
		unsigned long size = 100*TSS_PRI_MAX_BYTES;
		ioctl( fd(), DMX_SET_BUFFER_SIZE, size );
	}

	//	Set filter
	if (!ioctl( fd(), DMX_SET_FILTER, &fil )) {
		_io.assign( fd() );
		dvb()->readSection( &_io, _demux->pid() );
		result=true;
	}
	return result;
}

void DVBSectionFilter::stop( void ) {
	DVBFilter::stop();
	_io.cancel();
}

FilterDemuxer *DVBSectionFilter::demux() const {
	return _demux;
}
	
}
}

