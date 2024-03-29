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

#include <map>
#include <string>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <util/buffer.h>
#include "handler/types.h"

namespace connector {

class Connector;
class MessageHandler;
class KeepAliveHandler;
class KeepAliveData;

namespace local = boost::asio::local;

class Connector {
public:
	Connector( const std::string &file, bool isServer );
	virtual ~Connector();

	void send( const util::Buffer *msg );
	void addHandler( messages::type type, MessageHandler *handler );

	// Signals on connect / disconnect
	typedef boost::function<void ( void)> Callback;
	
	void onConnect( const Callback &callback );
	void onDisconnect( const Callback &callback );
	void onConnectTimeout( const Callback &callback );
	void onTimeout( const Callback &callback );	

	//	Client connect timeout
	void setRetryConnectTimeout( int retryTimeout );
	void setMaxConnectRetry( int maxRetry );

	//	Server connect timeout
	void setConnectTimeout( int timeout );

	//	Keep alive timeout
	void setKeepAliveInterval( int timeout );
	void setMaxKeeyAliveRetry( int timeout );

protected:
	typedef boost::shared_ptr<local::stream_protocol::socket> SocketPtr;	
	typedef boost::shared_ptr<local::stream_protocol::acceptor> AcceptorPtr;
	typedef std::map<enum messages::type, MessageHandler *> Handlers;
	typedef std::pair<enum messages::type, MessageHandler *> HandlerElement;

	void connectorThread( void );
	
	void handle_accept( AcceptorPtr acceptor, const boost::system::error_code& error );
	void onAcceptExpired( const boost::system::error_code& error );
	
	void handle_connect( const boost::system::error_code& err );

	void onKeepAliveEvent( KeepAliveData *ka );
	void onKeepAliveIntervalExpired( const boost::system::error_code& error );
	
	void handle_read( const boost::system::error_code& error, size_t transferred );
	void tryRead( void );	
	void assembleMessage( const util::Buffer &b );	
	void messageReceived( util::Buffer *msj );

	typedef void (Connector::*TimerCallback)( const boost::system::error_code& error );
	void launchTimer( boost::asio::deadline_timer &timer, int ms, TimerCallback );	
	void addHandlerImpl( messages::type type, MessageHandler *handler );
	void connected();
	void disconnected();
	void connectTimeout();
	void timeout();

private:
	boost::thread _thread;
	boost::mutex _mutex;	
	
	boost::asio::io_service _io;
	std::string _endPointName;
	SocketPtr _socket;
	Handlers _handlers;
	util::Buffer _dataIn;
	util::Buffer _msg;

	bool _isServer;
	bool _isConnected;

	//	Keep alvive
	KeepAliveHandler *_keepAliveHandler;
	int  _keepAliveInterval;
	int  _keepAlive;
	int  _maxKeepAliveRetry;
	boost::asio::deadline_timer _keepAliveTimer;

	//	Client
	int _retryConnectTimeout;
	int _maxConnectRetry;

	//	Server
	int _connectTimeout;
	boost::asio::deadline_timer _acceptTimer;	

	// Signals
	Callback _onConnect, _onConnectTimeout, _onDisconnect, _onTimeout;
};

}

