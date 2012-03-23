/*******************************************************************************

  Copyright 2010 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata

********************************************************************************

  This file is part of Ginga implementation.

    This program is free software: you can redistribute it and/or modify it 
  under the terms of the GNU General Public License as published by the Free 
  Software Foundation, either version 2 of the License.

    Ginga is distributed in the hope that it will be useful, but WITHOUT ANY 
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with
  this program. If not, see <http://www.gnu.org/licenses/>.

********************************************************************************

  Este archivo es parte de la implementación de Ginga.

    Este programa es Software Libre: Ud. puede redistribuirlo y/o modificarlo 
  bajo los términos de la Licencia Pública General GNU como es publicada por la
  Free Software Foundation, según la versión 2 de la licencia.

    Ginga se distribuye esperando que resulte de utilidad, pero SIN NINGUNA 
  GARANTÍA; ni siquiera la garantía implícita de COMERCIALIZACIÓN o ADECUACIÓN 
  PARA ALGÚN PROPÓSITO PARTICULAR. Para más detalles, revise la Licencia Pública 
  General GNU.

    Ud. debería haber recibido una copia de la Licencia Pública General GNU 
  junto a este programa. Si no, puede verla en <http://www.gnu.org/licenses/>.

*******************************************************************************/
#include "../../../include/util/mutex/dfbsurfacemutex.h"
#include "blitter_mutex.c"
#include <iostream>

using namespace std;

	DFBSurfaceMutex::DFBSurfaceMutex() {
		_enable  	= false;
		_open           = false;
	}


	DFBSurfaceMutex::~DFBSurfaceMutex() {
		if(_open) {
			BlitterMutexClose(_accessMutexId);
			BlitterMutexClose(_copyMutexId);
		}
	}


	void DFBSurfaceMutex::enable() {
		cout << "Rendering layer controller enabled"<<endl;
		_accessMutexId 	= BlitterMutexOpen(1);
		_copyMutexId   	= BlitterMutexOpen(2);

		if(_accessMutexId < 0) {
			cout <<"Access Mutex open error, id = "<<_accessMutexId<<endl;
		}else{
			cout <<"Access Mutex opened, id = " << _accessMutexId <<endl;
		}

		if(_copyMutexId < 0) {
			cout <<"Copy Mutex open error, id = "<<_copyMutexId<<endl;
		}else{
			cout <<"Copy Mutex opened, id = " << _copyMutexId <<endl;
		}

		_open = (_accessMutexId >= 0) && (_copyMutexId >= 0);
		_enable = true;	
	}


	void DFBSurfaceMutex::lock() {
		if(_enable && _open) {
			BlitterMutexLock(_accessMutexId);
		}
	}

	void DFBSurfaceMutex::unlock() {
		if(_enable && _open) {
			BlitterMutexUnlock(_accessMutexId);
		}
	}

	void DFBSurfaceMutex::copyToSD() {
		if(_enable && _open) {
			BlitterMutexUnlock(_copyMutexId);
		}
	}
