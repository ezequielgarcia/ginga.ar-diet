/*******************************************************************************

  Copyright (C) 2010, 2011 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata

********************************************************************************

  This file is part of DTV-util implementation.

    DTV-util is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation, either version 2 of the License.

    DTV-util is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
  this program. If not, see <http://www.gnu.org/licenses/>.

********************************************************************************

  Este archivo es parte de la implementación de DTV-util.

    DTV-util es Software Libre: Ud. puede redistribuirlo y/o modificarlo
  bajo los términos de la Licencia Pública General Reducida GNU como es publicada por la
  Free Software Foundation, según la versión 2 de la licencia.

    DTV-util se distribuye esperando que resulte de utilidad, pero SIN NINGUNA
  GARANTÍA; ni siquiera la garantía implícita de COMERCIALIZACIÓN o ADECUACIÓN
  PARA ALGÚN PROPÓSITO PARTICULAR. Para más detalles, revise la Licencia Pública
  General Reducida GNU.

    Ud. debería haber recibido una copia de la Licencia Pública General Reducida GNU
  junto a este programa. Si no, puede verla en <http://www.gnu.org/licenses/>.

*******************************************************************************/
#pragma once

#include "types.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <queue>

namespace util {

class Buffer;

namespace pool {

class ResourcePool {
public:
    ResourcePool( int id, DWORD max, DWORD blockSize );
    virtual ~ResourcePool( void );

    DWORD blocks( DWORD size ) const;
    DWORD blockSize( void ) const;
    bool canAlloc( DWORD moduleSize ) const;
    bool lockBlocks( DWORD moduleSize );
	void unlockBlocks( DWORD moduleSize );	
    Buffer *alloc( DWORD msTimeout=0 );
	void free( std::vector<Buffer *> &buffers, bool locked=false );
    void free( Buffer *buf, bool locked=false );

protected:
    void clear();
    DWORD availables( void ) const;

private:
    int   _id;
    DWORD _max;
    DWORD _blockSize;
    DWORD _allocated;
    DWORD _locked;
    boost::mutex _mFreeBuffers;
    boost::condition_variable _cWakeup;
    std::queue<Buffer *> _freeBuffers;
};

}
}
