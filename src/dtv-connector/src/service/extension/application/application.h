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

#include "applicationid.h"
#include <util/buffer.h>
#include <boost/function.hpp>
#include <vector>
#include <map>

namespace tuner {

namespace dsmcc {
	class Event;
	class DSMCCDemuxer;
	class StreamEventDemuxer;
	class ObjectCarouselDemuxer;
	typedef std::vector<Event *> Events;
}

namespace app {

class ApplicationController;
class ApplicationExtension;

class Application {
public:
	Application( ApplicationExtension *ext, const ApplicationID &id );
	Application( ApplicationExtension *ext, const ApplicationID &id, const std::string &path );
	virtual ~Application( void );

	bool operator==( const ApplicationID &id );

	//	Start/Stop
	bool start( void );
	void stop( bool kill=false );
	virtual bool isReady( void ) const;

	//	Status
	bool isRunning() const;
	status::type status() const;
	void status( status::type st );
	
	virtual void show() const;
	
	void componentTag( BYTE tag );
	BYTE componentTag() const;

	virtual void mount( const std::string &path );
	virtual void mount( const std::string &path, const dsmcc::Events &events );
	void unmount( void );
	bool isMounted() const;
	const std::string &path() const;	

	const ApplicationID &appID() const;
	const std::string &language() const;
	const std::string &name() const;
	void name( const std::string &n, const std::string &lang="" );

	bool autoStart() const;
	void autoStart( bool var );
	
	bool needDownload( void ) const;
	bool autoDownload() const;
	void autoDownload( bool var );
	bool startDownload( void );
	void stopDownload( void );

	bool isBoundToService( ID serviceID ) const;
	ID service( void ) const;	
	void service( ID serviceID );

	bool readOnly() const;
	void readOnly( bool var );

	visibility::type visibility() const;
	void visibility( visibility::type var );

	int priority() const;
	void priority( int var );

	void addVideoMode( video::mode::type mode );
	const std::vector<video::mode::type> &supportedModes() const;

	void addIcon( const std::string &icon );
	void addIcons( const std::vector<std::string> &icons );
	const std::vector<std::string> &icons() const;

	typedef boost::function<void (const util::Buffer &buf)> EventCallback;
	bool registerEvent( const std::string &url, const std::string &eventName, const EventCallback &callback );
	void unregisterEvent( const std::string &url, const std::string &eventName );

	ApplicationController *ctrl();

protected:
	ApplicationExtension *extension();
	
	virtual status::type doStart( void );
	virtual void doStop( bool kill=false );

	//	Video resolution
	void setupVideoResolution( int &width, int &heigth );

	//	Status changes
	virtual void onStatusChanged( status::type old, status::type actual ); 
		
	//	Object Carousel
	void onMounted( const std::string &path, const dsmcc::Events &events );

	//	Stream Events
	typedef std::map<WORD,EventCallback> StreamEventCallback;
	typedef std::pair<dsmcc::StreamEventDemuxer *,int> StreamEventType;
	typedef std::pair<std::string,StreamEventType> StreamEventItem;
	typedef std::map<std::string,StreamEventType> StreamEventFilters;
	void stopStreamEvents( void );
	void stopStreamEvent( ID pid );	
	void onEvent( WORD eventID, const util::Buffer &streamEvent );
	template<class T>
	inline void loopEvents( T &finder );

private:
	ApplicationExtension *_extension;
	dsmcc::Events    _events;
	status::type     _status;
	ApplicationID    _appID;
	std::string      _name;
	std::string      _language;
	std::string      _path;
	bool             _autoStart;
	bool             _autoDownload;
	bool             _readOnly;
	ID               _service;
	visibility::type _visibility;
	int              _priority;
	BYTE             _componentTag;
	std::vector<dsmcc::ObjectCarouselDemuxer *> _downloads;
	std::vector<video::mode::type> _videoModes;
	std::vector<std::string> _icons;
	StreamEventFilters  _streams;
	StreamEventCallback _streamCallbacks;
};

template<class T>
inline void Application::loopEvents( T &iter ) {
	dsmcc::Events::const_iterator it = _events.begin();
	while (it != _events.end()) {
		iter( (*it) );
		it++;
	}
}

}
}
