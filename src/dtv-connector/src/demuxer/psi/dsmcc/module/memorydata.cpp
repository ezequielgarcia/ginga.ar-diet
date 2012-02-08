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
#include "memorydata.h"
#include <util/buffer.h>
#include <util/resourcepool.h>
#include <util/functions.h>
#include <boost/filesystem.hpp>
#include <memory.h>

namespace tuner {
namespace dsmcc {

namespace fs = boost::filesystem;

util::pool::ResourcePool *MemoryData::_pool = NULL;

void MemoryData::initialize( DWORD maxBlocks, DWORD blockSize ) {
	_pool = new util::pool::ResourcePool( resources::modules, maxBlocks, blockSize );
}

void MemoryData::cleanup() {
	delete _pool;
	_pool = NULL;
}

bool MemoryData::canDownload( DWORD moduleSize ) {
	return _pool->canAlloc( moduleSize );
}

MemoryData::MemoryData( DWORD size )
	: ModuleData( size )
{
	_resourcesLocked = false;
	_blockSize = _pool->blockSize();
	_blocks.resize( _pool->blocks( size ), NULL );
}

MemoryData::~MemoryData( void )
{
    //  Collect free blocks
	std::vector<util::Buffer *> toFree;
    for (size_t i=0; i<_blocks.size(); i++) {
		if (_blocks[i]) {
			toFree.push_back( _blocks[i] );
		}
    }
	//	Free blocks
	if (toFree.size()) {
		_pool->free( toFree, true );
	}

	//	Free lock from resources
	if (_resourcesLocked) {
		_pool->unlockBlocks( size() );
	}
	
	//	Clean structrure
    _blocks.clear();
}

//	Getters
bool MemoryData::canStart( ResourceManager * /*resMgr*/ ) {
	assert (!_resourcesLocked);
	_resourcesLocked = _pool->lockBlocks( size() );
	return _resourcesLocked;
}

WORD MemoryData::blockSize() const {
    return _blockSize;
}

//  Buffer operations
SSIZE_T MemoryData::read( SSIZE_T off, BYTE *buf, SIZE_T len ) {
    return process( off, len, buf, &MemoryData::doRead );
}

SSIZE_T MemoryData::sendFile( FILE *of, SSIZE_T o_off, SSIZE_T i_off, SIZE_T len ) {
	::fseek( of, o_off, SEEK_SET );
    return process( i_off, len, of, &MemoryData::doSendfile );
}

SSIZE_T MemoryData::write( SSIZE_T off, BYTE *buf, SIZE_T len ) {
    return process( off, len, buf, &MemoryData::doWrite );
}

bool MemoryData::checkCRC32( DWORD crc ) {
	util::BYTE buf[_blockSize];

	SSIZE_T off = 0;
	DWORD calc = -1;
	SSIZE_T bytes;
	while (off < size()) {
		bytes = process( off, _blockSize, buf, &MemoryData::doRead );
		if (bytes) {
			//	Check CRC
			calc = util::crc_calc( calc, buf, bytes );
		}
		off += bytes;
	}
	return calc == crc;
}

bool MemoryData::saveAs( const std::string &name ) {
	bool result=false;

	//	If output file exist, remove ...
	if (fs::exists( name )) {
		fs::remove( name );
	}

	FILE *file = fopen( name.c_str(), "w" );
	if (file) {
		//  Write content to disk
		result = sendFile( file, 0, 0, size() ) == size();
		fclose( file );
	}
	return result;
}

void MemoryData::doRead( SSIZE_T block, SSIZE_T offset, SIZE_T len, SSIZE_T processed, BYTE *buf ) {
    //  Get block
    util::Buffer *b = _blocks[block];
    memcpy( buf+processed, b->buffer()+offset, len );
}

void MemoryData::doWrite( SSIZE_T block, SSIZE_T offset, SIZE_T len, SSIZE_T processed, BYTE *buf ) {
    //  Get first block
    Buffer *b = _blocks[block];
    if (!b) {
        //  Alloc buffer
        b = _pool->alloc();
        _blocks[block] = b;
    }
    b->copy( offset, (char *)buf+processed, len );
}

void MemoryData::doSendfile( SSIZE_T block, SSIZE_T offset, SIZE_T len, SSIZE_T /*processed*/, FILE *f ) {
    //  Get block
    Buffer *b = _blocks[block];
    //  Write to file
    fwrite( b->buffer()+offset, 1, len, f );
}

}
}
