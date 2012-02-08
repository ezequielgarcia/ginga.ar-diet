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

#include "../../../application.h"

namespace util {
namespace spawner {
	class TaskItem;
}
}

namespace connector {
	class Connector;
	class AVDescriptor;
	struct KeyEventStruct;
	typedef struct KeyEventStruct KeyEventData;
}

namespace tuner {
namespace app {

class NCLApplication : public Application {
public:
	NCLApplication( ApplicationExtension *ext, const ApplicationID &id, const std::string &script );
	virtual ~NCLApplication( void );

	virtual void show() const;	
	
	const std::string &script() const;

protected:
	virtual status::type doStart( void );
	virtual void doStop( bool kill=false );

	//	Status changed
	virtual void onStatusChanged( status::type /*old*/, status::type actual );	

	//	Aux Editing command
	void registerEditingCommands( void );
	void onEditingCommand( const Buffer &buf );	

	//	Aux
	const std::string ginga() const;
	void setupProcess( void );
	void onKeyEvent( connector::KeyEventData *data );
	void onResizeVideoEvent( connector::AVDescriptor *av );
	void onTextToSpeechHandler(std::string speech, bool queue=false, unsigned int delay=0); 
	void onStopTextToSpeechHandler(); 
	void onAudioOnHandler();
	void onAudioOffHandler();

private:
	std::string _script;
	connector::Connector *_connector;
	util::spawner::TaskItem *_proc;	
};

}
}
