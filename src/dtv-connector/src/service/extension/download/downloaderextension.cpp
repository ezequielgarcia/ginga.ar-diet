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
#include "downloaderextension.h"
#include "downloadinfo.h"
#include "../../service.h"
#include "../../serviceprovider.h"
#include "../../../resourcemanager.h"
#include "../../../provider/provider.h"
#include "../../../demuxer/ts.h"
#include "../../../demuxer/descriptors.h"
#include "../../../demuxer/psi/psi.h"
#include "../../../demuxer/psi/download/sdttdemuxer.h"
#include "../../../demuxer/psi/dsmcc/dsmccdemuxer.h"
#include "../../../demuxer/psi/dsmcc/module.h"
#include "../../../demuxer/psi/dsmcc/datacarouselhelper.h"
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

namespace tuner {

namespace fs = boost::filesystem;

DownloaderExtension::DownloaderExtension( ServiceManager *srvMgr, BYTE maker, BYTE model, Version version )
	: Extension( srvMgr )
{
	_makerID   = maker;
	_modelID   = model;
	_version   = version;
	_dc = new dsmcc::DataCarouselHelper();
}

DownloaderExtension::~DownloaderExtension( void )
{
	delete _dc;
}

void DownloaderExtension::onDownload( const OnDownload &callback ) {
	_onDownload = callback;
}

//	Notifications
void DownloaderExtension::onReady( bool isReady ) {
	if (isReady) {
		startFilters();
	}
	else {
		stopFilters();
	}

}

static bool collectDSMCC( BYTE streamType ) {
	return streamType == PSI_ST_TYPE_B || streamType == PSI_ST_TYPE_D;
}

void DownloaderExtension::onServiceReady( Service *srv ) {
	//	Get TAGs from all DSMCC streams ...
	bool find=_dc->collectTags( srv, &collectDSMCC );
	if (find) {
		tryDownload();
	}
}

void DownloaderExtension::startFilters( void ) {
	TSSectionDemuxer *demux;
	
	printf( "[DownloadExtension] Filter Low/High SDTT profiles\n" );
	assert (!_onDownload.empty());	

	//	Low profile
	demux = createDemuxer<SDTTDemuxer,Sdtt>(
		TS_PID_SDTT_LOW,
		boost::bind(&DownloaderExtension::onSdtt,this,_1) );
	srvMgr()->startFilter( demux );

	//	High profile
	demux = createDemuxer<SDTTDemuxer,Sdtt>(
		TS_PID_SDTT_HIGH,
		boost::bind(&DownloaderExtension::onSdtt,this,_1) );
	srvMgr()->startFilter( demux );
}

void DownloaderExtension::stopFilters( void ) {
	printf( "[DownloadExtension] Stop Low/High SDTT profiles\n" );

	srvMgr()->stopFilter( TS_PID_SDTT_LOW );
	srvMgr()->stopFilter( TS_PID_SDTT_HIGH );
	
	//	Clear any carousel and downloader data
	_dc->clearTags();
	BOOST_FOREACH( DownloadInfo *info, _downloads ) {
		srvMgr()->stopFilter( info->pid() );
		delete info;
	}
	_downloads.clear();
}

//	Tables
void DownloaderExtension::onSdtt( Sdtt *sdtt ) {
	ID model = sdtt->modelID();
	
	sdtt->show();

	//	Check if SDTT is for my model, current network and current TS ...	
	if (MAKER_ID(model) == _makerID && MODEL_ID(model) == _modelID &&
		checkTS( sdtt->tsID() ) && checkNit( sdtt->nitID() ))
	{
		bool find=false;
		const std::vector<Sdtt::ElementaryInfo> &elements = sdtt->elements();
		
		//	Iterate throw SDTT elements
		BOOST_FOREACH( const Sdtt::ElementaryInfo &elem, elements ) {
			bool apply = false;
		
			//	Is compulsory?
			bool isCompulsory = (elem.downloadLevel == 0x01) ? true : false;

			//	Check what version is the target
			switch (elem.versionIndicator) {
				case 0x00:	//	All versions are targeted
					apply = true;
					break;
				case 0x01:	//	Version(s) specified or later are targeted
					apply = (_version >= elem.targetVersion);
					break;
				case 0x02:	//	Version(s) specified or earlier are targeted
					apply = (_version <= elem.targetVersion);
					break;
				case 0x03:	//	Only specified version is targeted
					apply = (_version == elem.targetVersion);
					break;
			};

			printf( "[DownloadExtension] downloadLevel=%02x, version=%04x, indicator=%02x, target version=%04x, apply=%d\n",
				elem.downloadLevel, _version, elem.versionIndicator, elem.targetVersion, apply );

			//	Ok, apply, add download ...
			if (apply) {
				BOOST_FOREACH( const Sdtt::DownloadContentDescriptor &desc, elem.contents ) {
					printf( "[DownloadExtension] Add download: tag=%02x, downloadID=%lx, isCompulsory=%d\n",
						desc.componentTag, desc.downloadID, isCompulsory );
				
					_downloads.push_back( new DownloadInfo( desc.componentTag, desc.downloadID, isCompulsory ) );
				}
				find = true;
			}
		}

		if (find) {
			tryDownload();
		}
	}
	else {
		printf( "[DownloadExtension] Ignoring SDTT: makerID=%02x, modelID=%02x\n", _makerID, _modelID );
	}
	
	delete sdtt;
}

void DownloaderExtension::tryDownload( void ) {
	//	For each downloadInfo ...
	BOOST_FOREACH( DownloadInfo *info, _downloads ) {
		if (!info->started()) {
			printf( "[DownloadExtension] Try download: tag=%02x, downloadID=%lx\n", info->tag(), info->downloadID() );
		
			//	Try find association tag in DSMCC tags stored		
			ID pid=_dc->findTag( info->tag() );
			if (pid != TS_PID_NULL) {
				//	Create demuxer
				dsmcc::DSMCCDemuxer *demux = new dsmcc::DSMCCDemuxer( pid, resMgr() );

				//	Setup callback
				dsmcc::DSMCCDemuxer::DIICallback onParsed = boost::bind(
					&DownloaderExtension::onDII,this,demux,_1);
				demux->onDII( onParsed );

				//	Start filter
				srvMgr()->startFilter( demux );

				//	Mark download started
				info->start( pid );
			}
			else {
				printf( "[DownloadExtension] Warning, tag not found: tag=%02x, downloadID=%0lx\n", info->tag(), info->downloadID() );				
			}
		}
	}
}

struct FindDII {
	FindDII( DWORD downloadID ) { _downloadID = downloadID; }
	bool operator()( const DownloadInfo *info ) const {
		return info->downloadID() == _downloadID;
	}
	DWORD _downloadID;
};

DownloadInfo *DownloaderExtension::findDownload( DWORD downloadID ) {
	std::vector<DownloadInfo *>::const_iterator it = std::find_if( _downloads.begin(), _downloads.end(), FindDII(downloadID) );
	return (it != _downloads.end()) ? (*it) : NULL;
}

void DownloaderExtension::onDII( dsmcc::DSMCCDemuxer *demux, dsmcc::DII *dii ) {
	dii->show();

	//	Check if downloadID is correct
	DownloadInfo *info = findDownload( dii->downloadID() );
	if (info) {
		//	Check if DII is for my download
		bool found=false;
		BOOST_FOREACH( const dsmcc::compatibility::Descriptor &desc, dii->compatibilities() ) {
			if (desc.specifier == DOWNLOAD_SPECIFIER) {
				//	Callback to execute in provider context
				dsmcc::DSMCCDemuxer::ModuleCallback onModule = boost::bind( &DownloaderExtension::onModule,this,_1);
				demux->onModule( onModule );

				//	Filter all modules in DII
				demux->filterModules( dii->downloadID(), dii->modules(), dii->blockSize() );

				//	Set the amount of modules to be downloaded!!!
				info->files( dii->modules().size() );

				found = true;
			}
		}

		if (!found) {
			printf( "[DownloadExtension] Warning, ignoring DII becouse specifier not found: downloadID=%08lx\n", dii->downloadID() );			
		}
	}
	else {
		printf( "[DownloadExtension] Warning, ignoring DII: downloadID=%08lx not referenced\n", dii->downloadID() );
	}
	
	delete dii;
}

void DownloaderExtension::onModule( dsmcc::Module *module ) {
	desc::MapOfDescriptors descs;
	desc::MapOfDescriptors::const_iterator it;
	std::string fileName;

	//	Parse Module Info as descriptors
	dsmcc::module::parseDescriptors( module->info(), descs );
	printf( "[DownloaderExtension] Module received: descriptors=%d, moduleSize=%ld\n",
		descs.size(), module->size() );

	if (descs.size()) {
		//	Check type descriptor exist
		it=descs.find( MODULE_DESC_TYPE );
		if (it == descs.end()) {
			printf( "[DownloaderExtension] Warning: Module descriptor type not found\n" );
			return;
		}
		const std::string &type = (*it).second.get<std::string>();

		//	Check type = application/x-download
		if (type != "application/x-download") {
			printf( "[DownloaderExtension] Warning: Module descriptor type invalid (%s)\n", type.c_str() );
			return;
		}
			
		//	Check name descriptor
		it=descs.find( MODULE_DESC_NAME );
		if (it == descs.end()) {
			printf( "[DownloaderExtension] Warning: Module descriptor name not found\n" );
		}
		else {
			fileName = (*it).second.get<std::string>();
		}

		//	Check CRC descriptor
		it=descs.find( MODULE_DESC_CRC32 );
		if (it != descs.end()) {
			if (!module->checkCRC32( (*it).second.get<util::DWORD>() )) {
				printf( "[DownloadExtension] Warning, Module CRC32 failed\n" );
				return;
			}
			else {
				printf( "[DownloadExtension] CRC 32 check on module ok!\n" );
			}
		}
	}

	//	If filename not processed ...
	if (fileName.empty()) {
		//	Create random file name
		fileName = resMgr()->mkTempFileName( "module_XXXXXXXX", false );
	}

	//	Build path directory
	fs::path path = resMgr()->downloadPath();
	path /= fileName;

	//	Save file as
	if (module->saveAs( path.string() )) {
		printf( "[DownloaderExtension] File downloaded: name=%s, size=%ld\n", path.string().c_str(), module->size() );

		//	Get downloadInfo
		DownloadInfo *info = findDownload( module->downloadID() );
		if (info) {
			//	Add file to download
			info->addFile( path.string() );

			//	If update is complete
			if (info->isComplete()) {
				_onDownload( info );
			}
		}
		else {
			printf( "[DownloaderExtension] Warning: Module not referenced for any download: %s\n", path.string().c_str() );
			fs::remove( path );
		}
	}
	else {
		printf( "[DownloaderExtension] Warning: Module cannot be saved: %s\n", path.string().c_str() );
	}

	delete module;
}

}
