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
#include "tvplayer.h"
#include "player.h"
#include "../../service.h"
#include "../../servicemanager.h"
#include "../../../demuxer/ts.h"
#include <boost/foreach.hpp>
#include <stdio.h>

namespace tuner {

TvPlayer::TvPlayer( Player *player )
	: ServicePlayer( player )
{
}

TvPlayer::~TvPlayer( void )
{
}

bool TvPlayer::isValid( Service *srv ) const {
	return srv->pcrPID() != TS_PID_NULL && haveVideo( srv );
}

bool TvPlayer::play( ServiceManager *mgr, Service *srv ) {
	bool result = false;
	bool addPCR = true;
	PlayInfo playInfo;

	//	Check that provider support a valid pipe of stream
	playInfo.url = mgr->streamPipe();
	if (playInfo.url.size()) {
		playInfo.pcrPID = srv->pcrPID();
		
		const std::vector<tuner::Pmt::ElementaryInfo> &elements = srv->elements();
		BOOST_FOREACH( tuner::Pmt::ElementaryInfo info, elements ) {
			if (isAudio(info.streamType) || isVideo(info.streamType)) {
				printf( "[TvPlayer] Add PES filter: pid=%04x, type=%04x\n",
					info.pid, info.streamType );
				
				//	Add PES filter
				result = addFilter( mgr, info.pid );
				if (!result) {
					break;
				}

				//	Check if PES pid is PCR pid
				if (playInfo.pcrPID == info.pid) {
					addPCR = false;
				}

				playInfo.pids.push_back( std::make_pair( info.pid, info.streamType ) );
			}
		}

		//	Add PCR PES
		if (result && addPCR) {
			result=addFilter( mgr, srv->pcrPID() );
		}

		//	play TS
		if (result) {
			result = player()->play( playInfo );
		}

		//	Check result
		if (!result) {
			stop( mgr, srv );
		}
	}
	else {
		printf( "[TvPlayer] Warning, provider not support a valid stream pipe\n" );
	}
	
	return result;
}

void TvPlayer::stop( ServiceManager *mgr, Service * /*srv*/ ) {
	//	Stop video player
	player()->stop();

	//	Stop PES filters
	BOOST_FOREACH( ID pid, _filters ) {
		mgr->stopFilter( pid );
	}
	_filters.clear();
}

bool TvPlayer::addFilter( ServiceManager *mgr, ID pid ) {
	bool result= mgr->startPesFilter( pid );
	if (result) {
		_filters.push_back( pid );
	}
	return result;
}

}
