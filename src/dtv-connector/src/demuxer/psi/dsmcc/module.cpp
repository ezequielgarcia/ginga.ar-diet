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
#include "module.h"
#include "module/memorydata.h"
#include "module/filedata.h"
#include <zlib.h>
#include <assert.h>
#include <memory.h>

namespace tuner {
namespace dsmcc {

Module::Module( DWORD downloadID, const module::Type &moduleInfo, WORD blockSize )
	: _module( moduleInfo )
{
	_downloadID = downloadID;
	_blockSize  = blockSize;
	_data       = createData( _module.size );
    _downloadedSize = 0;
    _downloaded.resize( (_module.size/blockSize)+1, false );
}

Module::~Module()
{
	freeResources();
}

//  Getters
DWORD Module::downloadID() const {
	return _downloadID;
}

ID Module::id() const {
    return _module.id;
}

DWORD Module::size() const {
    return _module.size;
}

BYTE Module::version() const {
    return _module.version;
}

const Buffer &Module::info() const {
	return _module.info;
}

void Module::show() const {
    printf( "[dsmcc::Module] Show: downloadID=%08lx, moduleID=%04d, ver=%02x, size=%08ld, blockSize=%04d, blocks=%d, info=%d\n",
		_downloadID, _module.id, _module.version, _module.size, _blockSize, _downloaded.size(), _module.info.length() );
}

//	Basic read operations
bool Module::readDW( SSIZE_T &off, DWORD &var ) {
	BYTE tmp[sizeof(var)];
    if (read( off, tmp, sizeof(var) ) != sizeof(var)) {
		return false;
	}
    var = GET_DWORD(tmp);
    off += sizeof(var);
	return true;
}

bool Module::readW( SSIZE_T &off, WORD &var ) {
	BYTE tmp[sizeof(var)];	
    if (read( off, tmp, sizeof(var) ) != sizeof(var)) {
		return false;
	}
    var = GET_WORD(tmp);
    off += sizeof(var);
	return true;
}

//  Buffer operations
SSIZE_T Module::read( SSIZE_T off, BYTE *buf, SIZE_T len ) {
	return _data->read( off, buf, len );
}

SSIZE_T Module::sendFile( FILE *of, SSIZE_T o_off, SSIZE_T i_off, SIZE_T len ) {
	return _data->sendFile( of, o_off, i_off, len );
}

SSIZE_T Module::write( SSIZE_T off, BYTE *buf, SIZE_T len ) {
	return _data->write( off, buf, len );
}

bool Module::saveAs( const std::string &name ) {
	return _data->saveAs( name );
}

bool Module::checkCRC32( DWORD crc ) {
	return _data->checkCRC32( crc );
}

bool Module::pushData( WORD blockNumber, BYTE *buf, SIZE_T len ) {
    //  Check blockNumber valid
    if (blockNumber < _downloaded.size()) {
        //  Check that blockNumber not already downloaded
        if (!_downloaded[blockNumber]) {
			//printf( "[dsmcc::Module] push data: downloadID=%08lx, moduleID=%04x, block=%04x, len=%ld\n", _downloadID, _module.id, blockNumber, len );
			SSIZE_T bytes = write( blockNumber*_blockSize, buf, len );
			assert( bytes == len );
            _downloadedSize += bytes;
            _downloaded[blockNumber] = true;
        }
    }
    return _module.size == _downloadedSize;
}

#define CHUNK 4096
void Module::inflate( BYTE /*type*/, DWORD originalSize ) {
	int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

	//	Allocate inflate state
    strm.zalloc   = Z_NULL;
    strm.zfree    = Z_NULL;
    strm.opaque   = Z_NULL;
    strm.avail_in = 0;
    strm.next_in  = Z_NULL;
    ret = inflateInit(&strm);
    if (ret == Z_OK) {
		//	Alloc a new module data for the outout
		ModuleData *infData = createData( originalSize );
	
		//	Decompress until deflate stream ends or end of module
		bool err=false;
		SSIZE_T r_off, w_off;
		r_off = w_off = 0;
		strm.avail_in = read( r_off, in, CHUNK );
		while (!err && strm.avail_in) {
			strm.next_in = in;
			
			//	run inflate() on input until output buffer not full
			do {
				strm.avail_out = CHUNK;
				strm.next_out  = out;
				ret = ::inflate(&strm, Z_NO_FLUSH);
				assert(ret != Z_STREAM_ERROR);  //	state not clobbered
				switch (ret) {
					case Z_NEED_DICT:
					case Z_DATA_ERROR:
					case Z_MEM_ERROR:
						printf( "[dsmcc::Module] Warning, inflate error: ret=%x\n", ret );
						err=true;
						break;
				}

				//	Write inflated data
				have = CHUNK - strm.avail_out;
				if (infData->write( w_off, out, have ) != have) {
					printf( "[dsmcc::Module] Warning, write error: ret=%x\n", ret );
					err=true;
					break;
				}
				w_off += have;
				
			} while (strm.avail_out == 0);
			//	done when inflate() says it's done

			if (!err) {
				r_off += CHUNK;
				strm.avail_in = read( r_off, in, CHUNK );
			}
		}

		//	clean up
		(void)inflateEnd(&strm);
		if (!err) {
			freeResources();
			_data = infData;
			_module.size = originalSize;
		}
		else {
			delete infData;
			_module.size = 0;
		}
	}	
}

//	Create moduleData 
ModuleData *Module::createData( DWORD size ) {
	ModuleData *data;
	
	//	Create module data (depends if can be downloaded in memory)
	if (MemoryData::canDownload( size )) {
		printf( "[Module] Using memory for data: downloadID=%08lx, moduleID=%04d\n", _downloadID, _module.id );
		data = new MemoryData( size );
	}
	else {
		printf( "[Module] Using file for data: downloadID=%08lx, moduleID=%04d\n", _downloadID, _module.id );		
		data = new FileData( _module.size );
	}

	return data;
}

//  Resources operations
bool Module::canStartDownloading( ResourceManager *resMgr ) const {
    //  If free blocks availables, lock blocks
    return _data->canStart( resMgr );
}

void Module::freeResources( void ) {
	if (_data) {
		printf( "[Module] Free resources: downloadID=%08lx, moduleID=%04x\n", _downloadID, _module.id );
		delete _data;
		_data = NULL;
		_downloadedSize = 0;
		_downloaded.clear();
	}
}

}
}

