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

#include "../../../../include/LinksPlayer.h"

#include "../../../../include/PlayersComponentSupport.h"

#include "system/util/functions.h"
using namespace ::util;

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace core {
namespace player {
	IInputManager* LinksPlayer::im = NULL;

	LinksPlayer::LinksPlayer(string mrl) : Player(mrl), Thread::Thread() {
		ILocalDeviceManager* dm = NULL;

		mBrowser = NULL;
		this->x = 1;
		this->y = 1;
		this->w = 1;
		this->h = 1;
		left = top = 0;
		right = bottom = -1;// Indica que no están seteados

#if HAVE_COMPSUPPORT
		dm = ((LocalDeviceManagerCreator*)(cm->getObject(
				"LocalDeviceManager")))();

		if (im == NULL) {
			im = ((InputManagerCreator*)(cm->getObject("InputManager")))();
		}
#else
		dm = LocalDeviceManager::getInstance();
		im = InputManager::getInstance();
#endif

		if (dm != NULL) {
			setBrowserDFB(dm->getGfxRoot());
			setDisplayMenu(0);
		}

		mBrowser = openBrowser(x, y, w, h);
		hasBrowser = false;
	}

	LinksPlayer::~LinksPlayer() {
		cout << "LinksPlayer::~LinksPlayer " << endl;
		im->removeInputEventListener(this);

		if (hasBrowser) {
			_DFBSurfaceMutexSingleton.instance().lock();
			cout << "LinksPlayer::~LinksPlayer hasBrowser" << endl;
			closeBrowser(mBrowser);
			mBrowser = NULL;
			cout << "LinksPlayer::~LinksPlayer browser closed" << endl;
			_DFBSurfaceMutexSingleton.instance().unlock();
		}
	}

	void LinksPlayer::setNotifyContentUpdate(bool notify) {
		if (notify) {
			setGhostBrowser(mBrowser);
		}
		Player::setNotifyContentUpdate(notify);
	}

	void LinksPlayer::setBounds(int x, int y, int w, int h) {
		void* s;

		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;

		if (!hasBrowser) {
			//mBrowser = openBrowser(x, y, w, h);			
			loadUrlOn(mBrowser, mrl.c_str());
			_DFBSurfaceMutexSingleton.instance().lock();
			browserResizeCoord(mBrowser, x, y, w, h);
			_DFBSurfaceMutexSingleton.instance().unlock();
			if (mBrowser != NULL) {
				s = browserGetSurface(mBrowser);
				if (s != NULL) {
#if HAVE_COMPSUPPORT
					this->surface = ((SurfaceCreator*)(cm->getObject(
							"Surface")))(s, 0, 0);
#else
					this->surface = new DFBSurface(s);
#endif
				}
			}
			hasBrowser = true;
		}
	}

    void LinksPlayer::updateBounds(int x , int y, int w, int h) {
//		closeBrowser(mBrowser);
//		mBrowser = NULL;
//		mBrowser = openBrowser(x, y, w, h);
        if (!hasBrowser)
            setBounds(x ,y ,w , h);
        else{
            this->x = x;
            this->y = y;
            this->w = w;
            this->h = h;
	    _DFBSurfaceMutexSingleton.instance().lock();
            browserResizeCoord(mBrowser, x, y, w, h);
	    _DFBSurfaceMutexSingleton.instance().unlock();
        }
		//browserRequestFocus(mBrowser);
		/*parent = (Window*)(surface->getParent());
		if (parent != NULL) {
			cout << " UPDATE RENDER" << endl;
			parent->renderFrom(surface);
			parent->validate();

		} else {
			cout << " UPDATE PARENT IS NULL" << endl;
		}*/
	}

    void LinksPlayer::updateBounds(){
        if (!hasBrowser)
            setBounds(x ,y ,w , h);
        else{
	    _DFBSurfaceMutexSingleton.instance().lock();
            browserResizeCoord(mBrowser, x, y, w, h);
	    _DFBSurfaceMutexSingleton.instance().unlock();
	}
    }

    void LinksPlayer::setLeft(int newLeft){
        x = newLeft;
        left = newLeft;
    }

    void LinksPlayer::setRight(int newRight){
        right = newRight;
        if (left == 0)
            x = 0; //dsp el right se establece con el width
                  //No permite tener el browser fuear de pàntalla
    }

    void LinksPlayer::setTop(int newTop){
        top = newTop;
        y = newTop;
    }

    void LinksPlayer::setBottom(int newBottom){
        bottom = newBottom;
        if (top == 0)
            y = 0;
    }

    void LinksPlayer::setW(int newWidth){
        /*con rigth seteado y un width menor
         el media se debe pegar a la parte derecha
         de la pantalla
        */
        w = newWidth;
    }

    void LinksPlayer::setH(int newHeigth){
            h = newHeigth;
    }

    void LinksPlayer::updateWidth(int newWidth) {
        //si tiene seteado un right y no un left, y achica el width
        //debe pegar el midia contra la parte derecha.
        if ((right >= 0) and (left == 0) and (newWidth < w))
            x = w - newWidth;
        updateBounds(x, y, newWidth, h);
    }

    void LinksPlayer::updateHeight(int newHeigth) {
        //si tiene seteado un bottom y no un top, y achica el height
        //debe pegar el midia contra la parte derecha.
        if ((bottom >= 0) and (top == 0) and (newHeigth < h))
            y = h - newHeigth;
        updateBounds(x, y, w, newHeigth);
    }

	void LinksPlayer::play() {
	 /*Comentado para que no s muestre 2 veces el html */
        //browserShow(mBrowser);
     /*FIN - Comentado para que no s muestre 2 veces el html */

		//browserResizeCoord(mBrowser, x, y, w, h);
		//loadUrlOn(mBrowser, mrl.c_str());
		//browserRequestFocus(mBrowser);

		Thread::start();
		//::usleep(3000000);

		Player::play();
	}

	void LinksPlayer::stop() {
		if (hasBrowser) {
			closeBrowser(mBrowser);
			mBrowser = NULL;
			hasBrowser = false;
		}
		Player::stop();
	}

	void LinksPlayer::setPropertyValue(
			string name, string value, double duration, double by) {

		//TODO: set scrollbar, support...
		if (name == "transparency") {
			double val;

			val = stof(value);
			if (val >= 0.0 && val <= 1.0) {
				browserSetAlpha((int)(val * 0xFF), mBrowser);
			}

       /* } else if (name == "bounds") {
			int x, y, w, h;
			vector<string>* params;

			params = split(value, ",");
			if (params->size() != 4) {
				delete params;
				return;
			}

			x = (int)stof((*params)[0]);
			y = (int)stof((*params)[1]);
			w = (int)stof((*params)[2]);
			h = (int)stof((*params)[3]);

			delete params;

            updateBounds(x, y, w, h);

			return;*/
		}else if (name == "src") {
			if (value != "") {
				mrl = value;				
				loadUrlOn(mBrowser, mrl.c_str());
				sleep(5);
				_DFBSurfaceMutexSingleton.instance().copyToSD();				
			}
		}

		Player::setPropertyValue(name, value, duration, by);
	}

	bool LinksPlayer::setKeyHandler(bool isHandler) {
		if (isHandler && notifyContentUpdate) {
			im->addInputEventListener(this, NULL);

		} else {
			im->removeInputEventListener(this);
		}

		browserSetFocusHandler((int)isHandler, mBrowser);
		return isHandler;
	}

	bool LinksPlayer::userEventReceived(IInputEvent* userEvent) {
		browserReceiveEvent(mBrowser, (void*)(userEvent->getContent()));
		return true;
	}

	void LinksPlayer::run() {
		IWindow* parent;

		::usleep(1000000);
		if (surface != NULL) {
			parent = (IWindow*)(surface->getParent());
			if (parent != NULL) {
				parent->renderFrom(surface);
				browserSetFlipWindow(mBrowser, parent->getContent());
				sleep(5);
				_DFBSurfaceMutexSingleton.instance().copyToSD();
			}
		}

		if (notifyContentUpdate) {
			while (status == PLAY || status == PAUSE) {
				notifyListeners(
						PL_NOTIFY_UPDATECONTENT, "", TYPE_PASSIVEDEVICE);

				this->usleep(65);
			}
		}
	}
}
}
}
}
}
}


extern "C" ::br::pucrio::telemidia::ginga::core::player::IPlayer*
		createLinksPlayer(const char* mrl, bool hasVisual) {

	return new ::br::pucrio::telemidia::ginga::core::player::LinksPlayer(
			(string)mrl);
}

extern "C" void destroyLinksPlayer(
		::br::pucrio::telemidia::ginga::core::player::IPlayer* p) {

	delete p;
}
