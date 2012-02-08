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

#include "spawner.h"
#include "taskitem.h"
#include <util/mcr.h>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

namespace util {
namespace spawner {

struct Spawner::MaxTaskFinder {
	MaxTaskFinder() : _item( NULL ) { _pri = 0; }
	void operator()( TaskItem *item ) {
		if (item->pid() < 0 && item->priority() > _pri) {
			_item = item;
			_pri  = item->priority();
		}
	}
	TaskItem *_item;
	int _pri;
};

struct Spawner::MinTaskFinder {
	MinTaskFinder( int priority ) : _item( NULL ) { _pri = priority; }
	void operator()( TaskItem *item ) {
		if (item->pid() >= 0 && item->priority() < _pri) {
			_item = item;
			_pri  = item->priority();
		}
	}
	TaskItem *_item;
	int _pri;
};

Spawner::Spawner( int maxParallel )
{
	_enable = true;
	_maxParallel = maxParallel;
}

Spawner::~Spawner( void )
{
	CLEAN_ALL( TaskItem *, _tasks );
}

//	Spawner methods
status::type Spawner::run( TaskItem *item ) {
	assert( item );

	//	Try start
	status::type st=exec( item );
	if (st > status::failed) {
		//	Add item into tasks
		_tasks.push_back( item );
	}
	else {
		delete item;
	}
	
	return st;
}

void Spawner::stop( TaskItem *item, int msTimeout/*=0*/ ) {
	Tasks::iterator it=std::find( _tasks.begin(), _tasks.end(), item );
	if (it != _tasks.end()) {
		//	Stop item
		kill( (*it), msTimeout );

		//	Remove task from list
		delete (*it);
		_tasks.erase( it );

		//	Try run queued
		runQueued();
	}
}

void Spawner::enable( bool mustEnable ) {
	if (_enable != mustEnable) {
		printf( "[util::Spawner] Enable: old=%d, new=%d\n", _enable, mustEnable );
		
		_enable = mustEnable;
		
		if (_enable) {
			//	Spawn pending process
			for (int i=0; i<static_cast<int>(_tasks.size()); i++) {
				if (!runQueued()) {
					break;
				}
			}
		}
		else {
			//	Stop all process and enqueue
			BOOST_FOREACH( TaskItem *item, _tasks ) {
				if (item->pid() >= 0) {
					stopAndQueue(item);
				}
			}			
		}
	}
}

void Spawner::stopAndQueue( TaskItem *item ) {
	item->queue();
	
	//	Kill task!
	kill( item, 0 );
}

int Spawner::maxParallel() const {
	return _maxParallel;
}

int Spawner::running() const {
	int res=0;
	BOOST_FOREACH( TaskItem *item, _tasks ) {
		if (item->pid() >= 0) {
			res++;
		}
	}
	return res;
}

bool Spawner::canRun( int priority ) {
	bool result=false;

	if (_enable) {
		if (running() < maxParallel()) {
			result=true;
		}
		else {
			//	Check if some task running has low priority
			MinTaskFinder min(priority);
			if (find( min )) {
				stopAndQueue( min._item );
				result=true;
			}
		}
	}
	return result;
}

status::type Spawner::exec( TaskItem *item ) {
	status::type st=status::failed;
	if (canRun( item->priority() )) {
		//	Try fork process
		if (spawn( item )) {
			st = status::started;
		}
	}
	else {
		//	Enqueue process
		st=status::queued;
	}

	return st;
}

char **Spawner::makeParams( const Params &params, int size/*=-1*/ ) {
	char **cmd;

	if (size < 0) {
		size = params.size();
	}
	cmd = (char **)calloc( size+1, sizeof(char *) );

	int i;
	for (i=0; i<size; i++) {
		cmd[i] = strdup( params[i].c_str() );
		printf("[Spawner] makeParams[%d] = %s\n", i, cmd[i]);
	}
	cmd[i] = NULL;
	
	return cmd;
}

void Spawner::freeParams( char **params ) {
	int i=0;
	if (params) {
		for (i=0; params[i] != NULL; i++) {
			printf( "[Spawner] Free param[%d] = %s\n", i, params[i] );
			free( params[i] );
		}
		free( params );
	}
}
	
bool Spawner::spawn( TaskItem *item ) {
	bool result;
    pid_t pid;

	//	Call to aux start method
	item->start();

	assert( item->params().size() > 0 );

/*
 MICROTROL: 
 Reimplementado para utilizar posix_spawn() en vez de
 fork()/exec(). Esto se debe a que el proceso ginga se lanza desde
 un proceso con un espacio de direcciones __muy amplio__ (zapper)
 y por lo tanto es preferible forkear con vfork (ver implementacion
 de posix_spawn). 
*/
#if 1
    int status;
	char **argv = makeParams( item->params() );
	char **env = (item->env().size() > 0) ? makeParams( item->env() ) : NULL;

    // Spawning, env can be NULL
    status = posix_spawn(&pid, argv[0], NULL, NULL, argv, env);
	if (status == 0) {
		printf( "[Spawner] Fork process ok: pid=%d\n", pid );
		result=true;
	}
	else { // failed to fork
		printf( "[Spawner] Error, cannot fork process\n" );
		result=false;
	}

	freeParams(argv);
	freeParams(env);

#else
	//	Fork process
	pid = ::fork();
	if (pid == 0) {	// child
		//	Close all descriptors!!!
		int maxfd=::sysconf(_SC_OPEN_MAX);
		for(int fd=3; fd<maxfd; fd++) {
			close(fd);
		}

		//	Make parameters (before fork!!!!)
		char **argv = makeParams( item->params() );
		char **env = (item->env().size() > 0) ? makeParams( item->env() ) : NULL;

		//	Replace process
		if (env) {
			result=::execve( argv[0], argv, env ) >= 0;
		}
		else {
			result=::execv( argv[0], argv ) >= 0;
		}

		if (!result) {
			printf( "[Spawner] Error, cannot execute process: cmd=%s\n", argv[0] );
			exit(-1);
		}
		printf( "[Spawner] Running process: cmd=%s\n", argv[0] );
	}
	else if (pid < 0) { // failed to fork
		printf( "[Spawner] Error, cannot fork process\n" );
		result=false;
	}
	else {
		printf( "[Spawner] Fork process ok: pid=%d\n", pid );
		result=true;
	}
#endif

	//	Setup pid
	item->pid( pid );

	return result;
}

bool Spawner::runQueued( void ) {
	bool result=false;
	//printf( "[Spawner] Run queued process begin: size=%d\n", _tasks.size() );
	
	//	Get task with maximum priority
	MaxTaskFinder max;
	if (find( max )) {
		//	Run item
		result=exec( max._item ) == status::started;
		printf( "[Spawner] Queued: pid=%d\n", max._item->pid() );
	}
	//printf( "[Spawner] Run queued process end\n" );
	return result;
}

void Spawner::kill( TaskItem *item, int msTimeout ) {
	printf( "[Spawner] Stop process: pid=%d, timeout=%d\n", item->pid(), msTimeout );

	pid_t pid = item->pid();
	if (pid >= 0) {
		bool mustKill=true;
		
		if (msTimeout) {
			printf( "[Spawner] Wait for process: pid=%d\n", pid );			
			boost::this_thread::sleep( boost::posix_time::milliseconds(msTimeout) );
			if (::waitpid( pid, NULL, WNOHANG ) == pid) {
				mustKill=false;
			}
		}

		if (mustKill) {
			printf( "[Spawner] Kill process: pid=%d\n", pid );
			
			//	Send kill
			::kill( pid, SIGKILL );
		}

		//	Wait to stop
		::waitpid( pid, NULL, 0 );
		item->pid( -1 );

		printf( "[Spawner] Kill process successfully: pid=%d\n", pid );		
	}
}

}
}

