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

#include "taskitem.h"

namespace util {
namespace spawner {

TaskItem::TaskItem( void )
{
	_priority = 1;
	_pid = -1;
}

TaskItem::TaskItem( const Params &params, const Params &env )
	: _params( params ), _env( env )
{
	_priority = 1;
	_pid = -1;
}

TaskItem::~TaskItem( void )
{
	assert( _pid == -1 );
}

//	Methods
void TaskItem::queue() {
	if (!_queue.empty()) {
		_queue();
	}
}

void TaskItem::start() {
	if (!_start.empty()) {
		_start();
	}	
}

void TaskItem::setParams( const Params &params ) {
	_params = params;
}

void TaskItem::setEnvirioment( const Params &env ) {
	_env = env;
}

//	Getters
int TaskItem::priority( void ) const {
	return _priority;
}

int TaskItem::pid() const {
	return _pid;
}

const Params &TaskItem::params() const {
	return _params;
}

const Params &TaskItem::env() const {
	return _env;
}

//	Setters
void TaskItem::pid( pid_t p ) {
	_pid = p;
}

void TaskItem::priority( int pri ) {
	_priority = pri;
}

void TaskItem::onStart( const Callback &start ) {
	_start = start;
}

void TaskItem::onQueue( const Callback &queue ) {
	_queue = queue;
}
	
}
}


