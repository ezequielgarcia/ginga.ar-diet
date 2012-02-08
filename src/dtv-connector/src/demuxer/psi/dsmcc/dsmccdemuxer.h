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

#include "dsi.h"
#include "dii.h"
#include "../psidemuxer.h"
#include <list>

namespace tuner {

class ResourceManager;

namespace dsmcc {

class Module;

class DSMCCDemuxer : public PSIDemuxer {
public:
    DSMCCDemuxer( ID pid, ResourceManager *resMgr );
    virtual ~DSMCCDemuxer( void );

	//	Getters
	ResourceManager *resourceMgr();
	virtual WORD maxSectionBytes() const;
	virtual DWORD frequency() const;
	virtual DWORD timeout() const;	

    //  Callback called when DSI parsed
    typedef boost::function<void ( DSI *dsi )> DSICallback;
    void onDSI( const DSICallback &callback );

    //  Callback called when DII parsed	
    typedef boost::function<void ( DII *dii )> DIICallback;
    void onDII( const DIICallback &callback );
	
	//	Filter module
	bool filterModules( DWORD downloadID, const module::Modules &modules, WORD blockSize );	

    //  Callback called when a Module was parsed
    typedef boost::function<void ( Module *module )> ModuleCallback;
    void onModule( const ModuleCallback &callback );

protected:
    //  PSI demuxer virtual methods
    virtual bool useStandardVersion() const;
    virtual void onSection( BYTE *section, SIZE_T len );

    //  Parse Download Data Messages
    void parseDownloadDataMessages( BYTE *section, SIZE_T len );
    void parseDDB( BYTE *dsmccPayload, SIZE_T dsmccLenPayload );

    //  Parse Download Control Messages
	DWORD findControlMessage( DWORD tID );
    void parseDownloadControlMessages( BYTE *section, SIZE_T len );
    void parseDII( BYTE *dsmccPayload, SIZE_T dsmccLenPayload );
    void parseDSI( BYTE *dsmccPayload, SIZE_T dsmccLenPayload );

    //  Check DSM-CC header
    SIZE_T checkSection( BYTE *payload, SIZE_T len );

    //  Aux modules
	bool filterModule( DWORD downloadID, const module::Type &moduleInfo, WORD blockSize );
    Module *find( DWORD downloadID, ID id, BYTE version, const std::list<Module *> &modules );
    void clear( std::list<Module *> &modules );
    void moduleComplete( Module *mod );
    void startModules( void );

private:
	ResourceManager *_resMgr;	
    std::list<Module *> _listen;             //  Listening modules
    std::list<Module *> _waitingResources;   //  Waiting for resources
	std::vector<DWORD>  _controlMessages;    //	List of Control Messages processed (Transaction ID)
    DSICallback _onDSI;
	DIICallback _onDII;
	ModuleCallback _onModule;
};

}
}
