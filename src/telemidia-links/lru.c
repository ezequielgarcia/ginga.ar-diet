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
/* lru.c
 * LRU cache
 * (c) 2002 Karel 'Clock' Kulhavy
 * This file is a part of the Links program, released under GPL.
 */

#include "cfg.h"

#ifdef G

#include "links.h"

void lru_insert(struct lru *cache, void *entry, struct lru_entry ** row,
	unsigned bytes_consumed)
{
	struct lru_entry *new_entry=mem_alloc(sizeof(*new_entry));

	new_entry->above=NULL;
	new_entry->below=cache->top;
	new_entry->next=*row;
	new_entry->previous=row;
	new_entry->data=entry;
	new_entry->bytes_consumed=bytes_consumed;
	if (new_entry->below){
		new_entry->below->above=new_entry;
	}else{
		cache->bottom=new_entry;
	}
	if (new_entry->next){
		new_entry->next->previous=&(new_entry->next);
	}
	*row=new_entry;
	cache->top=new_entry;
cache->bytes+=bytes_consumed;
	cache->items++;		
}

/* Returns bottom (or NULL if the cache is empty) but doesn't
 * unlink it.
 */
void * lru_get_bottom(struct lru *cache)
{
	if (!cache->bottom) return NULL;
	return cache->bottom->data;
}

/* Destroys bottom on nonempty cache. If the cache is empty, segmentation
 * fault results.
 */
void lru_destroy_bottom(struct lru* cache)
{
	struct lru_entry *it=cache->bottom;
	
	cache->bytes-=cache->bottom->bytes_consumed;
	cache->items--;
	cache->bottom=it->above;
	if (cache->bottom) cache->bottom->below=NULL; else cache->top=NULL;
	
	if (it->next){
		it->next->previous=it->previous;
	}
	*(it->previous)=it->next;
	mem_free(it);
}

/* Returns a value of "data"
 * template is what we search for.
 */
void *lru_lookup(struct lru *cache, void *template, struct lru_entry *ptr)
{
	while (ptr){
		if (!cache->compare_function(ptr->data,template)){
			/* Found */
			if (ptr->above){
				if (ptr->below){
					ptr->below->above=ptr->above;
				}else{
					cache->bottom=ptr->above;
				}
				ptr->above->below=ptr->below;
				ptr->above=NULL;
				ptr->below=cache->top;
				cache->top->above=ptr;
				cache->top=ptr;
			}
			return ptr->data;
		}
		ptr=ptr->next;
	}
	return NULL;
}

void lru_init (struct lru *cache, int
(*compare_function)(void *entry, void *template), int max_bytes)
{
	cache->compare_function=compare_function;
	cache->top=NULL;
	cache->bottom=NULL;
	cache->bytes=0;
	cache->items=0;
	cache->max_bytes=max_bytes;
}

#endif
