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

#include "../../include/gfx/GingaNclGfx.h"

#include "generated/config.h"

#if HAVE_COMPSUPPORT
#include "cm/IComponentManager.h"
using namespace ::br::pucrio::telemidia::ginga::core::cm;
#else
#include <system/io/interface/output/dfb/DFBWindow.h>
#include <system/io/interface/output/dfb/DFBSurface.h>
#include <system/io/interface/content/text/DFBFontProvider.h>
#include <player/ImagePlayer.h>
#endif

#include <system/io/interface/content/text/IFontProvider.h>
#include <system/io/interface/output/ISurface.h>

#include <system/util/Color.h>
using namespace ::util;

#include <player/IPlayer.h>
using namespace ::br::pucrio::telemidia::ginga::core::player;

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
	int GingaNclGfx::w = 0;
	int GingaNclGfx::h = 0;
	vector<AnimePlayer*>* GingaNclGfx::animes = NULL;
	IWindow* GingaNclGfx::bg = NULL;
	IWindow* GingaNclGfx::top = NULL;
	IWindow* GingaNclGfx::bottom = NULL;
#if HAVE_COMPSUPPORT
	static IComponentManager* cm = IComponentManager::getCMInstance();
#endif

	void GingaNclGfx::show(int l, int t, int w, int h) {
		int compRollerW, compRollerH, initX, initY;
		unsigned int i, size;
		double rate;
		vector<int> x;
		vector<int> y;
		AnimePlayer* p;
		IWindow* win;
		ISurface* s;
		vector<string>* mrls, *mrlsAux;

		release();

		GingaNclGfx::w = w;
		GingaNclGfx::h = h;
		rate = 0.2;
		initX = (w/2) - ((550 * rate)/2);
		initY = (h/2) - ((530 * rate)/2);

		x.push_back(initX + (130 * rate));
		x.push_back(initX + (240 * rate));
		x.push_back(initX + (340 * rate));
		x.push_back(initX + (400 * rate));
		x.push_back(initX + (340 * rate));
		x.push_back(initX + (240 * rate));
		x.push_back(initX + (130 * rate));
		x.push_back(initX + (60 * rate));

		y.push_back(initY + (90 * rate));
		y.push_back(initY + (50 * rate));
		y.push_back(initY + (90 * rate));
		y.push_back(initY + (190 * rate));
		y.push_back(initY + (330 * rate));
		y.push_back(initY + (380 * rate));
		y.push_back(initY + (330 * rate));
		y.push_back(initY + (190 * rate));

		compRollerW = 115 * rate;
		compRollerH = 113 * rate;

		mrls = new vector<string>;
		mrls->push_back(PREFIX_PATH "/ncl-presenter/img/roller/b1.png");
		mrls->push_back(PREFIX_PATH "/ncl-presenter/img/roller/b2.png");
		mrls->push_back(PREFIX_PATH "/ncl-presenter/img/roller/b3.png");
		mrls->push_back(PREFIX_PATH "/ncl-presenter/img/roller/b4.png");
		mrls->push_back(PREFIX_PATH "/ncl-presenter/img/roller/b5.png");
		mrls->push_back(PREFIX_PATH "/ncl-presenter/img/roller/b6.png");
		mrls->push_back(PREFIX_PATH "/ncl-presenter/img/roller/b7.png");
		mrls->push_back(PREFIX_PATH "/ncl-presenter/img/roller/b8.png");

		animes = new vector<AnimePlayer*>;

#if HAVE_COMPSUPPORT
		bg = ((WindowCreator*)(cm->getObject("Window")))(l, t, w, h);
		s = ((ImageRenderer*)(cm->getObject("ImageRenderer")))(
				(char*)(PREFIX_PATH "/ncl-presenter/img/roller/loading.png"));
#else
		bg = new DFBWindow(l, t, w, h);
		s = ImagePlayer::renderImage(
				PREFIX_PATH "/ncl-presenter/img/roller/loading.png");
#endif

		bg->setCaps(bg->getCap("ALPHACHANNEL"));
		bg->draw();
		bg->show();
		bg->raiseToTop();

		bg->renderFrom(s);

		delete s;
		s = NULL;

#if HAVE_COMPSUPPORT
		bottom = ((WindowCreator*)(cm->getObject("Window")))(
				5, h - 30, w / 2, 30);
#else
		bottom = new DFBWindow(
				5, h - 30, w / 2, 30);
#endif

		bottom->setCaps(bottom->getCap("ALPHACHANNEL"));
		bottom->draw();
		bottom->setBackgroundColor(0, 0, 0, 0);
		bottom->show();
		bottom->raiseToTop();

#if HAVE_COMPSUPPORT
		top = ((WindowCreator*)(cm->getObject("Window")))(5, 5, w / 2, 30);
#else
		top = new DFBWindow(5, 5, w / 2, 30);
#endif

		top->setCaps(top->getCap("ALPHACHANNEL"));
		top->draw();
		top->setBackgroundColor(0, 0, 0, 0);
		top->show();
		top->raiseToTop();

		size = mrls->size();
		for (i = 0; i < size; i++) {
			mrlsAux = new vector<string>(*mrls);
			p = createAnimePlayer(mrlsAux);

#if HAVE_COMPSUPPORT
			win = ((WindowCreator*)(cm->getObject("Window")))(
					x[i], y[i], compRollerW, compRollerH);
#else
			win = new DFBWindow(
					x[i], y[i], compRollerW, compRollerH);
#endif

			win->setCaps(win->getCap("ALPHACHANNEL"));
			win->draw();
			win->show();
			win->raiseToTop();

			p->setVoutWindow(win);
			animes->push_back(p);
			roll(mrls);
		}

		delete mrls;
		mrls = NULL;

		for (unsigned int i = 0; i < size; i++) {
			((*animes)[i])->play();
		}
	}

	void GingaNclGfx::roll(vector<string>* mrls) {
		mrls->insert(mrls->begin(), (*mrls)[mrls->size() - 1]);
		mrls->pop_back();
	}

	AnimePlayer* GingaNclGfx::createAnimePlayer(vector<string>* mrls) {
		AnimePlayer* ap;

		ap = new AnimePlayer(mrls);
		delete mrls;
		mrls = NULL;
		return ap;
	}

	void GingaNclGfx::release() {
		AnimePlayer* p;
		vector<AnimePlayer*>::iterator i;

		if (animes != NULL) {
			i = animes->begin();
			while (i != animes->end()) {
				p = *i;
				p->stop();
				delete p;
				++i;
			}

			delete animes;
			animes = NULL;
		}

		if (bg != NULL) {
			bg->clearContent();
			bg->hide();
			delete bg;
			bg = NULL;
		}

		if (top != NULL) {
			top->clearContent();
			top->hide();
			delete top;
			top = NULL;
		}

		if (bottom != NULL) {
			bottom->clearContent();
			bottom->hide();
			delete bottom;
			bottom = NULL;
		}
	}

	void GingaNclGfx::showTop(string text) {
		if (animes == NULL || bg == NULL) {
			return;
		}

		showText(top, text);
	}

	void GingaNclGfx::showBottom(string text) {
		if (animes == NULL || bg == NULL) {
			return;
		}

		showText(bottom, text);
	}

	void GingaNclGfx::showText(IWindow* window, string text) {
		ISurface* s;
		IColor* fontColor;
		IColor* bgColor;
		string fontUri;
		int fontSize;

		fontUri = PREFIX_PATH "/player/fonts/decker.ttf";
		fontSize = 12;

		if (!fileExists(fontUri)) {
			cout << "GingaNclGfx::show Warning! File not found: '";
			cout << fontUri.c_str() << "'" << endl;
			return;
		}

		IFontProvider* font = NULL;

		window->clearContent();
		window->show();

#if HAVE_COMPSUPPORT
		font = ((FontProviderCreator*)(cm->getObject("FontProvider")))(
				fontUri.c_str(), fontSize);

		s = ((SurfaceCreator*)(cm->getObject("Surface")))(NULL, 0, 0);
#else
		font = new DFBFontProvider(fontUri.c_str(), fontSize);
		s = new DFBSurface();
#endif

		s->setParent(window);

		bgColor = new Color("black");
		fontColor = new Color("white");

		s->setFont(font);
		s->setColor(fontColor);

		font->playOver(
				(void*)s,
				text.c_str(), 0, 0, (0x00000004 | 0x00000000));

		delete font;
		font = NULL;

#if HAVE_COMPSUPPORT
		cm->releaseComponentFromObject("FontProvider");
#endif

		delete bgColor;
		bgColor = NULL;

		delete s;
		s = NULL;


		delete fontColor;
		fontColor = NULL;

		window->validate();
		window->raiseToTop();
	}
}
}
}
}
}
