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
#include "filedata.h"
#include "../../../../resourcemanager.h"
#include <util/functions.h>
#include <boost/filesystem.hpp>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/statvfs.h>

#define BUFFER_SIZE 4096

namespace tuner {
namespace dsmcc {

namespace fs = boost::filesystem;

FileData::FileData( DWORD size )
	: ModuleData( size )
{
	_file = NULL;
}

FileData::~FileData( void )
{	
	if (_file) {
		fclose( _file );
		fs::remove( _name );
	}
}

//	Getters
bool FileData::canStart( ResourceManager *resMgr ) {
	bool result=false;
	struct statvfs st;
	
	//	Check disk free
	if (::statvfs( resMgr->rootPath().c_str(), &st ) == 0) {
		result=st.f_bfree*st.f_bsize > size();
		if (result) {
			//	Try open temporary file
			assert(!_file);
			_file = resMgr->openTempFileName( "module_XXXXXXXX", _name );
			result = _file ? true : false;
		}
	}
	return result;
}

//	Operations
bool FileData::checkCRC32( DWORD crc ) {
	util::BYTE buf[BUFFER_SIZE];
	ssize_t r;
	DWORD calc = -1;

	assert(_file);
	::fseek( _file, 0, SEEK_SET );
	r=::fread( buf, 1, BUFFER_SIZE, _file );
	while (r > 0) {
		//	Check CRC
		calc = util::crc_calc( calc, buf, r );
		r=::fread( buf, 1, BUFFER_SIZE, _file );		
	}
	return calc == crc;
}

bool FileData::saveAs( const std::string &name ) {
	if (_file) {
		//	Close file ...
		fclose( _file );
		_file = NULL;

		try {
			//	If output file exist, remove ...
			if (fs::exists( name )) {
				fs::remove( name );
			}

			//	Rename temporary file to new name
			fs::rename( _name, name );
		} catch (...) {
			return false;
		}
		return true;
	}
	return false;
}

SSIZE_T FileData::read( SSIZE_T off, BYTE *buf, SIZE_T len ) {
	assert(_file);
	size_t bytes = 0;

	if (::fseek( _file, off, SEEK_SET ) == 0) {
		bytes = ::fread( buf, 1, len, _file );
		if (!bytes) {
			printf( "[FileData] read error: errno=%d, str=%s\n", errno, strerror(errno) );
		}
	}
	else {
		printf( "[FileData] fseek error: errno=%d, str=%s\n", errno, strerror(errno) );
		
	}
	return bytes;
}

SSIZE_T FileData::sendFile( FILE *of, SSIZE_T o_off, SSIZE_T i_off, SIZE_T /*len*/ ) {
	char buf[BUFFER_SIZE];
	assert(_file);

	::fseek( _file, i_off, SEEK_SET );
	::fseek( of, o_off, SEEK_SET );
	ssize_t r, bytes=0;
	r=::fread( buf, 1, BUFFER_SIZE, _file );
	while (r > 0) {
		bytes += ::fwrite( buf, 1, r, of );
		r=::fread( buf, 1, BUFFER_SIZE, _file );		
	}
	return bytes;
}

SSIZE_T FileData::write( SSIZE_T off, BYTE *buf, SIZE_T len ) {
	assert(_file);
	::fseek( _file, off, SEEK_SET );
	return ::fwrite( buf, 1, len, _file );
}

}
}

