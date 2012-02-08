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

#include "types.h"
#include <vector>

namespace tuner {

class Service;
class Extension;
class Task;
class ResourceManager;
class TSSectionDemuxer;

class ServiceManager {
public:
	ServiceManager( ResourceManager *resources );
	virtual ~ServiceManager( void );

	//	Extensions
	void addExtension( Extension *extension );

	//	Getters
	ResourceManager *resMgr();
	virtual DWORD currentTS() const=0;
	virtual ID currentNit() const=0;

	//	Filters
	virtual bool startFilter( TSSectionDemuxer *demux )=0;
	virtual void stopFilter( ID pid )=0;
	virtual bool startPesFilter( ID pid )=0;
	virtual std::string streamPipe( void )=0;

	//	Enqueue task into service manager thread
	virtual void enqueue( Task *task )=0;

protected:
	void exStart( bool withExtensions );
	void exStop( void );
	void exReady( bool ready );	
	void exServiceReady( Service *srv );		
	void exServiceExpired( Service *srv );	
	void exServiceStarted( Service *srv );	
	void exServiceStopped( Service *srv );

private:
	bool _enableExtensions;	
	std::vector<Extension *> _extensions;
	ResourceManager *_resMgr;
};

}
