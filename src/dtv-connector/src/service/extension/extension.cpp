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
#include "extension.h"
#include "../servicemanager.h"
#include "../../demuxer/psi/psi.h"
#include "../../resourcemanager.h"

namespace tuner {

Extension::Extension( ServiceManager *srvMgr )
{
	_srvMgr = srvMgr;
	_resMgr = NULL;
}

Extension::~Extension( void )
{
	delete _resMgr;
}

//	Getters/Setters
void Extension::resMgr( ResourceManager *resMgr ) {
	delete _resMgr;
	_resMgr = resMgr;
}

ServiceManager *Extension::srvMgr() {
	return _srvMgr;
}

void Extension::start( void ) {
}

void Extension::stop( void ) {
}

//	Notifications
void Extension::onReady( bool ) {}
void Extension::onServiceReady( Service * ) {}
void Extension::onServiceExpired( Service * ) {}
void Extension::onServiceStarted( Service * ) {}
void Extension::onServiceStopped( Service * ) {}

bool Extension::checkTS( ID tsID ) const {
	return _srvMgr->currentTS() == tsID;
}

bool Extension::checkNit( ID nitID ) const {
	ID currentNit = _srvMgr->currentNit();
	return (currentNit == NIT_ID_RESERVED || currentNit == nitID);
}

ResourceManager *Extension::resMgr() {
	return _resMgr ? _resMgr : _srvMgr->resMgr();
}

}
