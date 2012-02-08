/******************************************************************************
Este arquivo eh parte da implementacao do ambiente declarativo do middleware
Ginga (Ginga-NCL).

Direitos Autorais Reservados (c) 1989-2007 PUC-Rio/Laboratorio TeleMidia

Este programa eh software livre; voce pode redistribui-lo e/ou modificah-lo sob
os termos da Licenca Publica Geral GNU versao 2 conforme publicada pela Free
Software Foundation.

Este programa eh distribuido na expectativa de que seja util, porem, SEM
NENHUMA GARANTIA; nem mesmo a garantia implicita de COMERCIABILIDADE OU
ADEQUACAO A UMA FINALIDADE ESPECIFICA. Consulte a Licenca Publica Geral do
GNU versao 2 para mais detalhes.

Voce deve ter recebido uma copia da Licenca Publica Geral do GNU versao 2 junto
com este programa; se nao, escreva para a Free Software Foundation, Inc., no
endereco 59 Temple Street, Suite 330, Boston, MA 02111-1307 USA.

Para maiores informacoes:
ncl @ telemidia.puc-rio.br
http://www.ncl.org.br
http://www.ginga.org.br
http://www.telemidia.puc-rio.br
******************************************************************************
This file is part of the declarative environment of middleware Ginga (Ginga-NCL)

Copyright: 1989-2007 PUC-RIO/LABORATORIO TELEMIDIA, All Rights Reserved.
  Copyright 2010 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata


This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License version 2 as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License version 2 for more
details.

You should have received a copy of the GNU General Public License version 2
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

For further information contact:
ncl @ telemidia.puc-rio.br
http://www.ncl.org.br
http://www.ginga.org.br
http://www.telemidia.puc-rio.br
*******************************************************************************/

#ifndef FORMATTERLAYOUT_H_
#define FORMATTERLAYOUT_H_

#include "system/io/interface/output/ISurface.h"
using namespace ::br::pucrio::telemidia::ginga::core::system::io;

#include "ncl/layout/LayoutRegion.h"
using namespace ::br::pucrio::telemidia::ncl::layout;

#include "CascadingDescriptor.h"
#include "ExecutionObject.h"
using namespace ::br::pucrio::telemidia::ginga::ncl::model::components;

#include "FormatterRegion.h"
#include "FormatterDeviceRegion.h"

#include <map>
#include <string>
#include <set>
using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace model {
namespace presentation {
  class FormatterLayout {
	protected:
		set<string> typeSet;

	private:
		map<string, FormatterLayout*>* childs;
		LayoutRegion *deviceRegion;
		vector<string>* sortedRegions;
		map<string, int>* regionZIndex;
		map<string, set<FormatterRegion*>*>* regionMap;
		map<FormatterRegion*, ExecutionObject*>* objectMap;
		pthread_mutex_t mutex;

	public:
		FormatterLayout(int x, int y, int w, int h);
		virtual ~FormatterLayout();

		void addChild(string objectId, FormatterLayout* child);
		FormatterLayout* getChild(string objectId);
		string getBitMapRegionId();
		LayoutRegion* getNcmRegion(string regionId);

	private:
		void printRegionMap();
		void printObjectMap();

	public:
		bool getScreenShot(IWindow* region);

	private:
		void createDeviceRegion(int x, int y, int w, int h);

	public:
		ExecutionObject* getObject(int x, int y);

		void prepareFormatterRegion(
				ExecutionObject* object, ISurface* renderedSurface);

	private:
		void sortRegion(string regionId, int zIndex);

	public:
		void showObject(ExecutionObject* object);
		void hideObject(ExecutionObject* object);
		set<FormatterRegion*>* getFormatterRegionsFromNcmRegion(
			    string regionId);

		void clear();

	private:
		void lock();
		void unlock();
   };
}
}
}
}
}
}
}

#endif /*FORMATTERLAYOUT_H_*/
