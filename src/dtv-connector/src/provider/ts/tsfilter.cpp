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
#include "tsfilter.h"
#include "../../demuxer/ts/tsdemuxer.h"
#include "../../demuxer/ts/psifilter.h"
#include <boost/bind.hpp>
#include <stdio.h>

namespace tuner {

TSFilter::TSFilter( TSSectionDemuxer *demux, TSDemuxer *tsDemuxer, const OnFilterCallback &callback )
{
	_demux     = demux;
	_tsDemuxer = tsDemuxer;
	_psiFilter = NULL;
	_onFilter = callback;
}

TSFilter::~TSFilter( void )
{
	delete _demux;
}

bool TSFilter::initialized() const {
	return _psiFilter ? true : false;
}

bool TSFilter::initialize( void ) {
	//	printf( "[TSFilter] Initialize\n" );
	//	Create PSIFilter
	_psiFilter = new PSIFilter( _demux->pid(), _onFilter );
	return true;
}

void TSFilter::deinitialize( void ) {
	//printf( "[TSFilter] deinitialize\n" );
	delete _psiFilter;
	_psiFilter = NULL;
}

bool TSFilter::start( void ) {	
	//	printf( "[TSFilter] start\n" );
	return _tsDemuxer->startFilter( _psiFilter );
}

void TSFilter::stop( void ) {
	//	printf( "[TSFilter] stop\n" );
	_tsDemuxer->stopFilter( _psiFilter );
}

FilterDemuxer *TSFilter::demux() const {
	return _demux;
}

void TSFilter::onSection( BYTE *section, SIZE_T len ) {
	if (!_onFilter.empty()) {
		_onFilter( _demux->pid(), section, len );
	}
}

}
