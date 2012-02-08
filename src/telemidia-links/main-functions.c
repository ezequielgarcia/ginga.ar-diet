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

int retval = RET_OK;

void unhandle_basic_signals(struct terminal *);
void sig_terminate(struct terminal *);
void sig_intr(struct terminal *);
void sig_ctrl_c(struct terminal *);
void handle_basic_signals(struct terminal *);
void end_dump(struct object_request *, void *);
void init(void);
void terminate_all_subsystems(void);


void sig_terminate(struct terminal *t)
{
	unhandle_basic_signals(t);
	terminate_loop = 1;
	retval = RET_SIGNAL;
}

void sig_intr(struct terminal *t)
{
	if (!t) {
		unhandle_basic_signals(t);
		terminate_loop = 1;
	} else {
		unhandle_basic_signals(t);
		exit_prog(t, NULL, NULL);
	}
}

void sig_ctrl_c(struct terminal *t)
{
	if (!is_blocked()) kbd_ctrl_c();
}

void sig_ign(void *x)
{
}

void sig_tstp(struct terminal *t)
{
#ifdef SIGSTOP
#if defined (SIGCONT) && defined(SIGTTOU) && defined(HAVE_GETPID)
	pid_t pid = getpid();
	pid_t newpid;
#endif
	if (!F) {
		block_itrm(1);
	}
#ifdef G
	else drv->block(NULL);
#endif
#if defined (SIGCONT) && defined(SIGTTOU)
	if (!(newpid = fork())) {
		while (1) {
			sleep(1);
			kill(pid, SIGCONT);
		}
	}
#endif
	raise(SIGSTOP);
#if defined (SIGCONT) && defined(SIGTTOU)
	if (newpid != -1) kill(newpid, SIGKILL);
#endif
#endif
}

void sig_cont(struct terminal *t)
{
	if (!F) {
		unblock_itrm(1);
#ifdef G
	} else {
		drv->unblock(NULL);
#endif
	}
	/*else register_bottom_half(raise, SIGSTOP);*/
}

void handle_basic_signals(struct terminal *term)
{
	install_signal_handler(SIGHUP, (void (*)(void *))sig_intr, term, 0);
	if (!F) install_signal_handler(SIGINT, (void (*)(void *))sig_ctrl_c, term, 0);
	/*install_signal_handler(SIGTERM, (void (*)(void *))sig_terminate, term, 0);*/
#ifdef SIGTSTP
	if (!F) install_signal_handler(SIGTSTP, (void (*)(void *))sig_tstp, term, 0);
#endif
#ifdef SIGTTIN
	if (!F) install_signal_handler(SIGTTIN, (void (*)(void *))sig_tstp, term, 0);
#endif
#ifdef SIGTTOU
	install_signal_handler(SIGTTOU, (void (*)(void *))sig_ign, term, 0);
#endif
#ifdef SIGCONT
	if (!F) install_signal_handler(SIGCONT, (void (*)(void *))sig_cont, term, 0);
#endif
}

void unhandle_terminal_signals(struct terminal *term)
{
	install_signal_handler(SIGHUP, NULL, NULL, 0);
	if (!F) install_signal_handler(SIGINT, NULL, NULL, 0);
#ifdef SIGTSTP
	install_signal_handler(SIGTSTP, NULL, NULL, 0);
#endif
#ifdef SIGTTIN
	install_signal_handler(SIGTTIN, NULL, NULL, 0);
#endif
#ifdef SIGTTOU
	install_signal_handler(SIGTTOU, NULL, NULL, 0);
#endif
#ifdef SIGCONT
	install_signal_handler(SIGCONT, NULL, NULL, 0);
#endif
}

void unhandle_basic_signals(struct terminal *term)
{
	install_signal_handler(SIGHUP, NULL, NULL, 0);
	if (!F) install_signal_handler(SIGINT, NULL, NULL, 0);
	/*install_signal_handler(SIGTERM, NULL, NULL, 0);*/
#ifdef SIGTSTP
	install_signal_handler(SIGTSTP, NULL, NULL, 0);
#endif
#ifdef SIGTTIN
	install_signal_handler(SIGTTIN, NULL, NULL, 0);
#endif
#ifdef SIGTTOU
	install_signal_handler(SIGTTOU, NULL, NULL, 0);
#endif
#ifdef SIGCONT
	install_signal_handler(SIGCONT, NULL, NULL, 0);
#endif
}

int terminal_pipe[2];

int attach_terminal(int in, int out, int ctl, void *info, int len)
{
	struct terminal *term;
	fcntl(terminal_pipe[0], F_SETFL, O_NONBLOCK);
	fcntl(terminal_pipe[1], F_SETFL, O_NONBLOCK);
	handle_trm(in, out, out, terminal_pipe[1], ctl, info, len);
	mem_free(info);
	if ((term = init_term(terminal_pipe[0], out, win_func))) {
		handle_basic_signals(term);	/* OK, this is race condition, but it must be so; GPM installs it's own buggy TSTP handler */
		return terminal_pipe[1];
	}
	close(terminal_pipe[0]);
	close(terminal_pipe[1]);
	return -1;
}

#ifdef G

int attach_g_terminal(void *info, int len)
{
	struct terminal *term;
	term = init_gfx_term(win_func, info, len);
	mem_free(info);
	return term ? 0 : -1;
}

#endif

struct object_request *dump_obj;
off_t dump_pos;

void end_dump(struct object_request *r, void *p)
{
	struct cache_entry *ce;
	int oh;
	if (!r->state || (r->state == 1 && dmp != D_SOURCE)) return;
	if ((oh = get_output_handle()) == -1) return;
	ce = r->ce;
	if (dmp == D_SOURCE) {
		if (ce) {
			struct fragment *frag;
			nextfrag:
			foreach(frag, ce->frag) if (frag->offset <= dump_pos && frag->offset + frag->length > dump_pos) {
				int l = frag->length - (dump_pos - frag->offset);
				int w = hard_write(oh, frag->data + dump_pos - frag->offset, l);
				if (w != l) {
					detach_object_connection(r, dump_pos);
					if (w < 0) fprintf(stderr, "Error writing to stdout: %s.\n", strerror(errno));
					else fprintf(stderr, "Can't write to stdout.\n");
					retval = RET_ERROR;
					goto terminate;
				}
				dump_pos += w;
				detach_object_connection(r, dump_pos);
				goto nextfrag;
			}
		}
		if (r->state >= 0) return;
	} else if (ce) {
		struct document_options o;
		struct f_data_c *fd;
		if (!(fd = create_f_data_c(NULL, NULL))) goto terminate;
		memset(&o, 0, sizeof(struct document_options));
		o.xp = 0;
		o.yp = 1;
		o.xw = screen_width;
		o.yw = 25;
		o.col = 0;
		o.cp = dump_codepage == -1 ? 0 : dump_codepage;
		ds2do(&dds, &o);
		o.plain = 0;
		o.frames = 0;
		o.js_enable = 0;
		memcpy(&o.default_fg, &default_fg, sizeof(struct rgb));
		memcpy(&o.default_bg, &default_bg, sizeof(struct rgb));
		memcpy(&o.default_link, &default_link, sizeof(struct rgb));
		memcpy(&o.default_vlink, &default_vlink, sizeof(struct rgb));
		o.framename = "";
		if (!(fd->f_data = cached_format_html(fd, r, r->url, &o, NULL))) goto term_1;
		dump_to_file(fd->f_data, oh);
		term_1:
		reinit_f_data_c(fd);
		mem_free(fd);
	}
	if (r->state != O_OK) {
		unsigned char *m = get_err_msg(r->stat.state);
		fprintf(stderr, "%s\n", get_english_translation(m));
		retval = RET_ERROR;
		goto terminate;
	}
	terminate:
	terminate_loop = 1;
}

int g_argc;
unsigned char **g_argv;

unsigned char *path_to_exe;

int init_b = 0;

void initialize_all_subsystems(void);
void initialize_all_subsystems_2(void);

void init(void)
{
	int uh;
	void *info;
	int len;
	unsigned char *u;
#ifdef TEST
	{
		int test();
		test();
		return;
	}
#endif
	initialize_all_subsystems();

	utf8_table=get_cp_index("UTF-8");

/* OS/2 has some stupid bug and the pipe must be created before socket :-/ */
	if (c_pipe(terminal_pipe)) {
		error("ERROR: can't create pipe for internal communication");
		retval = RET_FATAL;
		goto ttt;
	}
	if (!(u = parse_options(g_argc - 1, g_argv + 1))) {
		retval = RET_SYNTAX;
		goto ttt;
	}
	if (ggr_drv[0] || ggr_mode[0]) ggr = 1;
	if (dmp) ggr = 0;
	if (!ggr && !no_connect && (uh = bind_to_af_unix()) != -1) {
		close(terminal_pipe[0]);
		close(terminal_pipe[1]);
		if (!(info = create_session_info(base_session, u, default_target, &len))) {
			close(uh);
			retval = RET_FATAL;
			goto ttt;
		}
		initialize_all_subsystems_2();
		handle_trm(get_input_handle(), get_output_handle(), uh, uh, get_ctl_handle(), info, len);
		handle_basic_signals(NULL);	/* OK, this is race condition, but it must be so; GPM installs it's own buggy TSTP handler */
		mem_free(info);
		return;
	}
	if ((dds.assume_cp = get_cp_index("ISO-8859-1")) == -1) dds.assume_cp = 0;
	load_config();
	init_b = 0;
	//init_bookmarks();
	create_initial_extensions();
	load_url_history();
	init_cookies();
	u = parse_options(g_argc - 1, g_argv + 1);
	if (!u) {
		ttt:
		initialize_all_subsystems_2();
		tttt:
		terminate_loop = 1;
		return;
	}
	if (!dmp) {
		if (ggr) {
#ifdef G
			unsigned char *r;
			if ((r = init_graphics(ggr_drv, ggr_mode, ggr_display))) {
				fprintf(stderr, "%s", r);
				mem_free(r);
				retval = RET_SYNTAX;
				goto ttt;
			}
			handle_basic_signals(NULL);
			init_dither(drv->depth);
			F = 1;
#else
			fprintf(stderr, "Graphics not enabled when compiling\n");
			retval = RET_SYNTAX;
			goto ttt;
#endif
		}
		initialize_all_subsystems_2();
		if (!((info = create_session_info(base_session, u, default_target, &len)) && gf_val(attach_terminal(get_input_handle(), get_output_handle(), get_ctl_handle(), info, len), attach_g_terminal(info, len)) != -1)) {
			retval = RET_FATAL;
			terminate_loop = 1;
		}
	} else {
		unsigned char *uu, *wd;
		initialize_all_subsystems_2();
		close(terminal_pipe[0]);
		close(terminal_pipe[1]);
		if (!*u) {
			fprintf(stderr, "URL expected after %s\n.", dmp == D_DUMP ? "-dump" : "-source");
			retval = RET_SYNTAX;
			goto tttt;
		}
		if (!(uu = translate_url(u, wd = get_cwd()))) uu = stracpy(u);
		request_object(NULL, uu, NULL, PRI_MAIN, NC_RELOAD, end_dump, NULL, &dump_obj);
		mem_free(uu);
		if (wd) mem_free(wd);
	}
}

/* Is called before gaphics driver init */
void initialize_all_subsystems(void)
{
	init_trans();
	set_sigcld();
	init_home();
	init_dns();
	init_cache();
	iinit_bfu();
	memset(&dd_opt, 0, sizeof dd_opt);
}

/* Is called sometimes after and sometimes before graphics driver init */
void initialize_all_subsystems_2(void)
{
	GF(init_dip());
	init_bfu();
	GF(init_imgcache());
	init_fcache();
	GF(init_grview());
}

/* changing: begin */
extern void setCoord(double cx, double cy, double cw, double ch);
extern struct session* tmpSession;
extern struct session* fatherSession;

int initialized = 0;
int initializing = 0;
/* changing: end */

void terminate_all_subsystems(void)
{
	/* changing: begin */
	if (initializing == 1) {
		usleep(350000);
	}
	initialized = 0;
	initializing = 0;
	init_b = 0;
	/* changing: end */

	if (!F) af_unix_close();
	check_bottom_halves();
	abort_all_downloads();
#ifdef HAVE_SSL
	ssl_finish();
#endif
	check_bottom_halves();
	destroy_all_terminals();
	check_bottom_halves();
	shutdown_bfu();
	GF(shutdown_dip());
	if (!F) free_all_itrms();
	release_object(&dump_obj);
	abort_all_connections();

	free_all_caches();
	if (init_b) save_url_history();
	free_history_lists();
	free_term_specs();
	free_types();
	free_blocks();
	if (init_b) finalize_bookmarks();
	free_conv_table();
	free_blacklist();
	if (init_b) cleanup_cookies();
	cleanup_auth();
	check_bottom_halves();
	end_config();
	free_strerror_buf();
	shutdown_trans();
	GF(shutdown_graphics());
	terminate_osdep();
	if (clipboard) mem_free(clipboard);
}


/* changing: begin */

/*int createBrowserWindowByArgs(int argc, char *argv[])
{
	path_to_exe = argv[0];
	g_argc = argc;
	g_argv = (unsigned char **)argv;

	select_loop(init);
	pthread_join(threadId_, 0);
	terminate_all_subsystems();

	check_memory_leaks();
	return retval;
}*/

static void* createBrowserWindowByArgs(void* args)
{
	char* argv[4];
	argv[0] = "./links";
	argv[1] = "-g";
	argv[2] = "-anonymous";
	argv[3] = "-anonymousGinga";

	path_to_exe = argv[0];
	g_argc = 4;
	g_argv = (unsigned char **)argv;

	select_loop(init);
	/*terminate_all_subsystems();*/
	destroyAllBrowsers();
	check_memory_leaks();
	return NULL;
}

extern struct session* getUsedSession();

static int numOfSessions = 0;

struct session* openBrowser(double cx, double cy, double cw, double ch) {
	numOfSessions++;
	if (initialized == 0 && initializing == 0) {
		initialized = 1;
		initializing = 1;
		setCoord(cx, cy, cw, ch);

		pthread_t threadId_;
		//printf("\n \n openBrowser  \n \n");
		pthread_create(&threadId_, 0, createBrowserWindowByArgs, NULL);
		pthread_detach(threadId_);
		while (fatherSession == NULL) {
			usleep(50000);
		}
		initializing = 0;
		//browserHide(fatherSession);
		return fatherSession;

	} else {
		while (initializing == 1) {
			usleep(50000);
		}
		//browserHide(fatherSession);
		struct session* someUsedSession = NULL;
		someUsedSession = getUsedSession();
		if (someUsedSession == NULL) {
			//printf("\n \n someUsedSession = NULL  \n \n");
			setCoord(cx, cy, cw, ch);
			open_in_new_window(tmpSession->term, send_open_new_xterm, tmpSession);

		} else {
			//printf("\n \n someUsedSession != NULL  \n \n");
			browserResizeCoord(someUsedSession, cx, cy, cw, ch);
			tmpSession = someUsedSession;
			someUsedSession = NULL;
		}
	//	browserHide(tmpSession);
		return tmpSession;
	}
}

void loadUrlOn(struct session* ses, const char* someUrl) {
	while (initializing == 1) {
		usleep(50000);
	}
	if (ses == NULL) {
		/*printf("\n \n loadUrlOn ses == NULL  \n \n");*/
		goto_url_not_from_dialog(fatherSession, (char*)someUrl);

	} else {
		/*printf("\n \n loadUrlOn ses != NULL  \n \n");*/
		goto_url_not_from_dialog(ses, (char*)someUrl);
	}
}

extern void closeTerminal(struct terminal* term);
extern void clear_terminal(struct terminal* term);

static int hasFather = 1;

int closeBrowser(struct session* ses) {
	numOfSessions--;
	if (initializing == 1) {
		usleep(350000);
	}
	initializing = 0;
	if (ses == NULL || ses == fatherSession) {
		//printf("\n \n really father  \n \n");
		if (hasFather) {
			//browserHide(fatherSession);
			hasFather = 0;
			really_exit_prog(fatherSession);
			closeTerminal(fatherSession->term);
		}

	} else {
		//printf("\n \n really child  \n \n");
		if (hasFather) {
			//browserHide(fatherSession);
		}
		//browserHide(ses);
		really_exit_prog(ses);
		closeTerminal(ses->term);
		ses = NULL;
	}
	//printf("\n \n closeBrowser return 1 \n \n");
	return 1;
}

int destroyAllBrowsers() {
	usleep(400000);

	if (!F) af_unix_close();
	check_bottom_halves();
	abort_all_downloads();
#ifdef HAVE_SSL
	ssl_finish();
#endif
	check_bottom_halves();
	destroy_all_terminals();
	check_bottom_halves();
	shutdown_bfu();
	GF(shutdown_dip());
	if (!F) free_all_itrms();
	release_object(&dump_obj);
	abort_all_connections();
	free_all_caches();
	if (init_b) save_url_history();
	free_history_lists();
	free_term_specs();
	free_types();
	free_blocks();
	if (init_b) finalize_bookmarks();
	free_conv_table();
	free_blacklist();
	if (init_b) cleanup_cookies();
	cleanup_auth();
	check_bottom_halves();
	end_config();
	free_strerror_buf();
	shutdown_trans();
	GF(shutdown_graphics());
	terminate_osdep();


	initialized = 0;
	initializing = 0;
	init_b = 0;

	return 1;
}

#ifdef TEST

unsigned char *txt;
int txtl;

struct style *gst;

#define G_S	16
struct bitmap xb;
struct bitmap *xbb[16] = {&xb, &xb, &xb, &xb, &xb, &xb, &xb, &xb, &xb, &xb, &xb, &xb, &xb, &xb, &xb, &xb, };

void rdr(struct graphics_device *dev, struct rect *r)
{
	struct style *st;
	int i;
	drv->set_clip_area(dev, r);
	/*for (i = 0; i <= dev->x / 2 && i <= dev->y / 2; i += 20) {
		int j;
		for (j = 0; j < 10000000; j++) ;
		drv->fill_area(dev, i, i, dev->x - i, dev->y - i, (i * 65536 + i * 1024 + i * 16) & 0xffffff);
	}
	{
		struct bitmap bmp;
		bmp.x = 100;
		bmp.y = 200;
		drv->get_empty_bitmap(&bmp);
		if (bmp.skip > 0) memset(bmp.data, 192, bmp.y * bmp.skip);
		else memset((char *)bmp.data + (bmp.y - 1) * bmp.skip, 192, -bmp.y * bmp.skip);
		drv->register_bitmap(&bmp);
		drv->draw_bitmap(dev, &bmp, 10, 10);
		drv->unregister_bitmap(&bmp);
	}*/
	{
		int x, y;
		for (y = 0; y < dev->size.y2; y+=G_S)
			for (x = 0; x < dev->size.x2; ) {
				g_print_text(drv, dev, x, y, gst, "Toto je pokus. ", &x);
				//drv->draw_bitmap(dev, &xb, x, y); x += xb.x;
				//drv->draw_bitmaps(dev, xbb, 16, x, y); x += xb.x << 4;
			}
	}
	st = g_get_style(0x000000, 0xffffff, 20, "", 0);
	g_print_text(drv, dev, 0, dev->size.y2 / 2, st, txt, NULL);
	g_free_style(st);
}

void rsz(struct graphics_device *dev)
{
	rdr(dev, &dev->size);
}

int dv = 2;

void key(struct graphics_device *dev, int x, int y)
{
	if (x == 'q') {
		drv->shutdown_device(dev);
		if (--dv) return;
		mem_free(txt);
		drv->unregister_bitmap(&xb);
		terminate_loop = 1;
		g_free_style(gst);
		return;
	}
	add_to_str(&txt, &txtl, "(");
	add_num_to_str(&txt, &txtl, x);
	add_to_str(&txt, &txtl, ",");
	add_num_to_str(&txt, &txtl, y);
	add_to_str(&txt, &txtl, ") ");
	rdr(dev, &dev->size);
}

int test(void)
{
	struct graphics_device *dev, *dev2;
	unsigned char *msg;
	initialize_all_subsystems();
	F = 1;
	initialize_all_subsystems_2();
	printf("i\n");fflush(stdout);
	msg = init_graphics("", "", "");
	if (msg) {
		printf("-%s-\n", msg);fflush(stdout);
		mem_free(msg);
		terminate_loop = 1;
		return 0;
	}
	printf("ok\n");fflush(stdout);
	gst = g_get_style(0x000000, 0xffffff, G_S, "", 0);
	txt = init_str();
	txtl = 0;
	dev = drv->init_device(); dev->redraw_handler = rdr; dev->resize_handler = rsz; dev->keyboard_handler = key;
	dev2 = drv->init_device(); dev2->redraw_handler = rdr; dev2->resize_handler = rsz; dev2->keyboard_handler = key;
	{
		int x, y;
		xb.x = G_S / 2;
		xb.y = G_S;
		drv->get_empty_bitmap(&xb);
		for (y = 0; y < xb.y; y++)
			for (x = 0; x < xb.x; x++)
				((char *)xb.data)[y * xb.skip + x] = x + y;
		drv->register_bitmap(&xb);
	}
	return 0;
}

#endif
