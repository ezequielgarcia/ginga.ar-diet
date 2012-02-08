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

#include "../../../../../include/adapters/application/declarative/xhtml/LinksPlayerAdapter.h"
#include "../../../../../include/adapters/AdaptersComponentSupport.h"
#include <player/LinksPlayer.h>

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace adapters {
namespace application {
namespace xhtml {
	LinksPlayerAdapter::LinksPlayerAdapter(IPlayerAdapterManager* manager) :
		FormatterPlayerAdapter(manager) {

	}

	void LinksPlayerAdapter::createPlayer() {
		FormatterRegion* region;
		LayoutRegion* ncmRegion;
		CascadingDescriptor* descriptor;
		//int w, h, x, y;
		//string value, strW, strH, strX, strY;
		int w,h,left,top,right,bottom;
		string value, strW, strH, strLeft, strTop, strBottom, strRight;

		if (mrl != "") {
			left= 0;
			top = 0;
			right = -1;
			bottom = -1;
			w = 1;
			h = 1;

			if (mrl.substr(0, 1) == "/") {
				mrl = updatePath(mrl);
			}
			if ((object == NULL) || ((descriptor = object->getDescriptor()) == NULL))
		                return;
			region = descriptor->getFormatterRegion();
			if (region != NULL) {
				ncmRegion = region->getLayoutRegion();
				left = (int)(ncmRegion->getAbsoluteLeft());
				top = (int)(ncmRegion->getAbsoluteTop());
				if (!isNaN(ncmRegion->getRight()))
                    right = (int)(ncmRegion->getRightInPixels());
                if (!isNaN(ncmRegion->getBottom()))
                    bottom = (int)(ncmRegion->getBottomInPixels());
				w = (int)(ncmRegion->getWidthInPixels());
				h = (int)(ncmRegion->getHeightInPixels());
			}

#if HAVE_COMPSUPPORT
			playerCompName = "LinksPlayer";
			player = ((PlayerCreator*)(cm->getObject(playerCompName)))(
					mrl.c_str(), true);
#else
			player = new LinksPlayer(mrl.c_str());
#endif
/* FIX - se vuelven a convertir en string, para q luego se vuelva a
   convertir en integer en el setPropertyValue
*/
			strLeft = itos(left);
			strTop = itos(top);
			strRight = itos(right);
			strBottom = itos(bottom);
			strW = itos(w);
			strH = itos(h);
			this->setPropertyValue("left", strLeft);
			if (right != -1)
                this->setPropertyValue("right", strRight);
			this->setPropertyValue("top", strTop);
			if (bottom != -1)
                this->setPropertyValue("bottom", strBottom);
			this->setPropertyValue("width", strW);
			this->setPropertyValue("height", strH);

			value = descriptor->getParameterValue("transparency");
			if (value != "") {
				player->setPropertyValue("transparency", value);

			} else {
				player->setPropertyValue("transparency", "1");
			}
		}
		FormatterPlayerAdapter::createPlayer();
	}

    bool LinksPlayerAdapter::setPropertyValue(
		    AttributionEvent* event, string value, Animation* animation) {
//Ahora se inviertó el orden de llamada, primero al formatter y dsp al this.
		bool res = FormatterPlayerAdapter::setPropertyValue(event, value, animation);
		string propName = (event->getAnchor())->getPropertyName();
		value = trim(value);
		setPropertyValue( propName, value);
		return res;
	}

	void LinksPlayerAdapter::setPropertyToPlayer(string propertyName, string propertyValue){
	    //Sin llamar al formatter igual inicializa bien, ¿de donde lo está llamando?
	    //-> Lo esta llamando en el FormatterPLayerAdapter::setPropertyValue()
		FormatterPlayerAdapter::setPropertyToPlayer(propertyName, propertyValue);
	    setPropertyValue(propertyName, propertyValue);
    }

    void LinksPlayerAdapter::setPropertyValue(string propName, string value){
        /*
        El código que setea los valroes en region se comenta, xq tmb lo esta seteando
        el ExecutionObject (dinamico y estatico), el problema es que los valores de aca los lee antes de
        que el ExecutionObject los haya seteado, por lo tanto estan desfasado.
        -> El problema es q llama dsp al del formatterplayer adapter
        */

        if (isPositionProperty(propName) and value != "") {
			if (player != NULL) {
			    LinksPlayer* linksPlayer = dynamic_cast<LinksPlayer*> (player);
				LayoutRegion* ncmRegion;
				ncmRegion = object->getDescriptor()->getFormatterRegion()->getLayoutRegion();
                value=trim(value);
                propName = trim(propName);
                if (propName == "bounds"){
                    vector<string>* splited = split(value, ",");
                    if (splited->size() == 4 ){
                        setPropertyValue("left", (*splited)[0]);
                        setPropertyValue("top", (*splited)[1]);
                        setPropertyValue("width", (*splited)[2]);
                        setPropertyValue("height", (*splited)[3]);
                        delete splited;
                    }
                }
                else if (propName == "size"){
                    vector<string>* splited = split(value, ",");
                    if (splited->size() == 2){
                        setPropertyValue("width", (*splited)[0]);
                        setPropertyValue("height", (*splited)[1]);
                        delete splited;
                    }
                }
                else if (propName == "location"){
                    vector<string>* splited = split(value, ",");
                    if (splited->size() == 2){
                        setPropertyValue("left", (*splited)[0]);
                        setPropertyValue("top", (*splited)[1]);
                    }
                    delete splited;
                }
                else{
			 if (propName == "left"){
	                  /*  if (isPercentualValue(value))
	                        ncmRegion->setTargetLeft(getPercentualValue(value), true);
	                    else
	                        ncmRegion->setTargetLeft(stof(value), false);
			*/
	                    linksPlayer->setLeft(ncmRegion->getAbsoluteLeft());
	                    linksPlayer->setW(ncmRegion->getWidthInPixels());
	                    linksPlayer->updateBounds();
        	        }
			else if (propName == "right"){
	/*
		            if (isPercentualValue(value))
		                ncmRegion->setTargetRight(getPercentualValue(value), true);
		            else
		                ncmRegion->setTargetRight(stof(value), false);
	*/
		            linksPlayer->setRight(ncmRegion->getParent()->getWidthInPixels() - ncmRegion->getRightInPixels());
		            linksPlayer->setW(ncmRegion->getWidthInPixels());
		            linksPlayer->updateBounds();
		        }
		        else if ( propName == "top"){
	/*
		            if (isPercentualValue(value))
		                ncmRegion->setTargetTop(getPercentualValue(value), true);
		            else
		                ncmRegion->setTargetTop(stof(value), false);
	*/
		            linksPlayer->setTop(ncmRegion->getAbsoluteTop());
		            linksPlayer->setH(ncmRegion->getHeightInPixels());
		            linksPlayer->updateBounds();
		        }
		        else if  ( propName == "bottom"){
	/*
		            if (isPercentualValue(value))
		                ncmRegion->setTargetBottom(getPercentualValue(value), true);
		            else
		                ncmRegion->setTargetBottom(stof(value), false);
	*/
		            linksPlayer->setBottom(ncmRegion->getBottomInPixels());
		            linksPlayer->updateHeight(ncmRegion->getHeightInPixels());
		            //linksPlayer->updateBounds();
		        }
		        else if (propName == "width"){
	/*
		            if (isPercentualValue(value))
		                ncmRegion->setTargetWidth(getPercentualValue(value), true);
		            else
		                ncmRegion->setTargetWidth(stof(value), false);
	*/
		            linksPlayer->updateWidth(ncmRegion->getWidthInPixels());
		        }
		        else if (propName == "height"){
	/*
		            if (isPercentualValue(value))
		                ncmRegion->setTargetHeight(getPercentualValue(value), true);
		            else
		                ncmRegion->setTargetHeight(stof(value), false);
	*/
		            linksPlayer->updateHeight(ncmRegion->getHeightInPixels());
		        }
		}
            }
	}
    }
}
}
}
}
}
}
}
}

extern "C" ::br::pucrio::telemidia::ginga::ncl::adapters::IPlayerAdapter*
		createLksAdapter(IPlayerAdapterManager* manager, void* param) {

	return new ::br::pucrio::telemidia::ginga::ncl::adapters::
			application::xhtml::LinksPlayerAdapter(manager);
}

extern "C" void destroyLksAdapter(
		::br::pucrio::telemidia::ginga::ncl::adapters::IPlayerAdapter* player) {

	return delete player;
}
