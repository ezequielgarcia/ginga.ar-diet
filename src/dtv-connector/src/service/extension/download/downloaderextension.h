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

#include "../extension.h"
#include <boost/function.hpp>
#include <vector>

namespace tuner {

namespace dsmcc {
	class DII;
	class DSMCCDemuxer;
	class Module;
	class DataCarouselHelper;
}

class Service;
class TSSectionDemuxer;
class Sdtt;
class DownloadInfo;

class DownloaderExtension : public Extension {
public:
	DownloaderExtension( ServiceManager *srvMgr, BYTE maker, BYTE model, Version version );
	virtual ~DownloaderExtension( void );

	typedef boost::function<void (DownloadInfo *)> OnDownload;
	void onDownload( const OnDownload &callback );

	//	Start/Stop 
	virtual void onReady( bool isReady );
	virtual void onServiceReady( Service *srv );
	
	//	Types
	typedef std::pair<BYTE,ID> DSMCCTag;

protected:
	//	Tables
	void onSdtt( Sdtt *sdtt );
	void onDII( dsmcc::DSMCCDemuxer *demux, dsmcc::DII *dii );
	void onModule( dsmcc::Module *module );

	//	Aux for filter SDTT
	void startFilters( void );
	void stopFilters( void );

	//	Download
	DownloadInfo *findDownload( DWORD downloadID );
	void tryDownload( void );	

private:
	BYTE _makerID;
	BYTE _modelID;
	ID _version;
	dsmcc::DataCarouselHelper *_dc;
	std::vector<DownloadInfo *> _downloads;
	OnDownload _onDownload;
};

}
