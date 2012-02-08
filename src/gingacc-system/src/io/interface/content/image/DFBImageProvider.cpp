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

#include "../../../../../include/util/Color.h"

#include "../../../../../include/io/interface/content/image/DFBImageProvider.h"
#include "../../../../../include/io/interface/output/dfb/DFBWindow.h"
#include "../../../../../include/io/interface/output/dfb/DFBSurface.h"
#include "../../../../../include/io/LocalDeviceManager.h"

/* macro for a safe call to DirectFB functions */
#ifndef DFBCHECK
#define DFBCHECK(x...)                                            \
{                                                                 \
	DFBResult err = x;                                            \
	if (err != DFB_OK) {                                          \
		fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );    \
	}                                                             \
}
#endif /*DFBCHECK*/

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace core {
namespace system {
namespace io {

	DFBImageProvider::DFBImageProvider() {
		_mrl = "";
	}
	
	DFBImageProvider::DFBImageProvider(string mrl) {
		_mrl = mrl;
	}

	DFBImageProvider::~DFBImageProvider() {
	}

	void* DFBImageProvider::getContent() {
		return NULL;
	}

	bool DFBImageProvider::isGif(){
		return (_mrl.length() > 4 && _mrl.substr(_mrl.length() - 4, 4) == ".gif");
	}

	void DFBImageProvider::applyCaps(ISurface* renderedSurface){
		IColor* chromaKey;
		if (_imageDsc.caps & DICAPS_ALPHACHANNEL) {
			if (isGif()) {
				chromaKey = new Color(0, 0, 0);
				renderedSurface->setChromaColor(chromaKey);
			} else {
				renderedSurface->setCaps(DWCAPS_ALPHACHANNEL);
			}
			DFBCHECK(_surface->SetBlittingFlags(_surface,
					(DFBSurfaceBlittingFlags)(DSBLIT_BLEND_ALPHACHANNEL)));
		}

		if (_imageDsc.caps & DICAPS_COLORKEY) {
			chromaKey = new Color( _imageDsc.colorkey_r, _imageDsc.colorkey_g, _imageDsc.colorkey_b);
			DFBCHECK(_surface->SetBlittingFlags(_surface,
				(DFBSurfaceBlittingFlags)(DSBLIT_BLEND_ALPHACHANNEL | DSBLIT_SRC_COLORKEY)));
			renderedSurface->setChromaColor(chromaKey);
		}

		if (_imageDsc.caps & DICAPS_NONE) {
			DFBCHECK(_surface->SetBlittingFlags(_surface,
				(DFBSurfaceBlittingFlags)DSBLIT_NOFX));
			renderedSurface->setCaps(DWCAPS_NONE);
		}
	}
	

	
	void DFBImageProvider::renderIntoSurface() {

		IDirectFBImageProvider* provider;
		
		IDirectFB* dfb = (IDirectFB*)(LocalDeviceManager::getInstance()->getGfxRoot());
		DFBCHECK(dfb->CreateImageProvider(dfb, _mrl.c_str(), &provider));
		
		provider->GetSurfaceDescription(provider, &_surfaceDsc);
		provider->GetImageDescription(provider, &_imageDsc);
		
		 _surface = (IDirectFBSurface*)(LocalDeviceManager::getInstance()->createSurface(&_surfaceDsc));

		DFBCHECK(provider->RenderTo(provider, _surface, NULL));
		provider->Release(provider);
	}


	ISurface* DFBImageProvider::renderImage() {

		ISurface* renderedSurface = NULL;
		if (_mrl != ""){
			renderIntoSurface();
			renderedSurface = new DFBSurface(_surface);
			applyCaps(renderedSurface);
		}
		return renderedSurface;
	}
	

	ISurface* DFBImageProvider::renderImage(string mrl) {

		_mrl = mrl;

		renderIntoSurface();
		ISurface* renderedSurface = new DFBSurface(_surface);
		applyCaps(renderedSurface);

		return renderedSurface;
	}

	void DFBImageProvider::renderImage(string mrl, ISurface* surface) {

		_mrl = mrl;

		renderIntoSurface();
		surface->setContent(_surface);
		applyCaps(surface);		
	}
}
}
}
}
}
}
}

extern "C" ::br::pucrio::telemidia::ginga::core::system::io::IImageProvider*
		createImageProvider(const char* mrl) {

	return (new ::br::pucrio::telemidia::ginga::core::system::io::
			DFBImageProvider());
}

extern "C" void destroyImageProvider(
		::br::pucrio::telemidia::ginga::core::system::io::IImageProvider* ip) {

	delete ip;
}
