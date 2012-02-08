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
#include "networkprovider.h"
#include "../tsprovider.h"
#include <util/buffer.h>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <stdio.h>

namespace tuner {

using boost::asio::ip::tcp;

NetworkProvider::NetworkProvider( const std::string &host, WORD firstPort )
	: _socket(_io), _host(host)
{
	_firstPort = firstPort;
}

NetworkProvider::~NetworkProvider()
{
}

//	Set current network
bool NetworkProvider::startNetwork( const NetworkType &net ) {
	bool result;

	//	Connect to url
	result=connect( net );
	if (result) {
		assert( !_thread.joinable() );
		
		//  start network thread
		_thread = boost::thread( boost::bind( &NetworkProvider::netReader, this, net ) );
	}

	printf( "[NetworkProvider] Start network: net=%s, result=%d\n", net.c_str(), result );
	
    return result;
}

void NetworkProvider::stopNetwork( void ) {
	printf( "[NetworkProvider] Stop network\n" );
		
	//	Cancel socket ...
	_socket.close();

	//	Join thread
	_thread.join();
}
	
bool NetworkProvider::firstNetwork( NetworkType &network ) {
	return scanUrl( 0, network );
}

bool NetworkProvider::nextNetwork( NetworkType &network ) {
	static util::WORD portOffset=0;
	return scanUrl( ++portOffset, network );
}

//	Aux methods
bool NetworkProvider::scanUrl( util::WORD offset, NetworkType &network ) {
	bool result=false;

	//	Is url ok?
	if (!_host.empty() && _firstPort) {
		//	Make url
		network  = _host;
		network += ":";
		network += boost::lexical_cast<std::string>( _firstPort+offset );

		//	Try connect to url
		result=startNetwork( network );
	}
	return result;
}

bool NetworkProvider::connect( const std::string &url ) {
    //  Try connect to url
    tcp::resolver resolver(_io);
    std::string host, port;

    //  TODO: parse url ok! hostname:port
    size_t pos = url.find_first_of( ":" );
    if (pos == std::string::npos) {
		printf( "[NetworkProvider] Invalid URL: %s\n", url.c_str() );
        return false;
    }
    host = url.substr( 0, pos );
    port = url.substr( pos+1 );

    tcp::resolver::query query( host, port );
    tcp::resolver::iterator it = resolver.resolve(query);
    boost::system::error_code error = boost::asio::error::host_not_found;
    _socket.connect(*it, error);
    if (error) {
		_socket.close();
		printf( "[NetworkProvider] Cannot connect to %s\n", url.c_str() );
        return false;
    }

    //  Set low water mark (warning, that is not strict!)
    boost::asio::socket_base::receive_low_watermark option(188*4);
    _socket.set_option(option);
	
	return true;
}

//	Running in provider thread
void NetworkProvider::netReader( const std::string net ) {
    boost::system::error_code error;
    util::Buffer *buf=NULL;

	printf( "[NetworkProvider] network thread started: net=%s\n", net.c_str() );
	
    while (true) {
        //  Get a buffer
        if (!buf) {
            buf = provider()->allocBuffer();
			assert(buf);
        }

        //  Read from network
        size_t len = _socket.read_some( boost::asio::buffer(buf->buffer(),buf->capacity()), error );
        if (error) {
			provider()->freeBuffer( buf );
            assert(!len);
            break;
        }
        else if (len) {
			//	Enqueue buffer into demuxer
            buf->resize( len );
			provider()->process( buf );
            buf = NULL;
        }
    }
    printf( "[NetworkProvider] network thread terminated: net=%s\n", net.c_str() );
}

}

