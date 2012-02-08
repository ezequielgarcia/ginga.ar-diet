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
/* memory.c
 * (c) 2002 Mikulas Patocka
 * This file is a part of the Links program, released under GPL.
 */

#include "links.h"

struct cache_upcall {
	struct cache_upcall *next;
	struct cache_upcall *prev;
	int (*upcall)(int);
	unsigned char name[1];
};

struct list_head cache_upcalls = { &cache_upcalls, &cache_upcalls }; /* cache_upcall */

int shrink_memory(int type)
{
	struct cache_upcall *c;
	int a = 0;
	foreach(c, cache_upcalls) a |= c->upcall(type);
	return a;
}

void register_cache_upcall(int (*upcall)(int), unsigned char *name)
{
	struct cache_upcall *c;
	c = mem_alloc(sizeof(struct cache_upcall) + strlen(name) + 1);
	c->upcall = upcall;
	strcpy(c->name, name);
	add_to_list(cache_upcalls, c);
}

void free_all_caches(void)
{
	struct cache_upcall *c;
	int a, b;
	do {
		a = 0;
		b = ~0;
		foreach(c, cache_upcalls) {
			int x = c->upcall(SH_FREE_ALL);
			a |= x;
			b &= x;
		}
	} while (a & ST_SOMETHING_FREED);
	if (!(b & ST_CACHE_EMPTY)) {
		unsigned char *m = init_str();
		int l = 0;
		foreach(c, cache_upcalls) if (!(c->upcall(SH_FREE_ALL) & ST_CACHE_EMPTY)) {
			if (l) add_to_str(&m, &l, ", ");
			add_to_str(&m, &l, c->name);
		}
		internal("could not release entries from caches: %s", m);
		mem_free(m);
	}
	free_list(cache_upcalls);
}
