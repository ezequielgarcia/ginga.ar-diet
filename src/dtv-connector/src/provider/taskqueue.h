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

#include <util/types.h>
#include <queue>
#include <boost/thread.hpp>
#include <boost/function.hpp>

namespace tuner {

class Task {
public:
	Task( void ) {}
	virtual ~Task( void ) {}

	virtual void run( void )=0;
};

class BasicTask : public Task {
public:
	BasicTask( boost::function<void (void)> fnc ) { _task = fnc; }
	virtual ~BasicTask( void ) {}

	virtual void run( void ) {
		_task();
	}

private:
	boost::function<void (void)> _task;
};

typedef boost::function<void (Task *)> Dispatcher;

class TaskQueue {
public:
	TaskQueue( void );
	~TaskQueue( void );

	void clear( void );
	bool start( void );
	void stop( void );
	void push( Task *task );
	void loop( util::DWORD msTimeout );

protected:
	bool getTask( const boost::system_time &timeout, std::vector<Task *> &tasks );
	
private:
	//	Tasks
	std::queue<Task *> _tasks;
	bool _exit;
	boost::thread _thread;
	boost::mutex  _mutex;
	boost::condition_variable _cond;
};

}
