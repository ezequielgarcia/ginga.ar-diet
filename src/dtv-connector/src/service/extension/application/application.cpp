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
#include "application.h"
#include "applicationextension.h"
#include "controller/applicationcontroller.h"
#include "../../servicemanager.h"
#include "../../../resourcemanager.h"
#include "../../../demuxer/psi/dsmcc/event.h"
#include "../../../demuxer/psi/dsmcc/dsmccdemuxer.h"
#include "../../../demuxer/psi/dsmcc/streameventdemuxer.h"
#include "../../../demuxer/psi/dsmcc/objectcarouseldemuxer.h"
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <assert.h>
#include <stdio.h>

namespace tuner {
namespace app {

namespace fs = boost::filesystem;

struct EventFinder {
	EventFinder( const std::string &url, const std::string &name, WORD &eventID )
		: _url(url), _name(name), _eventID(eventID) {}
	bool operator()( dsmcc::Event *evt ) {
		if (evt->url() == _url && evt->includes( _name, _eventID )) {
			return true;
		}
		return false;
	}
	const std::string &_url;
	const std::string &_name;
	WORD &_eventID;
};

Application::Application( ApplicationExtension *ext, const ApplicationID &id )
	: _appID ( id )
{
	_extension    = ext;
	_autoStart    = false;
	_autoDownload = false;
	_readOnly     = true;
	_service      = 0;
	_visibility   = visibility::none;
	_priority     = 0;
	_componentTag = 0xFF;
	_status       = status::unmounted;
}

Application::Application( ApplicationExtension *ext, const ApplicationID &id, const std::string &path )
	: _appID( id )
{
	//	Filesystem
	_extension    = ext;
	_path         = path;
	_autoStart    = false;
	_autoDownload = false;
	_readOnly     = true;
	_service      = 0;
	_visibility   = visibility::both;
	_priority     = 0;
	_componentTag = 0xFF;
	_status       = status::mounted;
}

Application::~Application( void )
{
	assert( _downloads.size() == 0 );
	assert( _streams.size() == 0 );
	assert( _streamCallbacks.size() == 0 );	
	BOOST_FOREACH( dsmcc::Event *ev, _events ) {
		delete ev;
	}
}

//	Operators
bool Application::operator==( const ApplicationID &id ) {
	return _appID == id;
}

//	Start/Stop
bool Application::start( void ) {
	bool result=false;	
	printf( "[app::Application] Start application: %s\n", name().c_str() );

	//	Application already running
	if (!isRunning()) {
		//	Application ready?
		if (isReady()) {
			status ( doStart() );
		}
		else {
			//	Force start
			autoStart( true );
			
			//	Download application
			result=startDownload();
		}
	}
	else {
		printf( "[Application] Warning, application already started: name=%s, id=%s\n",
			name().c_str(), appID().asString().c_str() );
	}

	return result;
}

void Application::stop( bool kill/*=false*/ ) {
	printf( "[app::Application] Stop application: name=%s, kill=%d\n", name().c_str(), kill );

	//	Stop any download
	stopDownload();
		
	//	Application is running ...
	if (isRunning()) {
		doStop( kill );

		{	//	Returns the keys
			std::vector<util::key::type> keys;
			ctrl()->reserveKeys( keys );
		}

		//	Resize main video
		ctrl()->resizeVideo( 0, 0, 1, 1 );		
	}

	//	Change status
	status( status::mounted );
}

ApplicationExtension *Application::extension() {
	return _extension;
}

status::type Application::doStart( void ) {
	return status::starting;
}

void Application::doStop( bool /*kill=false*/ ) {
}

bool Application::isRunning() const {
	//	If application is waiting, need stop
	return status() > status::mounted;
}

status::type Application::status() const {
	return _status;
}

void Application::status( status::type st ) {
	//	Try change status	
	if (_status != st) {
		//	Notify status changed
		onStatusChanged( _status, st );
		
		//	Change status
		_status = st;

		//	Notificate ctrl that status was changed
		if (_extension) {
			ctrl()->onApplicationChanged( this );
		}
	}
}

void Application::onStatusChanged( status::type /*old*/, status::type /*actual*/ ) {
}

bool Application::isReady( void ) const {
	return isMounted();
}

void Application::show() const {
	printf( "[app::Application] name=%s, language=%s, id=%s\n", _name.c_str(), _language.c_str(), _appID.asString().c_str() );
	printf( "[app::Application] \treadOnly=%d, start=%d, download=%d\n", _readOnly, _autoStart, _autoDownload );
	printf( "[app::Application] \tpath=%s, service=%04x, visibility=%d\n", _path.c_str(), _service, _visibility );
	printf( "[app::Application] \tpriority=%d, tag=%02x\n", _priority, _componentTag );
	printf( "[app::Application] \tmodes (%d)\n", _videoModes.size() );
	BOOST_FOREACH( video::mode::type mode, _videoModes ) {
		printf( "[app::Application] \t\t%x\n", mode );
	}
	printf( "[app::Application] \ticons (%d)\n", _icons.size() );
	BOOST_FOREACH( const std::string &icon, _icons) {
		printf( "[app::Application] \t\t%s\n", icon.c_str() );
	}
}

void Application::componentTag( BYTE tag ) {
	_componentTag = tag;
}

BYTE Application::componentTag() const {
	return _componentTag;
}

const std::string &Application::path() const {
	return _path;
}

void Application::mount( const std::string &path ) {
	if (fs::exists( path )) {
		printf( "[app::Application] Mount application: app=%s, path=%s\n", _appID.asString().c_str(), path.c_str() );
		_path = path;
		status( status::mounted );
	}
	else {
		printf( "[app::Application] Warning, mount point not exist: path=%s\n", path.c_str() );
	}
}

void Application::mount( const std::string &path, const dsmcc::Events &events ) {
	stopStreamEvents();	
	mount( path );
	_events = events;
}

bool Application::isMounted() const {
	return !_path.empty();
}

void Application::unmount( void ) {
	if (isMounted()) {
		printf( "[app::Application] Unmount application: app=%s\n", _appID.asString().c_str() );
		
		if (!readOnly() && fs::exists( _path ))	{
			//	Remove all content
			fs::remove_all( _path );
		}

		_path = "";
		status( status::unmounted );
	}
}

//	Getters/setters
ApplicationController *Application::ctrl() {
	assert(_extension);
	return _extension->ctrl();
}

bool Application::isBoundToService( ID serviceID ) const {
	return _service ? (serviceID == _service) : false;
}

ID Application::service( void ) const {
	return _service;
}

void Application::service( ID bound ) {
	_service = bound;
}

visibility::type Application::visibility() const {
	return _visibility;
}

void Application::visibility( visibility::type var ) {
	_visibility = var;
}

int Application::priority() const {
	return _priority;
}

void Application::priority( int pri ) {
	_priority = pri;
}

void Application::addIcon( const std::string &icon ) {
	_icons.push_back( icon );
}

void Application::addIcons( const std::vector<std::string> &icons ) {
	_icons.insert( _icons.end(), icons.begin(), icons.end() );
}

const std::vector<std::string> &Application::icons() const {
	return _icons;
}

void Application::addVideoMode( video::mode::type mode ) {
	_videoModes.push_back( mode );
}

const std::vector<video::mode::type> &Application::supportedModes() const {
	return _videoModes;
}

bool Application::readOnly() const {
	return _readOnly;
}

void Application::readOnly( bool var ) {
	_readOnly = var;
}

const ApplicationID &Application::appID() const {
	return _appID;
}

const std::string &Application::language() const {
	return _language;
}

const std::string &Application::name() const {
	return _name;
}

void Application::name( const std::string &n, const std::string &lang/*=""*/ ) {
	_name = n;
	_language = lang;
}

bool Application::autoStart() const {
	return _autoStart;
}

void Application::autoStart( bool var ) {
	_autoStart = var;
}

//	Download methods
bool Application::needDownload( void ) const {
	return (_autoDownload || _autoStart) && !isMounted() && componentTag() != 0xFF;
}

bool Application::autoDownload() const {
	return _autoDownload;
}

void Application::autoDownload( bool var ) {
	_autoDownload = var;
}

void Application::onMounted( const std::string &root, const dsmcc::Events &events ) {
	printf( "[Application] On Object Carousel mounted: root=%s, events=%d\n",
		root.c_str(), events.size() );

	//  MICROTROL
	//	Verificar si la aplicacion esta corriendo.
	//	Si el dsmcc::ObjectCarouselDemuxer detecta un cambio en el carousel, producto de un cambio 
	//	en el transactionid, invocara esta funcion luego de que la aplicacion este corriendo.
	//	Si no se detiene la misma antes de desmontar y volver a montar el carousel el estado resultante 
	//	sera mounted cuando en realidad la aplicacion se encuentra en ejecucion.
	//	Al intentar reiniciar la aplicacion el spawner fallara y el objeto NCLApplication seteara _proc en NULL
	//  Como el spawner mantiene en su cola el taskitem del primer start, ante un disable/enable de aplicaciones
	//  terminara invocando el metodo setupProcess de NCLApplicacion. En dicho metodo se dereferencia el puntero
	//  _proc, pero como el mismo es nulo producto del intento de start fallido, el sistema crashea.
	if (isRunning()) {

		doStop( true );

		{	//	Returns the keys
			std::vector<util::key::type> keys;
			ctrl()->reserveKeys( keys );
		}

		//	Resize main video
		ctrl()->resizeVideo( 0, 0, 1, 1 );		
	
		//	Change status
		status( status::mounted );
	}

	//	Applications already mounted?
	if (isMounted()) {
		//	Un mount previously mounted applications
		unmount();
	}
		
	//	Create application name
	fs::path path = _extension->resMgr()->applicationsPath();
	path /= _appID.asString();

	//	If exits path, remove!
	if (fs::exists( path )) {
		fs::remove_all( path );
	}
	
	//	Rename rootDirectory to applicationsPath/ApplicationName
	fs::rename( root, path ); 

	//	Mount filesystem
	mount( path.string(), events );

	//	Application need start?
	if (autoStart()) {
		start();
	}
}

void Application::onEvent( WORD eventID, const util::Buffer &streamEvent ) {
	StreamEventCallback::const_iterator it=_streamCallbacks.find( eventID );
	if (it != _streamCallbacks.end()) {
		(*it).second( streamEvent );
	}
}

bool Application::startDownload( void ) {
	//	Find PID from DSMCC tag
	ID pid=_extension->findTag( componentTag(), service() );
	if (pid == TS_PID_NULL) {
		printf( "[Application] Warning, ignoring download becouse cannot find data carousel\n" );
		return false;
	}

	//	Create DSMCC	
	dsmcc::DSMCCDemuxer *dsmcc=new dsmcc::DSMCCDemuxer( pid, _extension->resMgr() );

	//	Create temporary root directory
	std::string root = _extension->resMgr()->mkTempFileName( "oc_XXXXXX" );
	
	//	Create object carousel and setup
	dsmcc::ObjectCarouselDemuxer *oc = new dsmcc::ObjectCarouselDemuxer( componentTag(), root, dsmcc );
	oc->onMounted( boost::bind(&Application::onMounted, this, _1, _2) );

	//	Start filter
	if (!_extension->srvMgr()->startFilter( dsmcc )) {
		printf( "[Application] Warning, cannot download application. Filter cannot be started: pid=%04x\n", pid );
		delete oc;
		return false;
	}

	//	Save object carousel demuxer
	_downloads.push_back( oc );
	return true;
}

void Application::stopDownload( void ) {
	//	Stop all filters
	stopStreamEvents();
	
	//	Stop all object carousel
	BOOST_FOREACH( dsmcc::ObjectCarouselDemuxer *oc, _downloads ) {
		ID pid = oc->demux()->pid();
		_extension->srvMgr()->stopFilter( pid );
		delete oc;
	}
	_downloads.clear();
}

//	Video resolution
void Application::setupVideoResolution( int &w, int &h ) {
	video::mode::type mode;

	//	Get list of supported modes
	const std::vector<video::mode::type> &modes = supportedModes();
	if (!modes.empty()) {
		//	Choice one mode from a resolution
		mode = ctrl()->setupVideoResolution( modes );
	}
	else {
		mode = video::mode::sd;
	}

	//	Get resolution
	switch (mode) {
		case video::mode::sd: {
			w = 720;
			h = 576;
			break;
		}
		case video::mode::hd: {
			w = 1280;
			h = 720;
			break;
		}
		case video::mode::fullHD: {
			w = 1920;
			h = 1080;
			break;
		}
		default: {
			h = 720;
			h = 576;
			break;
		}
	}
}

//	Stream event
void Application::stopStreamEvent( ID pid ) {
	_extension->srvMgr()->stopFilter( pid );
}

void Application::stopStreamEvents( void )  {
	BOOST_FOREACH( const StreamEventItem &item, _streams ) {
		const StreamEventType &ste = item.second;
		stopStreamEvent( ste.first->pid() );
	}
	_streams.clear();
	_streamCallbacks.clear();
}

bool Application::registerEvent( const std::string &url, const std::string &eventName, const EventCallback &callback ) {
	bool result=false;
	dsmcc::StreamEventDemuxer *demux=NULL;

	//	Find event on list of available events
	WORD eventID;
	dsmcc::Events::const_iterator it=std::find_if( _events.begin(), _events.end(), EventFinder(url,eventName,eventID) );
	if (it != _events.end()) {
		bool created=false;
		
		//	Find url on already started DSMCC event filters
		StreamEventFilters::iterator fit=_streams.find( url );
		if (fit != _streams.end()) {
			StreamEventType &ste = (*fit).second;
			//	Get already started demux
			demux = ste.first;
			//	Add reference
			ste.second++;
		}
		else {
			//	Find PID from DSMCC tag
			ID pid=_extension->findTag( (*it)->tag(), service() );
			if (pid == TS_PID_NULL) {
				printf( "[Application] Warning, ignoring stream event becouse cannot find PID\n" );
			}
			else {
				//	Create DSMCC demuxer from tag
				demux = new dsmcc::StreamEventDemuxer( pid );
				created = true;
			}
		}
	
		if (demux) {
			//	Setup demuxer
			dsmcc::StreamEventDemuxer::Callback onEvent = boost::bind( &Application::onEvent,this,eventID,_1 );
			demux->registerStreamEvent( eventID, onEvent );

			//	Start demuxer
			if (created) {
				//	Try start filter
				result=_extension->srvMgr()->startFilter( demux );
				if (result) {
					//	Save demux
					_streams[url] = std::make_pair(demux,1);
				}
				else {
					printf( "[Application] Warning, cannot start filter for event: url=%s, eventName=%s\n",
						url.c_str(), eventName.c_str() );
				}
			}
			else {
				result=true;
			}

			if (result) {
				//	Save eventID callback
				_streamCallbacks[eventID] = callback;
			}
		}
	}
	else {
		printf( "[Application] Warning, event not found: url=%s, name=%s\n",
			url.c_str(), eventName.c_str() );
	}

	printf( "[Application] Register event: url=%s, name=%s, eventID=%04x, result=%d\n",
		url.c_str(), eventName.c_str(), eventID, result );	

	return result;
}

void Application::unregisterEvent( const std::string &url, const std::string &eventName ) {
	//	Find url on already started DSMCC event filters
	StreamEventFilters::iterator fit=_streams.find( url );
	if (fit != _streams.end()) {
		//	Find event
		WORD eventID;
		dsmcc::Events::const_iterator it=std::find_if( _events.begin(), _events.end(), EventFinder(url,eventName,eventID) );
		if (it != _events.end()) {
			StreamEventType &ste = (*fit).second;
			
			//	Get already started demux and un-register event
			dsmcc::StreamEventDemuxer *demux = ste.first;
			demux->unregisterStreamEvent( eventID );

			//	Remove reference
			ste.second--;
			if (!ste.second) {
				stopStreamEvent( demux->pid() );
				_streams.erase( fit );
			}

			//	Remove stream event callback
			StreamEventCallback::iterator it=_streamCallbacks.find( eventID );
			if (it != _streamCallbacks.end()) {
				_streamCallbacks.erase( it );
			}
		}

		printf( "[Application] Un-register event: url=%s, name=%s, eventID=%04x\n",
			url.c_str(), eventName.c_str(), eventID );	
	}
}

}
}
