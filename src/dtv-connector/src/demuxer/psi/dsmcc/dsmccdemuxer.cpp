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
#include "dsmccdemuxer.h"
#include "parser.h"
#include "module.h"
#include "../psi.h"
#include "../../descriptors/demuxer.h"
#include "../../../resourcemanager.h"
#include <util/buffer.h>
#include <boost/foreach.hpp>
#include <stdio.h>
#include <algorithm> 

namespace tuner {
namespace dsmcc {

DSMCCDemuxer::DSMCCDemuxer( ID pid, ResourceManager *resMgr )
  : PSIDemuxer( pid )
{
	_resMgr = resMgr;
}

DSMCCDemuxer::~DSMCCDemuxer()
{
    clear( _listen );
    clear( _waitingResources );
}

ResourceManager *DSMCCDemuxer::resourceMgr() {
	return _resMgr;
}

DWORD DSMCCDemuxer::timeout() const {
	//	Ignore timeout ...
	return 0;
}

DWORD DSMCCDemuxer::frequency() const {
	//	2Mbs -> 61 seccions in 1000ms -> 1section c 16ms
	return 16;
}

WORD DSMCCDemuxer::maxSectionBytes() const {
	return TSS_PRI_MAX_BYTES;
}

//  Callback called when DII parsed	
void DSMCCDemuxer::onDSI( const DSICallback &callback ) {
	_onDSI = callback;
}

//  Callback called when DII parsed	
void DSMCCDemuxer::onDII( const DIICallback &callback ) {
	_onDII = callback;	
}

//  Callback called when a Module was parsed
void DSMCCDemuxer::onModule( const ModuleCallback &callback ) {
	_onModule = callback;	
}

//	Filter module
bool DSMCCDemuxer::filterModules( DWORD downloadID, const module::Modules &modules, WORD blockSize ) {
	bool canDownloadAll=false;

	//	Check if modules size can be downloaded
	if (modules.size() < static_cast<size_t>(_resMgr->maxModules())) {
		canDownloadAll=true;
		
		//	Check if all modules can be downloaded by module size (as a transaction)
		BOOST_FOREACH( const module::Type &mod, modules ) {
			canDownloadAll &= (mod.size < _resMgr->maxModuleSize());
		}
	}

	//	If all modules can be downloaded, begin!
	if (canDownloadAll) {
		BOOST_FOREACH( const module::Type &mod, modules ) {
			filterModule( downloadID, mod, blockSize );
		}
		return true;
	}
	else {
		printf( "[DSMCCDemuxer] Warning: Modules can not be downloaded.\n" );
	}
	return false;
}

bool DSMCCDemuxer::filterModule( DWORD downloadID, const module::Type &moduleInfo, WORD blockSize ) {
	assert( !_onModule.empty() );
	
	//  Create new module
	Module *module = new Module( downloadID, moduleInfo, blockSize );

	//  Can start downloading now?
	if (module->canStartDownloading( _resMgr )) {
		//  Start downloading now!
		_listen.push_back( module );

		printf( "[DSMCCDemuxer] Starting filter of module %04x\n", module->id() );
	}
	else {
		//  Wait for modules to release resources
		_waitingResources.push_back( module );

		printf( "[DSMCCDemuxer] Module %04x waiting for resources\n", module->id() );
	}
	return true;
}

//  PSI demuxer virtual methods
bool DSMCCDemuxer::useStandardVersion() const {
    return false;
}

void DSMCCDemuxer::onSection( BYTE *section, SIZE_T len ) {
    BYTE tableID = PSI_TABLE(section);

	//printf( "[DSMCCDemuxer] onSection: tableID=%x, len=%ld\n", tableID, len );
    switch (tableID) {
        case DSMCC_TID_DDM:
            parseDownloadDataMessages( section, len );
            break;
        case DSMCC_TID_UNM:
            parseDownloadControlMessages( section, len );
            break;
        default:
            printf( "[DSMCCDemuxer] Warning: DSMCC table not parsed: %x\n", tableID );
    };
}

//  Parse Download Data Messages
void DSMCCDemuxer::parseDownloadDataMessages( BYTE *section, SIZE_T len ) {
    //  Skip PSI header
    BYTE *dsmccSection = section+PSI_PAYLOAD_OFFSET;

    //  Check DSMCC common header (len,adaptation len)
    int dsmccHeaderLen = checkSection( dsmccSection, len );
    if (dsmccHeaderLen) {
        //  Sanity checks on common header
        if (DSMCC_TYPE(dsmccSection) != DSMCC_TYPE_DOWNLOAD ||
            DSMCC_DISCRIMINATOR(dsmccSection) != 0x11 ||
            DSMCC_RESERVED(dsmccSection) != 0xFF)
        {
            printf( "[DSMCCDemuxer] Warning: Download data message header incorrect: len=%d, type=%d, discriminator=%x, reserved=%d\n",
                dsmccHeaderLen, DSMCC_TYPE(dsmccSection), DSMCC_DISCRIMINATOR(dsmccSection), DSMCC_RESERVED(dsmccSection) );
            return;
        }

        WORD msgID = DSMCC_ID(dsmccSection);
        if (msgID == DSMCC_MSGID_DDB) {
            // Parse Download Data Block
            parseDDB( dsmccSection, dsmccHeaderLen );
        }
		else {
			printf( "[DSMCCDemuxer] Warning: Download data message not parsed: msgID%x\n", msgID );
		}
    }
}

void DSMCCDemuxer::parseDDB( BYTE *dsmccSection, SIZE_T dsmccHeaderLen ) {
    BYTE *dsmccPayload  = dsmccSection+dsmccHeaderLen;
    int dsmccLenPayload = DSMCC_LEN(dsmccSection)-DSMCC_ADAPTATION_LEN(dsmccSection);
    int offset = 0;

    DWORD downloadID   = DSMCC_DOWNLOAD_ID(dsmccSection);

    WORD moduleID      = RW(dsmccPayload,offset);
    BYTE moduleVersion = RB(dsmccPayload,offset);
    BYTE reserved      = RB(dsmccPayload,offset);
    WORD blockNumber   = RW(dsmccPayload,offset);
    if (reserved == 0xFF) {
        //  find module only on listening modules
        Module *module=find( downloadID, moduleID, moduleVersion, _listen );

        //  If module exist, add push block of data
        if (module && module->pushData( blockNumber, dsmccPayload+offset, dsmccLenPayload-offset )) {
            moduleComplete( module );
        }
    }
}

//  Parse Download Control Messages
struct TransactionIDFinder {
	TransactionIDFinder( DWORD tID ) : _tID(tID) {}
	bool operator()( DWORD &tID ) {
		return (DSMCC_CM_IDENT(tID) == DSMCC_CM_IDENT(_tID));
	}
	DWORD _tID;
};

DWORD DSMCCDemuxer::findControlMessage( DWORD tID ) {
	DWORD res;
	
	//	Find tID on list of control messages stored ...
	struct TransactionIDFinder pred(tID);
	std::vector<DWORD>::iterator it=std::find_if( _controlMessages.begin(), _controlMessages.end(), pred );
	if (it == _controlMessages.end()) {
		//	Not found!, store it
		_controlMessages.push_back( tID );

		//	Return tID but with UPDATED bit flag changed
		tID ^= DSMCC_TID_UPDATE;
		res = tID;
	}
	else {
		res = (*it);
		(*it) = tID;
	}
	return res;
}

void DSMCCDemuxer::parseDownloadControlMessages( BYTE *section, SIZE_T len ) {
    //  Skip PSI header
    BYTE *dsmccSection = section+PSI_PAYLOAD_OFFSET;

    //  Check DSMCC common header (len,adaptation len)
    int dsmccHeaderLen = checkSection( dsmccSection, len );
    if (dsmccHeaderLen) {
        BYTE *dsmccPayload  = dsmccSection+dsmccHeaderLen;
        int dsmccLenPayload = DSMCC_LEN(dsmccSection)-DSMCC_ADAPTATION_LEN(dsmccSection);

		//  DSMCC message
		WORD msgID = DSMCC_ID(dsmccSection);

		//	Need process message
		if ((msgID == DSMCC_MSGID_DSI && !_onDSI.empty()) ||
			(msgID == DSMCC_MSGID_DII && !_onDII.empty()))
		{
			//  DVB BlueBook A068 Rev.3 Annex B, Table B.34
			//	TransactionID provide of both identification and versioning
			DWORD tID     = DSMCC_TRANSACTION_ID(dsmccSection);
			DWORD lastTID = findControlMessage( tID );

			//	Check if message was updated (or new)
			if (DSMCC_CM_UPDATED(tID) != DSMCC_CM_UPDATED(lastTID) ||
				DSMCC_CM_VERSION(tID) != DSMCC_CM_VERSION(lastTID))
			{
				printf( "[DSMCCDemuxer] control msg=%x, tid=%lx, lastTID=%lx\n", msgID, tID, lastTID);
			
				if (msgID == DSMCC_MSGID_DII) {
					//	Parse Download Info Indication
					parseDII( dsmccPayload, dsmccLenPayload );
				}
				else if (msgID == DSMCC_MSGID_DSI) {
					//	Parse Download Server Initiate
					parseDSI( dsmccPayload, dsmccLenPayload );
				}
				else {
					printf( "[DSMCCDemuxer] Warning: Download control message no processed. Message msg=%x\n", msgID );
				}
			}
		}
		else {
			printf( "[DSMCCDemuxer] Warning: No callback attached, message %x ignored\n", msgID );
		}
    }
}

// Parse Download Info Indication
void DSMCCDemuxer::parseDII( BYTE *dsmccPayload, SIZE_T dsmccLenPayload ) {
    Buffer privateData;
	module::Modules modules;
	compatibility::Descriptors compatibilities;	
    int offset = 0;

	DWORD downloadID = RDW(dsmccPayload,offset);
    WORD blockSize   = RW(dsmccPayload,offset);

    //  Fields pre-assigned windowSize + ackPeriod + tcDownloadWindow + tcDownloadScenario
    offset += 1+1+4+4;

	//	Parse Compatibility descriptors
	offset += compatibility::parse( dsmccPayload+offset, dsmccLenPayload-offset, compatibilities );		

	//	Parse modules
	offset += module::parse( dsmccPayload+offset, dsmccLenPayload-offset, modules );

	//	Parse private data
    WORD privateDataLength = RW(dsmccPayload,offset);
    if (privateDataLength) {
        privateData.assign( (char *)dsmccPayload+offset, privateDataLength );
		offset += privateDataLength;		
    }

	DII *dii = new DII( downloadID, blockSize, compatibilities, modules, privateData );
	_onDII( dii );
}

// Parse Download Server Indication
void DSMCCDemuxer::parseDSI( BYTE *dsmccPayload, SIZE_T dsmccLenPayload ) {
    int offset = 0;
	compatibility::Descriptors compatibilities;	
    Buffer serverID, privateData;

    //  ServerID (20 bytes of 0xFF);
	serverID.assign( (char *)(dsmccPayload+offset), 20 );
    offset += 20;

	//	Parse compatibility descriptors
	offset += compatibility::parse( dsmccPayload+offset, dsmccLenPayload-offset, compatibilities );

	//	Get private data
    WORD privateDataLength = RW(dsmccPayload,offset);
    if (privateDataLength) {
        privateData.assign( (char *)dsmccPayload+offset, privateDataLength );
        offset += privateDataLength;		
    }

	//  Signal Data carousel listening
	DSI *dsi = new DSI( serverID, compatibilities, privateData );
	_onDSI( dsi );
}

//  Check DSM-CC header
SIZE_T DSMCCDemuxer::checkSection( BYTE *payload, SIZE_T len ) {
    SIZE_T size = 0;
    if (len >= PSI_GENERIC_HEADER) {
        BYTE adaptationLen = DSMCC_ADAPTATION_LEN(payload);
        SIZE_T calcSize = DSMCC_GENERIC_HEADER + adaptationLen;
        if (len >= calcSize) {
            size = calcSize;
        }
    }
	
	//printf( "[DSMCCDemuxer] check section header: size=%ld, len=%ld\n", size, len );
	
    return size;
}

//  Aux modules
Module *DSMCCDemuxer::find( DWORD downloadID, ID id, BYTE version, const std::list<Module *> &modules ) {
    Module *module=NULL;

	//	Try to start modules in pendig resources queue
	startModules();

	//	Find in listen queue
    BOOST_FOREACH( Module *mod, modules ) {
		if (mod->downloadID() == downloadID &&
			mod->id() == id &&
			mod->version() == version)
		{
            module=mod;
            break;
        }
    }
    return module;
}

void DSMCCDemuxer::clear( std::list<Module *> &modules ) {
    BOOST_FOREACH( Module *mod, modules ) {
        delete mod;
    }
    modules.clear();
}

void DSMCCDemuxer::startModules( void ) {
    std::list<Module *>::iterator it=_waitingResources.begin();
    while (it != _waitingResources.end()) {
        //  Can start?
        if ((*it)->canStartDownloading( _resMgr )) {
            //  Add to listen queue
            _listen.push_back( (*it) );

            //  Remove from queue
            it = _waitingResources.erase( it );
        }
        else {
            it++;
        }
    }
}

void DSMCCDemuxer::moduleComplete( Module *module ) {
	//	Show module info (DEBUG)
	module->show();

    //  Notify to consumer
    _onModule( module );

    //  Remove module from filter
    _listen.remove( module );
}

}
}
