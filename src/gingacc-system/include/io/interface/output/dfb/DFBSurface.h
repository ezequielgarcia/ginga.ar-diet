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

#ifndef DFBSURFACE_H_
#define DFBSURFACE_H_

#include "../../../../util/mutex/dfbsurfacemutex.h"

#include "../IWindow.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <directfb.h>
#ifdef __cplusplus
}
#endif

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace core {
namespace system {
namespace io {
	class DFBSurface : public ISurface {
		public:
			static const int DFB_BG_R = 0;
			static const int DFB_BG_G = 0;
			static const int DFB_BG_B = 0;
			static const int DFB_BG_A = 0;
			
		private:
			IDirectFBSurface* sur;
			IWindow* parent;
			IColor* chromaColor;
			int caps;
			::util::Singleton<DFBSurfaceMutex> _DFBSurfaceMutexSingleton;

		public:
			DFBSurface();
			DFBSurface(int w, int h);
			DFBSurface(void* someSurface);      //SurfaceGraphicsWg
			virtual ~DFBSurface();
			void addCaps(int caps);
			void setCaps(int caps);
			int getCap(string cap);
			int getCaps();
			bool setParent(void* parentWindow); //IWindow
			void* getParent();                  //IWindow
			void* getContent();
			void setContent(void* surface);
			void setChromaColor(IColor* color);
			IColor* getChromaColor();
			void clearContent();
			void clearSurface();
			ISurface* getSubSurface(int x, int y, int w, int h);
			void drawLine(int x1, int y1, int x2, int y2);
			void drawRectangle(int x, int y, int w, int h);
			void fillRectangle(int x, int y, int w, int h);
			void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3);
			void drawEllipse(bool fill, int x, int y, int w, int h, float start, float end);
			void pixel(int x, int y);
			void drawString(int x, int y, const char* txt);
			void setBorder(IColor* borderColor);
			void setBgColor(IColor* bgColor);
			void setColor(IColor* writeColor);
			void setFont(void* font);
			void getStringExtents(const char* text, int* w, int* h);
			void flip();
			void blit(
					int x,
					int y,
					ISurface* src=NULL,
					int srcX=-1, int srcY=-1, int srcW=-1, int srcH=-1);

			void setClip(int x, int y, int w, int h);
			void getSize(int* width, int* height);
			string getDumpFileUri();
		private:
			void plot4EllipsePoints(int x, int y, int cx, int cy, float start, float end);
			void fillEllipsePoins(int x, int y, int cx, int cy, float start, float end);
	};
}
}
}
}
}
}
}

#endif /*DFBSURFACE_H_*/
