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
#pragma once

#include "../types.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <set>

namespace util {
	class Buffer;

	namespace pool {
		class ResourcePool;
	}
}

namespace tuner {

class Task;
class Filter;
typedef boost::shared_ptr<Filter> FilterPtr;
class TSSectionDemuxer;

class Provider
{
public:
	Provider( void );
	virtual ~Provider();

	//	Start/Stop provider
	bool start( DWORD networkBlocks=1000 );
	void stop();
	status::type status( void );

	//	Set current network
	bool setNetwork( const NetworkType &network );

	//	Scan networks
	bool firstNetworkScan( NetworkType &network );
	bool nextNetworkScan( NetworkType &network );

	//	Enqueue task into provider thread
	void enqueue( Task *task );
	
	//	Start Section Filters
	bool startFilter( TSSectionDemuxer *demux );

	//	Start PES filters
	virtual bool startPesFilter( ID /*pid*/ ) { return true; };
	virtual std::string streamPipe( void ) const { return ""; }	

	//	Stop filters (any filter)
	void stopFilter( ID pid, bool destroy=true );

protected:
	//	Types
	enum WaitResult { wFail, wExit, wFilter, wTask };
	typedef struct {
		util::Buffer *buf;
		ID pid;
	} FilterData;
	typedef struct {
		util::Buffer *buf;
		FilterPtr filter;
	} FilterProcess;
	
	//	Getters/Setters
	void status( status::type newState );

	//	Network methods
	void stopCurrent( void );	
	virtual bool startNetwork( const NetworkType &net )=0;
	virtual void stopNetwork( void )=0;
	virtual bool firstNetwork( NetworkType &network )=0;
	virtual bool nextNetwork( NetworkType &network )=0;

	//	Aux filters engine
	virtual bool startEngineFilter( void );
	virtual void stopEngineFilter( void );

	//	Aux filters
	virtual Filter *createFilter( TSSectionDemuxer *sectionDemux )=0;
	virtual bool startFilter( FilterPtr &filter );
	virtual void stopFilter( FilterPtr &filter, bool destroy=true );	
	virtual int maxFilters() const=0;
	bool removeFilter( FilterPtr &filter, ID pid );
	bool canAllocFilter( ID pid );
	bool getFilter( FilterPtr &filter, ID pid );

	//	Filter data
	void timeout( ID pid );
	bool enqueue( ID pid, util::BYTE *data, DWORD size );	
	bool enqueue( ID pid, util::Buffer *buf );
	void cleanup( ID pid );

	//	Aux Network buffer
	util::Buffer *getNetworkBuffer();
	void freeNetworkBuffer( util::Buffer *buf );

	//	Aux task
	void runTask( Task *task );
	void runTasks( void );

	//	Aux thread
	void providerThread( void );
	WaitResult wait( Task **task, FilterProcess &callback );
	
private:
	status::type _status;
	boost::shared_mutex  _mFilters;	
	std::list<FilterPtr> _filters;
	std::list<FilterData> _filtersData;
	std::list<Task *> _tasks;
	boost::condition_variable _cWakeup;
	boost::mutex _mutex;
	boost::thread _thread;
	bool _needStop;
    util::pool::ResourcePool *_pool;
};

}
