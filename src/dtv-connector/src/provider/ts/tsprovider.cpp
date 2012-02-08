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
#include "tsprovider.h"
#include "data/tsdataprovider.h"
#include "../../demuxer/ts/tsdemuxer.h"
#include "tsfilter.h"
#include <util/resourcepool.h>
#include <util/buffer.h>
#include <boost/foreach.hpp>
#include <stdio.h>

namespace tuner {

TSProvider::TSProvider( TSDataProvider *dataProvider )
{
	_pool = new util::pool::ResourcePool( resources::tsdemuxer, 1000, dataProvider->bufferSize() );
	_ts = dataProvider;
	_tsDemuxer = new TSDemuxer();
}

TSProvider::~TSProvider()
{
	delete _ts;
    delete _tsDemuxer;
	delete _pool;
}

//	Reset demuxer
void TSProvider::reset() {
	_mutex.lock();
	cleanupData();
	_tsDemuxer->reset();
	_mutex.unlock();
}

//	Enqueue bytes to process
void TSProvider::process( util::Buffer *buf ) {
	if (status() > tuner::status::idle) {
		_mutex.lock();
		_data.push_back( buf );
		_mutex.unlock();
		_cWakeup.notify_all();
	}
	else {
		printf( "[TSProvider] Warning, discard buffer\n" );
		//	Discard buffer, provider not accept more buffers
		freeBuffer( buf );
	}
}

//	Set current network
bool TSProvider::startNetwork( const NetworkType &net ) {
	printf( "[TSProvider] Start network\n" );
	reset();	
	return _ts->startNetwork( net );
}

void TSProvider::stopNetwork( void ) {
	printf( "[TSProvider] Stop network\n" );
	reset();
	return _ts->stopNetwork();	
}

bool TSProvider::firstNetwork( NetworkType &network ) {
	printf( "[TSProvider] First network\n" );	
	return _ts->firstNetwork( network );
}

bool TSProvider::nextNetwork( NetworkType &network ) {
	printf( "[TSProvider] Next network\n" );		
	return _ts->nextNetwork( network );	
}

//	Aux filters engine
bool TSProvider::startEngineFilter( void ) {
	printf( "[TSProvider] Start engine filter\n" );

	if (_ts->initialize( this )) {
		//	Start demuxer thread
		_needStop = false;
		_thread = boost::thread( boost::bind( &TSProvider::tsParser, this ) );
		return true;
	}
	return false;
}

void TSProvider::stopEngineFilter( void ) {
	printf( "[TSProvider] Stop engine filter\n" );
	
	//	Signal thread to exit
	_mutex.lock();
	_needStop = true;
	_mutex.unlock();
	_cWakeup.notify_all();

	//	Join
	_thread.join();

	//	Deinitialize ts data provider
	_ts->deinitialize();

	//	Free all data enqueued
	cleanupData();
}

void TSProvider::onFilterCallback( ID pid, BYTE *buf, SIZE_T len ) {
	enqueue( pid, buf, len );
}

//	Aux filters
Filter *TSProvider::createFilter( TSSectionDemuxer *sectionDemux ) {
	return new TSFilter( sectionDemux, _tsDemuxer, boost::bind(&TSProvider::onFilterCallback,this,_1,_2,_3) );
}

int TSProvider::maxFilters() const {
	return 100;
}

//	Buffer handling
util::Buffer *TSProvider::allocBuffer() {
	return _pool->alloc();
}

void TSProvider::freeBuffer( util::Buffer *buf ) {
	_pool->free( buf );
}

//	Data aux
void TSProvider::cleanupData( void ) {
	printf( "[TSProvider] Clean ts data\n" );
	
	BOOST_FOREACH( util::Buffer *buf, _data ) {
		freeBuffer( buf );
	}
	_data.clear();
}

util::Buffer *TSProvider::waitData( void ) {
	util::Buffer *data=NULL;	
	
	//	Wait for callbacks or exit
	boost::unique_lock<boost::mutex> lock( _mutex );
	while (_data.empty() && !_needStop) {
		_cWakeup.wait( lock );
	}

	if (!_needStop && !_data.empty()) {
		data = _data.front();
		_data.pop_front();
	}
	
	return data;
}

void TSProvider::tsParser( void ) {
	bool exit=false;
	util::Buffer *buf=NULL;
	
	printf( "[TSProvider] Begin parser thread\n" );

	while (!exit) {
		buf = waitData();
		if (buf) {
			//	Parse buffer
			_tsDemuxer->parse( buf );

			//	Returns buffer to pool
			freeBuffer( buf );
		}
		else {
			exit=true;
		}
	}
	printf( "[TSProvider] End parser thread\n" );	
}

}

