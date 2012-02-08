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

#include "moduledata.h"
#include <vector>

namespace util {

class Buffer;

namespace pool {
	class ResourcePool;
}

}

namespace tuner {
namespace dsmcc {

class MemoryData : public ModuleData {
public:
	MemoryData( DWORD size );
	virtual ~MemoryData( void );

	static void initialize( DWORD maxBlocks, DWORD blockSize );
	static void cleanup();
	static bool canDownload( DWORD moduleSize );

	//	Getters
	virtual bool canStart( ResourceManager *resMgr );	

	//	Operations
	virtual bool checkCRC32( DWORD crc );
	virtual bool saveAs( const std::string &name );	
	virtual SSIZE_T read( SSIZE_T off, BYTE *buf, SIZE_T len );
	virtual SSIZE_T sendFile( FILE *of, SSIZE_T o_off, SSIZE_T i_off, SIZE_T len );	
	virtual SSIZE_T write( SSIZE_T off, BYTE *buf, SIZE_T len );
	
protected:
	WORD blockSize() const;

	//  Function to process the buffers
	template<typename T>
		SSIZE_T process( SSIZE_T off, SIZE_T len, T param, void (MemoryData::*fnc)( SSIZE_T block, SSIZE_T offset, SIZE_T len, SIZE_T processed, T param ) );

	//  Aux to buffer methods
	void doSendfile( SSIZE_T block, SSIZE_T offset, SIZE_T len, SSIZE_T processed, FILE *f );
	void doRead( SSIZE_T block, SSIZE_T offset, SIZE_T len, SSIZE_T processed, BYTE *buf );
	void doWrite( SSIZE_T block, SSIZE_T offset, SIZE_T len, SSIZE_T processed, BYTE *buf );
	
private:
	bool _resourcesLocked;	
	std::vector<util::Buffer *> _blocks;
	WORD _blockSize;
	static util::pool::ResourcePool *_pool;
};

template<typename T>
inline SSIZE_T MemoryData::process(
	SSIZE_T off,
	SIZE_T len,
	T param,
	void (MemoryData::*fnc)( SSIZE_T block, SSIZE_T offset, SIZE_T len, SIZE_T processed, T param ) )
{
    SSIZE_T processed=0;
	DWORD s = size();
	
    //printf( "process begin: id=%d, size=%ld, offset=%ld, len=%ld\n", id, _module.size, off, len );

    //  Check overflow
	if (off < s) {
		SIZE_T bytes = (len > s-off) ? s-off : len;

		//  Calculate the first block, offset
		SSIZE_T block = off / _blockSize;
		SSIZE_T offset = off % _blockSize;

		SIZE_T copy;
		while (bytes) {
			//  Calculate the bytes to process
			copy = bytes > _blockSize-offset ? _blockSize-offset : bytes;

			(this->*fnc)( block, offset, copy, processed, param );
			//printf( "process: b=%p, block=%ld, offset=%ld, bytes=%ld, copy=%ld\n", b, block, offset, bytes, copy );

			bytes -= copy;
			processed += copy;
			block++;
			offset = 0;
		}
	}
	return processed;
}

}
}
