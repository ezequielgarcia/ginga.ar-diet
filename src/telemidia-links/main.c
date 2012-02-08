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
/* main.c
        ihowMessage("AAAEEEEEEE","Valeu BOLINHA!!!!!!", 0, 200, 720, 139,5000)
 * main()
 * (c) 2002 Mikulas Patocka
 * This file is a part of the Links program, released under GPL.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "directfb.h"

#ifdef __cplusplus
}
#endif

#include "links.h"
#include "telemidialinks.h"

int main(int argc, char *argv[]) {
	struct session* ses1 = NULL;
	struct session* ses2 = NULL;
	struct session* ses3 = NULL;
	IDirectFB* dfb;

	DirectFBInit( NULL, NULL );
	DirectFBCreate( &dfb );

	setBrowserDFB(dfb);
	setDisplayMenu(0);

	ses1 = openBrowser(10, 10, 100, 100);
	loadUrlOn(ses1, "http://www.google.com.br");
	double teste = -5000;
	while (1) {
		teste = teste + 0.5;
		if (teste > 400000000)
			break;
	}

	ses2 = openBrowser(10, 110, 100, 100);
	loadUrlOn(ses2, "http://www.google.com.br");
	teste = -5000;
	while (1) {
		teste = teste + 0.5;
		if (teste > 600000000)
			break;
	}
	closeBrowser(ses2);
	teste = -5000;
	while (1) {
		teste = teste + 0.5;
		if (teste > 200000000)
			break;
	}

	ses3 = openBrowser(10, 210, 100, 100);
	loadUrlOn(ses3, "http://www2.telemidia.puc-rio.br/inf/labredes");

	teste = -5000;
	while (1) {
		teste = teste + 0.5;
		if (teste > 300000000)
			break;
	}
	closeBrowser(ses1);
	teste = -5000;
	while (1) {
		teste = teste + 0.5;
		if (teste > 400000000)
			break;
	}

	setBrowserDFB(dfb);
	ses2 = openBrowser(10, 310, 200, 200);
	loadUrlOn(ses2, "file:///tmp/test.html");

	while (1) {

	}
	return 1;
}

/*
int main(int argc, char *argv[])
{
	path_to_exe = argv[0];
	g_argc = argc;
	g_argv = (unsigned char **)argv;

	select_loop(init);
	terminate_all_subsystems();

	check_memory_leaks();
	return retval;
}
*/
/* changing: end */
