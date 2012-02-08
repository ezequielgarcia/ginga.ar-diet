/*******************************************************************************

  Copyright 2010 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata

********************************************************************************

  This file is part of Ginga implementation.

    This program is free software: you can redistribute it and/or modify it 
  under the terms of the GNU General Public License as published by the Free 
  Software Foundation, either version 2 of the License.

    Ginga is distributed in the hope that it will be useful, but WITHOUT ANY 
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with
  this program. If not, see <http://www.gnu.org/licenses/>.

********************************************************************************

  Este archivo es parte de la implementación de Ginga.

    Este programa es Software Libre: Ud. puede redistribuirlo y/o modificarlo 
  bajo los términos de la Licencia Pública General GNU como es publicada por la
  Free Software Foundation, según la versión 2 de la licencia.

    Ginga se distribuye esperando que resulte de utilidad, pero SIN NINGUNA 
  GARANTÍA; ni siquiera la garantía implícita de COMERCIALIZACIÓN o ADECUACIÓN 
  PARA ALGÚN PROPÓSITO PARTICULAR. Para más detalles, revise la Licencia Pública 
  General GNU.

    Ud. debería haber recibido una copia de la Licencia Pública General GNU 
  junto a este programa. Si no, puede verla en <http://www.gnu.org/licenses/>.

*******************************************************************************/
#ifndef TELEMIDIALINKS_H_
#define TELEMIDIALINKS_H_

#ifdef __cplusplus
extern "C" {
#endif

void browserShow(struct session* ses);
void browserHide(struct session* ses);
void browserResizeCoord(
		struct session* resizeSession,
		double cx, double cy, double cw, double ch);

void browserSetFocusHandler(int isHandler, struct session* someSes);
void browserReceiveEvent(struct session* ses, void* event);
void* getDFBWindow(struct session* someSes);
void* browserGetSurface(struct session* someSes);
void browserSetFlipWindow(struct session* ses, void* flipWindow);
void setBrowserDFB(void* dfb);
void browserSetAlpha(int alpha, struct session* ses);
void browserRequestFocus(struct session* ses);
void setDisplayMenu(int toDisplay);
struct session* openBrowser(double cx, double cy, double cw, double ch);
void loadUrlOn(struct session* ses, const char* url);
void setGhostBrowser(struct session* ses);
int closeBrowser(struct session* ses);
int destroyAllBrowsers();

#ifdef __cplusplus
}
#endif

#endif /*TELEMIDIALINKS_H_*/

