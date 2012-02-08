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
#include "provider.h"
#include "filter.h"
#include "taskqueue.h"
#include "../demuxer/tssectiondemuxer.h"
#include "../demuxer/psi/psi.h"
#include <util/resourcepool.h>
#include <util/buffer.h>
#include <util/mcr.h>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

namespace tuner {

struct FilterFinder {
	ID _pid;
	FilterFinder( ID pid ) : _pid( pid ) {}
	bool operator()( const FilterPtr &filter ) const {
		return filter->pid() == _pid;
	}
};

inline std::list<FilterPtr>::iterator findByPid( std::list<FilterPtr> &list, ID pid ) {
	return std::find_if( list.begin(), list.end(), FilterFinder(pid) );
}

Provider::Provider( void )
{
	_status   = status::off;
	_needStop = false;
}

Provider::~Provider()
{
	assert(_status < status::idle);
}

bool Provider::start( DWORD networkBlocks/*=1000*/ ) {
	bool result;
	printf( "[Provider] Start\n" );

	//	Start filter engine
	result=startEngineFilter();
	if (result) {
		//	Create network resource pool
		_pool = new util::pool::ResourcePool( resources::provider, networkBlocks, TSS_PRI_MAX_BYTES );

		//	Start demuxer thread
		_needStop = false;
		_thread = boost::thread( boost::bind( &Provider::providerThread, this ) );

		status( status::idle );		
	}
	else {
		status( status::fail );
	}
	
	return result;
}

void Provider::stop( void ) {
	if (status() >= status::idle) {
		printf( "[Provider] Stop\n" );
		
		//	Stop current network
		stopCurrent();
	
		//	Wait section demuxer thread
		_mutex.lock();
		_needStop = true;
		_mutex.unlock();
		_cWakeup.notify_all();
		_thread.join();
		//	All tasks stopped!!!!
		printf( "[Provider] Join to thread successfully\n" );

		//	Stop/cleanup lost filters
		_mFilters.lock();
		BOOST_FOREACH( FilterPtr filter, _filters ) {
		 	printf( "[Provider] Warning, filter lost: pid=%04x\n", filter->pid() );
		 	stopFilter( filter );
		}
		_filters.clear();
		_mFilters.unlock();		
	
		//	Stop filter engine
		stopEngineFilter();

		delete _pool;
	}

	//	Set status always to off
	status( status::off );	
}

status::type Provider::status( void ) {
	status::type res;

	_mutex.lock();
	res = _status;
	_mutex.unlock();
	
	return res;
}

void Provider::status( status::type newState ) {
	_mutex.lock();
	_status = newState;
	_mutex.unlock();
}

//	Tasks methods
void Provider::enqueue( Task *task ) {
	bool notify=false;

	assert(task);
	
	_mutex.lock();
	if (_status <= tuner::status::fail) {
		printf( "[Provider] Warning, task enqueued when provider is not running: task=%p\n", task );
	}
	else {
		printf( "[Provider] enqueue task: task=%p, queue size=%d\n", task, _tasks.size() );		
		_tasks.push_back( task );
		notify=true;
	}
	_mutex.unlock();

	if (notify) {
		_cWakeup.notify_all();
	}
}

void Provider::runTask( Task *task ) {
	printf( "[Provider] run task: task=%p\n", task );
	task->run();
	delete task;
}

void Provider::runTasks( void ) {
	//	Copy tasks to private list, so tasks can be running without lock!
	bool exit=false;
	while (!exit) {
		_mutex.lock();
		std::list<Task *> toRun = _tasks;
		_tasks.clear();
		_mutex.unlock();

		BOOST_FOREACH( Task *task, toRun ) {
			runTask( task );
		}

		exit = (toRun.size() == 0);
	}
}

//	Section filters
bool Provider::startFilter( TSSectionDemuxer *demux ) {
	assert(demux);

	printf( "[Provider] Start filter: demux=%p, pid=%04x, timeout=%ld\n", demux, demux->pid(), demux->timeout() );

	//	Check if can alloc a new filter
	bool result = canAllocFilter( demux->pid() );
	if (result) {
		//	Create filter
		Filter *filter = createFilter( demux );
		if (filter) {
			FilterPtr ptr(filter);
			result=startFilter( ptr );
		}
		else {
			printf( "[Provider] Warning, cannot create filter: pid=%04x\n", demux->pid() );
			delete demux;
			result=false;
		}
	}
	else {
		printf( "[Provider] Warning, cannot alloc a new filter, maximum already allocated or pid already filtrated: pid=%04x\n", demux->pid() );
		delete demux;
	}
	
	return result;
}

void Provider::stopFilter( ID pid, bool destroy/*=true*/ ) {
	if (pid != TS_PID_NULL) {
		printf( "[Provider] Stop filter: pid=%04x\n", pid );

		//	Remove filter from active list
		FilterPtr filter;
		if (removeFilter( filter, pid )) {
			stopFilter( filter, destroy );
		}
	}
}

//	Aux filters engine
bool Provider::startEngineFilter( void ) {
	printf( "[Provider] Start engine filter\n" );
	return true;
}

void Provider::stopEngineFilter( void ) {
	printf( "[Provider] Stop engine filter\n" );
}

//	Set current network
bool Provider::setNetwork( const NetworkType &network ) {
	bool result=false;
	
	if (status() >= status::idle) {
		printf( "[Provider] Set current: %s\n", network.c_str() );

		//	Stop current network
		stopCurrent();

		//	Start network
		result=startNetwork( network );
	
		//	If ok, set status to tuned else to idle
		status( result ? tuner::status::tuned : tuner::status::idle );
	}
	
	return result;
}

//	Scan networks
bool Provider::firstNetworkScan( NetworkType &network ) {
	//	Stop current network
	stopCurrent();

	//	Tune the first scanned network
	bool result=firstNetwork( network );

	//	If ok, set status to tuned else to idle	
	status( result ? tuner::status::scanning : tuner::status::idle );	

	return result;
}

bool Provider::nextNetworkScan( NetworkType &network ) {
	//	Stop current network	
	stopCurrent();

	//	Change to next network
	bool result=nextNetwork( network );

	//	If ok, set status to tuned else to idle	
	status( result ? tuner::status::scanning : tuner::status::idle );	

	return result;
}

//	Stop current network
void Provider::stopCurrent( void ) {
	if (status() > tuner::status::idle) {
		printf( "[Provider] stop current network\n" );

		//	Set status to idle (no accept more sections or tasks)
		status( tuner::status::idle );

		//	Stop network
		stopNetwork();

		//	Clean all data of sections
		cleanup( TS_PID_NULL );
	}
}

//	Aux filters
bool Provider::startFilter( FilterPtr &filter ) {
	bool result=true;
	
	//	Initialize filter
	if (!filter->initialized()) {
		result=filter->initialize();
	}

	if (result) {
		//	Start filter
		result=filter->start();
		if (result) {
			//	Add to list of filters
			_mFilters.lock();
			_filters.push_back( filter );
			_mFilters.unlock();
		}
		else {
			filter->deinitialize();
			printf( "[Provider] Warning, filter cannot be started: pid=%04x\n", filter->pid() );
		}
	}
	else {
		printf( "[Provider] Warning, filter initialization fail: filter=%04x\n", filter->pid() );
	}

	return result;
}

void Provider::stopFilter( FilterPtr &filter, bool destroy/*=true*/ ) {
	//	Stop filter
	filter->stop();

	//	Destroy filter
	if (destroy) {
		filter->deinitialize();
		filter.reset();
	}
}

bool Provider::removeFilter( FilterPtr &filter, ID pid ) {
	bool found=false;
	//printf( "[Provider] Remove filter: pid=%04x\n", pid );	
	
	_mFilters.lock();
	std::list<FilterPtr>::iterator it = findByPid(_filters,pid);
	if (it != _filters.end()) {
		found = true;

		//	Get filter
		filter = (*it);

		//	Remove from list
		_filters.erase( it );
		
		//	Cleanup filter associated data
		cleanup( filter->pid() );
	}
	_mFilters.unlock();
	
	return found;
}

bool Provider::getFilter( FilterPtr &filter, ID pid ) {
	bool result=false;

	_mFilters.lock_shared();
	std::list<FilterPtr>::const_iterator it = findByPid( _filters, pid );
	if (it != _filters.end()) {
		filter = (*it);
		result=true;
	}
	_mFilters.unlock_shared();
	
	return result;
}

bool Provider::canAllocFilter( ID pid ) {
	bool result;
	
	_mFilters.lock_shared();
	result = _filters.size() < static_cast<size_t>(maxFilters());
	if (result) {
		std::list<FilterPtr>::const_iterator it = findByPid( _filters, pid );
		result = (it == _filters.end());
	}
	_mFilters.unlock_shared();
	
	return result;
}

void Provider::timeout( ID pid ) {
	enqueue( pid, NULL );
}

bool Provider::enqueue( ID pid, util::BYTE *data, DWORD size ) {
	assert(data);
	util::Buffer *buf = getNetworkBuffer();
	buf->copy( (char *)data, size );
	return enqueue( pid, buf );	
}

bool Provider::enqueue( ID pid, util::Buffer *buf ) {
	bool notify=false;

	_mutex.lock();

	//	Only enqueue if network is tunned or scanning AND fiter is valid
	if (_status > tuner::status::idle) {
		//	Enqueue filter data into provider thread
		FilterData data;
		data.pid = pid;
		data.buf = buf;
		_filtersData.push_back( data );
		notify=true;
		//printf( "[Provider] enqueue filter: pid=%04x, count=%d\n", pid, _filtersData.size() );
	}
	else if (buf) {
		printf( "[Provider] Warning, ignoring section. Filter not active or provider idle: pid=%04x\n", pid );
		
		//	Returns network buffer
		freeNetworkBuffer( buf );
	}
	
	_mutex.unlock();

	if (notify) {
		_cWakeup.notify_all();
	}
	
	return notify;
}

void Provider::cleanup( ID pid ) {
	std::vector<util::Buffer *> toFree;

	_mutex.lock();
	size_t queueSize = _filtersData.size();
	std::list<FilterData>::iterator it = _filtersData.begin();
	while (it != _filtersData.end()) {
		const FilterData &item = (*it);

		if (pid == TS_PID_NULL || item.pid == pid) {
			if (item.buf) {
				//	Add buffer to free later
				toFree.push_back( item.buf );
			}
			
			//	Remove item from list
			it = _filtersData.erase( it );
		}
		else {
			it++;
		}
	}
	_mutex.unlock();

	printf( "[Provider] clean sections data: pid=%04x, toFree=%d, queue=%d\n",
		pid, toFree.size(), queueSize );

	//	Free all buffers
	_pool->free( toFree );
}

//	Aux network buffer
util::Buffer *Provider::getNetworkBuffer() {
	return _pool->alloc();
}

void Provider::freeNetworkBuffer( util::Buffer *buf ) {
	_pool->free( buf );
}

//	Aux thread
Provider::WaitResult Provider::wait( Task **task, FilterProcess &toProcess ) {
	WaitResult result=wFail;

	//	Wait for callbacks or exit
	boost::unique_lock<boost::mutex> lock( _mutex );
	while (result == wFail) {
		//	Wait a event ...
		while (_filtersData.empty() && _tasks.empty() && !_needStop) {
			_cWakeup.wait( lock );
		}
	
		if (_needStop) {
			result = wExit;
		}
		else if (!_tasks.empty()) {
			*task = _tasks.front();
			_tasks.pop_front();
			result = wTask;		
		}
		else {
			FilterData data;
			result=wFail;
			while (!_filtersData.empty()) {
				//	Get data
				data = _filtersData.front();
				_filtersData.pop_front();

				//	Data is from a valid filter?
				if (getFilter( toProcess.filter, data.pid )) {
					toProcess.buf = data.buf;
					result = wFilter;
					break;
				}
				else {
					//	Filter not active
					freeNetworkBuffer( data.buf );
				}
			}
		}
	}

	// printf( "[Provider] wait return: filters=%d, tasks=%d, exit=%d, result=%d\n",
	// 	_filtersData.size(), _tasks.size(), _needStop, result );

	return result;
}

void Provider::providerThread( void ) {
	bool exit=false;
	FilterProcess process;
	Task *task;
	WaitResult result;
	
	while (!exit) {
		//	Get callback to call, task to execute or exit from thread
		result = wait( &task, process );
		switch (result) {
			case wExit: {
				exit = true;
				//	Run all tasks when exit
				runTasks();
				break;
			}
			case wFilter: {
				//	Parser buffer
				process.filter->process( process.buf );
				if (process.buf) {
					freeNetworkBuffer( process.buf );
				}
				process.filter.reset();
				break;
			}
			case wTask:
				runTask( task );
				break;
			default:
				break;
		}
	}

	printf( "[Provider] End provider thread: result=%d\n", result );	
}

}

