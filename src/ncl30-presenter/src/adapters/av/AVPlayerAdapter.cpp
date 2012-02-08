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

#include "../../../include/adapters/av/AVPlayerAdapter.h"

#include "../../../include/adapters/AdaptersComponentSupport.h"

#include <player/mainavaplayer.h>
#include <string.h>
#include <ncl/layout/LayoutRegion.h>

#include "generated/config.h"
namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace adapters {
namespace av {
	AVPlayerAdapter::AVPlayerAdapter(
			IPlayerAdapterManager* manager, bool hasVisual)
				: FormatterPlayerAdapter(manager) {

		this->hasVisual = hasVisual;
		_isMainAV = false;
		typeSet.insert("AVPlayerAdapter");
	}

	void AVPlayerAdapter::createPlayer() {
		CascadingDescriptor* descriptor;
		string soundLevel;


		if (mrl != "") {
			_isMainAV = mrl.substr(0, 11) == "sbtvd-ts://";
			if (_isMainAV){
				player = new MainAVPlayer(mrl.c_str());
				cout << "MainAVPlayer created." <<endl;
			}else{
				//TODO cambiar para saber si el provider tiene soporte para av.
				//Usamos XINE para indicar que no se está compilando para la plataforma ST.
				#if HAVE_XINE
				player = new AVPlayer(mrl.c_str(), hasVisual);
				cout << "AVPlayer created." <<endl;
				#endif
			}
		}

		if (object != NULL && player != NULL) {
			descriptor = object->getDescriptor();
			if (descriptor != NULL) {
				soundLevel = descriptor->getParameterValue("soundLevel");
				if (soundLevel == "") {
					soundLevel = "1.0";
				}
				player->setPropertyValue("soundLevel", soundLevel);
			}
		}

		FormatterPlayerAdapter::createPlayer();

		//Se llama para setear en el mainAVVideo las propiedades estaticas.
		setPlayerProperties(NULL);
	}

	void AVPlayerAdapter::setPlayerProperties(AttributionEvent* event){

		if (player == NULL || object == NULL) {
			return;
		}

		string propName = "nullEvent";

		if (event != NULL){
			 propName = event->getAnchor()->getPropertyName();
		}

		if (propName ==  "nullEvent" || propName == "size" || propName == "location" ||
			propName == "bounds" ||
			propName == "top" || propName == "left" ||
			propName == "bottom" || propName == "right" ||
			propName == "width" || propName == "height") {

            FormatterRegion* fr = object->getDescriptor()->getFormatterRegion();
            if (fr == NULL){ //Puede haber medias de audio sin ninguna region asociada
                cout << "[AVPlayerAdapter] WARNING: Media Object Without Region" << endl;
            } else{
                LayoutRegion* ncmRegion = fr->getLayoutRegion();
                LayoutRegion* parent = ncmRegion->getParent();

                //PARCHE hasta resolver herencia multiple de player
                if (_isMainAV){
                    ((MainAVPlayer*)player)->setVoutWindow(
                         ncmRegion->getAbsoluteLeft()    /  (float)(parent->getWidthInPixels()),
                         ncmRegion->getAbsoluteTop()    /  (float)(parent->getHeightInPixels()),
                         ncmRegion->getWidthInPixels()  /  (float)(parent->getWidthInPixels()),
                         ncmRegion->getHeightInPixels() /  (float)(parent->getHeightInPixels())
                    );
                }else{
                    ((AVPlayer*)player)->setVoutWindow(ncmRegion->getAbsoluteLeft(),
                        ncmRegion->getAbsoluteTop(),
                        ncmRegion->getWidthInPixels(),
                        ncmRegion->getHeightInPixels());
                }
            }
		}
	}

	bool AVPlayerAdapter::setPropertyValue(
		    AttributionEvent* event,
		    string value,
		    Animation* animation) {


		bool result = FormatterPlayerAdapter::setPropertyValue(event, value, animation);
		setPlayerProperties(event);
		return result;

	}

	bool AVPlayerAdapter::getHasVisual() {
		return this->hasVisual;
	}
}
}
}
}
}
}
}

extern "C" ::br::pucrio::telemidia::ginga::ncl::adapters::IPlayerAdapter*
		createAaVAdapter(IPlayerAdapterManager* manager, void* param) {

	bool hasVisual = false;
	if (strcmp("true", (char*)param) == 0) {
		hasVisual = true;
	}
	return new ::br::pucrio::telemidia::ginga::ncl::adapters::av::
			AVPlayerAdapter(manager, hasVisual);
}

extern "C" void destroyAaVAdapter(
		::br::pucrio::telemidia::ginga::ncl::adapters::IPlayerAdapter* player) {

	return delete player;
}
