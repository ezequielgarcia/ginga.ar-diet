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
#include "fileprovider.h"
#include "../tsprovider.h"
#include <util/buffer.h>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <stdio.h>

namespace tuner {

namespace fs = boost::filesystem;

FileProvider::FileProvider( const std::string &root )
{
	scanFiles( root, _files );
	printf( "[FileProvider] Scan files: %d\n", _files.size() );
	
	_scan = 0;
	_exit = false;
	_loop = true;
}

FileProvider::~FileProvider()
{
}

//	Set current network
bool FileProvider::startNetwork( const NetworkType &net ) {
	std::vector<std::string>::const_iterator it = std::find( _files.begin(), _files.end(), net );
	if (it != _files.end()) {
		assert( !_thread.joinable() );
		
		//  start network thread
		_exit = false;		
		_thread = boost::thread( boost::bind( &FileProvider::fileReader, this, net ) );
		return true;
	}
	return false;
}

void FileProvider::stopNetwork( void ) {
	printf( "[FileProvider] Stop network\n" );

	//	Join thread
	_exit = true;	
	_thread.join();	
}

bool FileProvider::firstNetwork( NetworkType &network ) {
	_scan = -1;
	return scan( network );
}

bool FileProvider::nextNetwork( NetworkType &network ) {
	return scan( network );
}

//	Running in provider thread
void FileProvider::fileReader( const std::string net ) {
    util::Buffer *buf=NULL;

	printf( "[FileProvider] file thread started: net=%s\n", net.c_str() );

	FILE *file = fopen( net.c_str(), "r" );
	if (!file) {
		printf( "[FileProvider] file thread terminated: net=%s\n", net.c_str() );
		return;
	}

	//	Seek at random position (TS_PACKET_SIZE)
	//	TODO ::fseek( _file, off, SEEK_SET );
	
    while (!_exit) {
        //  Get a buffer
        if (!buf) {
            buf = provider()->allocBuffer();
			assert(buf);
        }

        //  Read from network
		int bytes = ::fread( buf->buffer(), 1, buf->capacity(), file );		
        if (bytes <= 0 && _loop) {
			::fseek( file, 0, SEEK_SET );
			continue;
		}
        else if (bytes) {
			//	Enqueue buffer into demuxer
            buf->resize( bytes );
			provider()->process( buf );
            buf = NULL;
        }
		else {
            break;
		}
    }
	if (buf) {
		provider()->freeBuffer( buf );
	}
	fclose( file );
    printf( "[FileProvider] file thread terminated: net=%s\n", net.c_str() );
}

bool FileProvider::scan( NetworkType &network ) {
	bool result=false;
	_scan++;
	if (_files.size() > static_cast<size_t>(_scan)) {
		network = _files[_scan];
		result=startNetwork( network );
	}
	printf( "[FileProvider] Scan %d: %s\n", _scan, network.c_str() );
	return result;
}

void FileProvider::scanFiles( const std::string &root, std::vector<std::string> &files ) {
	if (fs::exists( root )) {
		fs::directory_iterator end_itr;
		for (fs::directory_iterator itr( root ); itr != end_itr; ++itr) {
			if (fs::is_directory( itr->status() )) {
				scanFiles( itr->path().string(), files );
			}
			else if (fs::is_regular_file( itr->status() )) {
				fs::path file = itr->path();
				if (file.extension() == ".ts") {
					files.push_back( file.string() );
				}
			}
		}
	}
}

}

