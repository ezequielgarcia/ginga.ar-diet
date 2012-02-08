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

#include "player/INCLPlayer.h"
using namespace ::br::pucrio::telemidia::ginga::core::player;

#include "../../../../../include/adapters/application/declarative/ncl/NCLPlayerAdapter.h"

#include "../../../../../include/adapters/AdaptersComponentSupport.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace adapters {
namespace application {
namespace ncl {
	NCLPlayerAdapter::NCLPlayerAdapter(IPlayerAdapterManager* manager) :
		ApplicationPlayerAdapter(manager)  {

	}

	void NCLPlayerAdapter::createPlayer() {
		FormatterRegion* region;
		LayoutRegion* ncmRegion;
		CascadingDescriptor* descriptor;
		int w, h, x, y, devClass;
		ISurface* s;
		NclPlayerData* childData, *playerData;

		player = NULL;

		if (fileExists(mrl)) {
			descriptor = object->getDescriptor();
			if (descriptor == NULL) {
				cout << "NCLPlayerAdapter::createPlayer Warning! ";
				cout << "descriptor not found for: '" << mrl << "'" << endl;
				return;
			}

			region = descriptor->getFormatterRegion();
			if (region != NULL) {
				ncmRegion      = region->getLayoutRegion();
				x              = (int)(ncmRegion->getAbsoluteLeft());
				y              = (int)(ncmRegion->getAbsoluteTop());
				w              = (int)(ncmRegion->getWidthInPixels());
				h              = (int)(ncmRegion->getHeightInPixels());

				devClass       = ncmRegion->getDeviceClass();
				playerCompName = "Formatter";
				playerData     = manager->getNclPlayerData();

				cout << "NCLPlayerAdapter::createPlayer for '" << mrl;
				cout << "', devClass '" << devClass << "',  x = '" << x;
				cout << "', y = '" << y << "', w = '" << w << "', h = '";
				cout << h << "' parentDocId = '" << playerData->docId << "'";
				cout << endl;

				childData                    = new NclPlayerData;
				childData->baseId            = playerData->baseId;
				childData->playerId          = object->getId();
				childData->devClass          = devClass;
				childData->x                 = x;
				childData->y                 = y;
				childData->w                 = w;
				childData->h                 = h;
				childData->enableGfx         = false;
				childData->parentDocId       = playerData->docId;
				childData->nodeId            = ((NodeEntity*)(
						object->getDataObject()->getDataEntity()))->getId();

				childData->docId             = "";
				childData->focusManager      = playerData->focusManager;

#if HAVE_COMPSUPPORT
				player = ((NCLPlayerCreator*)(cm->getObject(playerCompName)))(
						childData);

				s = ((SurfaceCreator*)(cm->getObject("Surface")))(
						NULL, 0, 0);
#else
				player = (INCLPlayer*)(new FormatterMediator(childData));
				s = new DFBSurface();
#endif

				s->setCaps(s->getCap("ALPHACHANNEL"));

				player->setSurface(s);
				if (((INCLPlayer*)player)->setCurrentDocument(mrl) == NULL) {
					cout << "NCLPlayerAdapter::createPlayer Warning! ";
					cout << "can't set '" << mrl << "' as document";
					cout << endl;
				}
				((INCLPlayer*)player)->setParentLayout(
						region->getLayoutManager());
			}

		} else {
			cout << "NCLPlayerAdapter::createPlayer Warning! ";
			cout << "file not found: '" << mrl << "'" << endl;
		}

		FormatterPlayerAdapter::createPlayer();
	}

	void NCLPlayerAdapter::setCurrentEvent(FormatterEvent* event) {
		string interfaceId;

		lockEvent();
		if (event == NULL) {
			unlockEvent();
			return;
		}

		if (preparedEvents->count(event->getId()) != 0 &&
				!event->instanceOf("SelectionEvent") &&
				event->instanceOf("AnchorEvent")) {

			interfaceId = ((AnchorEvent*)event)->getAnchor()->getId();

			if ((((AnchorEvent*)event)->getAnchor())->instanceOf(
					"LabeledAnchor")) {

				interfaceId = ((LabeledAnchor*)((AnchorEvent*)event)->
						getAnchor())->getLabel();

			} else if ((((AnchorEvent*)event)->getAnchor())->instanceOf(
					"LambdaAnchor")) {

				interfaceId = "";
			}

			currentEvent = event;
			((ApplicationExecutionObject*)object)->setCurrentEvent(
					currentEvent);

			player->setCurrentScope(interfaceId);

		} else if (event->instanceOf("AttributionEvent")) {
			interfaceId = ((AttributionEvent*)
					event)->getAnchor()->getPropertyName();

			player->setScope(interfaceId, IPlayer::TYPE_ATTRIBUTION);

			currentEvent = event;
			((ApplicationExecutionObject*)object)->setCurrentEvent(
					currentEvent);

			player->setCurrentScope(interfaceId);
		}
		unlockEvent();
	}

	void NCLPlayerAdapter::flip() {
		((INCLPlayer*)player)->flip();
	}
}
}
}
}
}
}
}
}

using namespace ::br::pucrio::telemidia::ginga::ncl::adapters;

extern "C" IPlayerAdapter* createNCLAdapter(
		IPlayerAdapterManager* manager, void* param) {

	return new application::ncl::NCLPlayerAdapter(manager);
}

extern "C" void destroyNCLAdapter(IPlayerAdapter* player) {
	return delete player;
}
