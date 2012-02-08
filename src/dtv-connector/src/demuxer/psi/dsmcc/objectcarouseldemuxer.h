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

#include "biop/objectlocation.h"
#include <boost/function.hpp>
#include <list>
#include <vector>

namespace tuner {
namespace dsmcc {

class DSI;
class DII;
class Module;
class Event;
typedef std::vector<Event *> Events;
class DSMCCDemuxer;

namespace biop {
	class Object;
	class Event;
	typedef DWORD ObjectKeyType;
}

class ObjectCarouselDemuxer {
public:
	ObjectCarouselDemuxer( ID tag, const std::string &rootPath, DSMCCDemuxer *demux );
	virtual ~ObjectCarouselDemuxer( void );

	//	On Object Carousel mounted
	typedef boost::function<void (const std::string &root, const Events &events )> OnMounted;
	void onMounted( const OnMounted &callback );

	//	Returns DSMCC demuxer to use
	DSMCCDemuxer *demux() const;

protected:
	//	Tables
	void onDSI( DSI *dsi );
	void onDII( DII *dii );	
	void onModule( Module *module );

	//	Generic aux
	void updateCarousel( biop::Object *object );
	void cleanupRoot( void );	
	void cleanup( bool freeResources=true );
	void mountCarousel( void );	

private:
	ID _tag;
	DSMCCDemuxer *_demux;
	std::string _rootPath;
	biop::ObjectLocation _rootLocation;
    biop::Object *_root;
    std::list<biop::Object *> _objects;
	OnMounted _onMounted;
};

}
}

