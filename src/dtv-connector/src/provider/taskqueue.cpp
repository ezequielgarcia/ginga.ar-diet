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
#include "taskqueue.h"
#include <boost/foreach.hpp>
#include <stdio.h>

namespace tuner {

TaskQueue::TaskQueue( void )
{
	_exit = false;
}

TaskQueue::~TaskQueue( void )
{
	clear();
}

void TaskQueue::clear( void ) {
	Task *task;
	_mutex.lock();
	while (!_tasks.empty()) {
		task = _tasks.front();
		//printf( "[TaskQueue %p] clear task=%p, empty=%d\n", this, task, _tasks.empty() );
		delete task;
		_tasks.pop();
	}
	_mutex.unlock();
}

void TaskQueue::push( Task *task ) {
	assert(task);

	//printf( "[TaskQueue %p] push task=%p\n", this, task );
	
	_mutex.lock();
	if (!_exit) {
		_tasks.push( task );
	}
	else {
		delete task;
	}
	_mutex.unlock();

	//	Wakeup thread
	_cond.notify_all();
}

bool TaskQueue::start( void ) {
	//printf( "[TaskQueue %p] start\n", this );
	
	_thread = boost::thread( boost::bind( &TaskQueue::loop, this, 0 ) );
	return true;
}

void TaskQueue::stop( void ) {
	//printf( "[TaskQueue %p] stop\n", this );

	_mutex.lock();
	_exit = true;
	_mutex.unlock();

   	//	Wakeup thread
	_cond.notify_all();

	_thread.join();
}

bool TaskQueue::getTask( const boost::system_time &timeout, std::vector<Task *> &tasks ) {
	Task *task;
	bool exit=true;
	
	boost::unique_lock<boost::mutex> lock( _mutex );
	if (!_exit) {
		if (!timeout.is_infinity()) {
			//	Wait for task or timeout
			exit = timeout < boost::get_system_time() ? true : !_cond.timed_wait( lock, timeout );
		}
		else {
			//	Wait only for task
			exit = false;
			_cond.wait( lock );
		}

		//	Get tasks to run
		while (!_tasks.empty()) {
			task = _tasks.front();
			_tasks.pop();
			tasks.push_back( task );
		}
	}
	
	return exit;
}

void TaskQueue::loop( util::DWORD msTimeout ) {
	bool exit;
	std::vector<Task *> tasks;
	
	//	Initialize
	_mutex.lock();
	exit = _exit = false;
	_mutex.unlock();

	//	Calculate the timeout
	boost::system_time timeout = msTimeout ? boost::get_system_time()+boost::posix_time::milliseconds(msTimeout) : boost::posix_time::pos_infin;
	
	//	Wait for task, exit or timeout
    while (!exit) {
		//printf( "[TaskQueue %p] get task or timeout of %ldms\n", this, msTimeout );
		exit = getTask( timeout, tasks );
		
		//	Run tasks
		BOOST_FOREACH( Task *task, tasks ) {
			//printf( "[TaskQueue %p] run task=%p\n", this, task );				
			task->run();
			delete task;
		}
		tasks.clear();
	}

	clear();
	printf( "[TaskQueue %p] exit loop\n", this );	
}

}
