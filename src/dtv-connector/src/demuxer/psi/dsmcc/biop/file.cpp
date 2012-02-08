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
#include "file.h"
#include "../module.h"
#include "../../../../resourcemanager.h"
#include <boost/filesystem.hpp>
#include <stdio.h>


namespace tuner {
namespace dsmcc {
namespace biop {

namespace fs = boost::filesystem;

File::File( const ObjectLocation &loc )
: Object( loc )
{
    _contentLen = 0;
}

File::~File()
{
	clean( _tmpFileName );
}

bool File::parseBody( ResourceManager *resMgr, Module *module, SSIZE_T offset, DWORD /*bodyLen*/ ) {
    //  Read content length
	if (!module->readDW( offset, _contentLen )) {
		printf( "[biop::File] Warning, cannot read content length\n" );
		return false;
	}

    if (_contentLen) {
        //  Create temporary file to write to disk
        std::string templateFile = "file_XXXXXXXX";
        FILE *file = resMgr->openTempFileName( templateFile, _tmpFileName );
        if (!file) {
            printf( "[dsmcc::File] Warning cannot create temporary file %s\n", _tmpFileName.c_str() );
            return false;
        }

        //  Write content to disk
        if (module->sendFile( file, 0, offset, _contentLen ) != _contentLen) {
			printf( "[dsmcc::File] Warning cannot write file content: file=%s\n", _tmpFileName.c_str() );
			return false;
		}
        fclose( file );

        //printf( "[dsmccFile] key=%lx, filename: %s, size=%ld, offset=%ld\n", key(), _tmpFileName.c_str(), _contentLen, offset );
    }
	return true;
}

void File::process( Object *parent, const std::string &objName, std::list<Object *> & /*unprocessed*/ ) {
    name( getPath(parent,objName) );

    printf( "[dsmcc::File]        Object name %s processed\n", name().c_str() );

    //  Rename temporary file to name
    fs::rename( _tmpFileName, name() );
    _tmpFileName = "";
}

void File::clean( const std::string &file ) {
	if (!file.empty() && fs::exists( file.c_str() )) {
		fs::remove( file.c_str() );
	}
}

void File::freeResources( void ) {
	clean( _tmpFileName );
	clean( name() );
}

BYTE File::type() const {
    return 1;
}

bool File::isComplete() const {
    return _tmpFileName.empty();
}

}
}
}
