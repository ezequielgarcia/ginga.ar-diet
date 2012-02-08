/*******************************************************************************

  Copyright (C) 2011 Fernando Vasconcelos - MICROTROL - Rosario, Argentina
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
#include "nclapplication.h"
#include "../../../controller/applicationcontroller.h"
#include "../../../controller/spawner.h"
#include "../../../controller/spawner/taskitem.h"
#include "../../../applicationextension.h"
#include "../../../../../servicemanager.h"
#include "../../../../../../resourcemanager.h"
#include "../../../../../../demuxer/psi/dsmcc/event.h"
#include "../../../../../../provider/taskqueue.h"
#include "../../../../../../connector/connector.h"
#include "../../../../../../connector/handler/keepalivehandler.h"
#include "../../../../../../connector/handler/keyeventhandler.h"
#include "../../../../../../connector/handler/videoresizehandler.h"
#include "../../../../../../connector/handler/exitcommandhandler.h"
#include "../../../../../../connector/handler/editingcommandhandler.h"
#include "../../../../../../connector/handler/ttshandler.h"
#include "generated/config.h"
#include <util/string.h>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <set>
#include <assert.h>
#include <stdio.h>

namespace tuner {
namespace app {

namespace SpawnerStatus = util::spawner::status;

NCLApplication::NCLApplication( ApplicationExtension *ext, const ApplicationID &id, const std::string &script )
	: Application( ext, id ), _script( script )
{
	name( script );
	_connector = NULL;
	_proc = NULL;
}

NCLApplication::~NCLApplication( void )
{
	assert(!_connector);
	assert(!_proc);
}

void NCLApplication::show() const {
	Application::show();
	printf( "[NCLApplication]    \tscript=%s\n", _script.c_str() );
}

const std::string &NCLApplication::script() const {
	return _script;
}

const std::string NCLApplication::ginga() const {
	char *ptr = getenv( "GINGA_PROCESS" );
	return (ptr) ? ptr : GINGA_DEFAULT_PROCESS;	
}

status::type NCLApplication::doStart( void ) {
	//	Setup item to spawn
	_proc = new util::spawner::TaskItem();
	_proc->priority( priority() );
	_proc->onStart( boost::bind( &NCLApplication::setupProcess, this ) );
	_proc->onQueue( boost::bind( &Application::status, this, status::waiting ) );

	//	Run ginga
	SpawnerStatus::type st=ctrl()->spawner()->run( _proc );
	if (st == SpawnerStatus::failed) {
		_proc = NULL;
		return status();
	}
	else {
		return (st == SpawnerStatus::queued) ? status::waiting : status::starting;
	}
}

void NCLApplication::doStop( bool kill/*=false*/ ) {
	assert( _proc );
	
	//	Stop ginga
	if (status() == status::started) {

		// MICROTROL
		// Se quita el handle del evento disconnect del conector para que no se encole otro 
		// stop luego de finalizar la aplicacion producto de la desconexion del socketr
		_connector->onDisconnect(NULL);

		connector::ExitCommandHandler exit;
		exit.send( _connector );
	}

	//	Kill process if ginga exit fail
	ctrl()->spawner()->stop( _proc, kill ? 0 : GINGA_TIMEOUT );

	//	Cleanup connector
	delete _connector;
	_connector = NULL;

	_proc = NULL;
}

//	NCL Editing command
void NCLApplication::onEditingCommand( const Buffer &buf ) {
	connector::EditingCommandHandler cmd;
	printf( "[app::NCLApplication] Editing command received: size=%d\n", buf.length() );
	cmd.send( _connector, buf );	
}

struct NCLEventFinder {
	NCLEventFinder( std::vector<std::string> &events ) : _events( events ) {}
	void operator()( dsmcc::Event *evt ) {
		WORD eventID;
		evt->show();
		if (evt->includes( "nclEditingCommand", eventID )) {
			_events.push_back( evt->url() );
		}
	}

	std::vector<std::string> &_events;
};

void NCLApplication::registerEditingCommands( void ) {
	printf( "[app::NCLApplication] Register editing commands\n" );
	
	std::vector<std::string> names;
	NCLEventFinder iter(names);
	loopEvents( iter );
	BOOST_FOREACH( const std::string &name, names ) {
		printf( "[app::NCLApplication] Editing command found: event=%s\n", name.c_str() );
		registerEvent( name, "nclEditingCommand", boost::bind(&NCLApplication::onEditingCommand,this,_1) );
	}
}

//	Connector aux
void NCLApplication::setupProcess( void ) {
	assert( !_connector );
	assert( _proc );

	//	Make pipe
	std::string pipe = extension()->resMgr()->mkTempFileName( "connector_XXXXXX" );
	printf( "[app::NCLApplication] Setup process: pipe = %s\n", pipe.c_str() );
	
	//	Setup params
	util::spawner::Params params;
	params.push_back( ginga() );
	params.push_back( "-r" );

	//	Setup NIT
	ID nitID = extension()->srvMgr()->currentNit();
	if (nitID != NIT_ID_RESERVED) {
		params.push_back( "--baseId" );
		params.push_back( boost::lexical_cast<std::string>( nitID ) );
	}

	{	//	Setup resolution
		int w,h;
		setupVideoResolution( w, h );
		params.push_back( "--set-width" );
		params.push_back( boost::lexical_cast<std::string>(w) );
		params.push_back( "--set-height" );
		params.push_back( boost::lexical_cast<std::string>(h) );
	}

	//	Setup pipe
	params.push_back( "-c" );
	params.push_back( pipe );

	//	Setup NCL name
	std::string ncl = path() + "/" + _script;
	params.push_back( "--ncl" );
	params.push_back( ncl );

	//	Setup envirioment
	util::spawner::Params env;
	const char *ptrEnv = getenv( "GINGA_PROCESS_ENV" );
	if (ptrEnv) {
		env.push_back( ptrEnv );
	}

	//	Set parameters and envirioment to process
	_proc->setParams( params );
	_proc->setEnvirioment( env );

	//	Create connector
	_connector = new connector::Connector( pipe, true );
	_connector->onConnect( boost::bind( &ApplicationController::status, ctrl(), appID(), status::started ) );

	//	Handle error cases
	boost::function<void (void)> fnc = boost::bind( &ApplicationController::stop, ctrl(), appID() );
	_connector->onConnectTimeout( fnc );
	_connector->onDisconnect( fnc );
	_connector->onTimeout( fnc );
	
	//	Reserve/Release keys messages
	connector::KeyEventHandler *keyHandler = new connector::KeyEventHandler();
	keyHandler->onKeyEvent( boost::bind(&NCLApplication::onKeyEvent,this,_1) );
	_connector->addHandler( connector::messages::keyEvent, keyHandler );

	//	Video resize
	connector::VideoResizeHandler *videoResizeHandler = new connector::VideoResizeHandler();
	videoResizeHandler->onVideoResizeEvent( boost::bind(&NCLApplication::onResizeVideoEvent,this,_1) );
	_connector->addHandler( connector::messages::videoResize, videoResizeHandler );
	
	//TextToSpeech
	connector::TextToSpeechHandler *textToSpeechHandler = new connector::TextToSpeechHandler();
	//Registro las cuatro funciones de callback para textToSpeechHandler
	textToSpeechHandler->onTextToSpeechHandler( boost::bind(&NCLApplication::onTextToSpeechHandler,this,_1,_2,_3) );
	textToSpeechHandler->onStopTextToSpeechHandler( boost::bind(&NCLApplication::onStopTextToSpeechHandler,this) );
	textToSpeechHandler->onAudioOnHandler( boost::bind(&NCLApplication::onAudioOnHandler,this) );
	textToSpeechHandler->onAudioOffHandler( boost::bind(&NCLApplication::onAudioOffHandler,this) );
	_connector->addHandler( connector::messages::textToSpeech, textToSpeechHandler );
}

//	Status changes
void NCLApplication::onStatusChanged( status::type /*old*/, status::type actual ) {
	switch (actual) {
		case status::started:
			//	Register to all nclEditingCommand events
			registerEditingCommands();
			break;
		case status::waiting: {
			//	Kill connnector
			delete _connector;
			_connector = NULL;
			break;
		}
		default:
			break;
	};
}

void NCLApplication::onResizeVideoEvent( connector::AVDescriptor *av ) {
	ctrl()->resizeVideo( av->getX(), av->getY(), av->getWidth(), av->getHeight() );
}

void NCLApplication::onKeyEvent( connector::KeyEventData *data ) {
	std::vector<util::key::type> keys;
	BOOST_FOREACH( util::key::type key, data->keys ) {
		keys.push_back( key );
	}
	ctrl()->reserveKeys( keys );
}

void NCLApplication::onTextToSpeechHandler(std::string speech, bool queue, unsigned int delay) {
	ctrl()->speechText(speech, queue, delay);
}

void NCLApplication::onStopTextToSpeechHandler() {
	ctrl()->stopSpeechText();
}

void NCLApplication::onAudioOnHandler() {
	printf("[DEBUG] %s - calling audioOn()\n", __func__);
	ctrl()->audioOn();
}

void NCLApplication::onAudioOffHandler() {
	printf("[DEBUG] %s - calling audioOff()\n", __func__);
	ctrl()->audioOff();
}

}
}
