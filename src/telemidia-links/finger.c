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
/* finger.c
 * finger:// processing
 * (c) 2002 Mikulas Patocka
 * This file is a part of the Links program, released under GPL.
 */

#include "links.h"

void finger_send_request(struct connection *);
void finger_sent_request(struct connection *);
void finger_get_response(struct connection *, struct read_buffer *);
void finger_end_request(struct connection *);

void finger_func(struct connection *c)
{
	int p;
	if ((p = get_port(c->url)) == -1) {
		setcstate(c, S_INTERNAL);
		abort_connection(c);
		return;
	}
	c->from = 0;
	make_connection(c, p, &c->sock1, finger_send_request);
}

void finger_send_request(struct connection *c)
{
	unsigned char *req = init_str();
	int rl = 0;
	unsigned char *user;
	add_to_str(&req, &rl, "/W");
	if ((user = get_user_name(c->url))) {
		add_to_str(&req, &rl, " ");
		add_to_str(&req, &rl, user);
		mem_free(user);
	}
	add_to_str(&req, &rl, "\r\n");
	write_to_socket(c, c->sock1, req, rl, finger_sent_request);
	mem_free(req);
	setcstate(c, S_SENT);
}

void finger_sent_request(struct connection *c)
{
	struct read_buffer *rb;
	set_timeout(c);
	if (!(rb = alloc_read_buffer(c))) return;
	rb->close = 1;
	read_from_socket(c, c->sock1, rb, finger_get_response);
}

void finger_get_response(struct connection *c, struct read_buffer *rb)
{
	struct cache_entry *e;
	int l;
	set_timeout(c);
	if (get_cache_entry(c->url, &e)) {
		setcstate(c, S_OUT_OF_MEM);
		abort_connection(c);
		return;
	}
	c->cache = e;
	if (rb->close == 2) {
		setcstate(c, S_OK);
		finger_end_request(c);
		return;
	}
	l = rb->len;
	if (c->from + l < 0) {
		setcstate(c, S_LARGE_FILE);
		abort_connection(c);
		return;
	}
	c->received += l;
	if (add_fragment(c->cache, c->from, rb->data, l) == 1) c->tries = 0;
	c->from += l;
	kill_buffer_data(rb, l);
	read_from_socket(c, c->sock1, rb, finger_get_response);
	setcstate(c, S_TRANS);
}

void finger_end_request(struct connection *c)
{
	if (c->state == S_OK) {
		if (c->cache) {
			truncate_entry(c->cache, c->from, 1);
			c->cache->incomplete = 0;
		}
	}
	abort_connection(c);
}
