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
#include "resourcemanager.h"
#include <boost/filesystem.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_REPEAT    5
#define MAX_PATH      256

namespace tuner {

namespace fs = boost::filesystem;

static void check( const fs::path &path ) {
	//	If not exist
	if (!fs::exists( path )) {
		//	create directory
		if (!fs::create_directory( path )) {
			printf( "[ApplicationExtension] Cannot create directory %s\n", path.string().c_str() );
			throw;
		}
	}
}

ResourceManager::ResourceManager( const std::string &root, int memoryBlocks, int maxModules, DWORD maxModuleSize )
{
	_root          = root;
	_memoryBlocks  = memoryBlocks;
	_maxModules    = maxModules;
	_maxModuleSize = maxModuleSize;

	printf( "[ResourceManager] Initializing: root=%s, memBlocks=%d, maxModules=%d, maxModuleSize=%ld\n",
		_root.c_str(), _memoryBlocks, _maxModules, _maxModuleSize );

	//	Clean temporary files
	clean();
}

ResourceManager::~ResourceManager( void )
{
}

//	Pathset
const std::string &ResourceManager::rootPath( void ) const {
	return _root;
}

std::string ResourceManager::applicationsPath( void ) const {
	fs::path root = _root;
	root /= "applications";
	return root.string();
}

std::string ResourceManager::downloadPath( void ) const {
	fs::path root = _root;
	root /= "download";
	return root.string();	
}

std::string ResourceManager::temporaryPath( void ) const {
	fs::path root = _root;
	root /= "tmp";
	return root.string();	
}

//	Utils
std::string ResourceManager::mkTempFileName( const std::string &templateFileName, bool useTempPath/*=true*/ ) const {
	char tmpFile[MAX_PATH];
	char *name=NULL;

	if (!templateFileName.empty()) {
		fs::path tmp;
		if (useTempPath) {
			tmp = temporaryPath();
		}
		tmp /= templateFileName;
		strncpy( tmpFile, tmp.string().c_str(), MAX_PATH );
		name = mktemp( tmpFile );
	}
	return name;
}

int ResourceManager::mkTemp( const std::string &templateFileName ) const {
	char tmpFile[MAX_PATH];

	if (!templateFileName.empty()) {
		fs::path tmp = temporaryPath();
		tmp /= templateFileName;
		strncpy( tmpFile, tmp.string().c_str(), MAX_PATH );
		return mkstemp( tmpFile );
	}
	return -1;
}

FILE *ResourceManager::openTempFileName( const std::string &templateFileName, std::string &filename ) const {
    char tmpFile[MAX_PATH];
    FILE *file = NULL;
    char *name;
    struct stat st;
    int iter=0;

	if (!templateFileName.empty()) {
		fs::path tmp = temporaryPath();
		tmp /= templateFileName;
		strncpy( tmpFile, tmp.string().c_str(), MAX_PATH );
		while (!file && iter < MAX_REPEAT) {
			name = mktemp( tmpFile );
			//printf( "[ResourceManager::openTempFileName] template=%s, result=%s\n", tmp.string().c_str(), name );
			if (name && stat(name,&st) < 0) {
				//  If file not exist, open file
				file = fopen(name,"w+");
			}
			iter++;
		}
		if (iter < MAX_REPEAT) {
			filename = name;
			//printf( "[ResourceManager::openTempFileName] result=%s\n", name );			
		}
	}

    return file;
}

void ResourceManager::clean() {
	//	If ramdisk exist ...
	if (fs::exists( _root )) {
		//	Remove all content
		fs::remove_all( _root );
	}

	//	create ramDisk directory
	if (!fs::create_directory( _root )) {
		printf( "[ApplicationExtension] Cannot create directory %s\n", _root.c_str() );
		throw;
	}

	//	check applications path
	check( applicationsPath() );

	//	check download path
	check( downloadPath() );

	//	check temporary path
	check( temporaryPath() );
}

//	Resources
int ResourceManager::memoryblocks() const {
	return _memoryBlocks;
}

int ResourceManager::maxModules() const {
	return _maxModules;
}

DWORD ResourceManager::maxModuleSize() const {
	return _maxModuleSize;
}

}
