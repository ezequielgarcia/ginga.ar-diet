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
# if defined(_WIN32) || defined(__CYGWIN__)

#include <windows.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "os_dep.h"

static		int		w32_input_pid ;
#if defined(HAVE_PTHREADS)
static    int   b1EndThread = FALSE;
#endif /* defined(HAVE_PTHREADS) */

static char* keymap[] = {
	"\E[5~", /* VK_PRIOR */
	"\E[6~", /* VK_NEXT */
	"\E[F", /* VK_END */
	"\E[H", /* VK_HOME */
	"\E[D", /* VK_LEFT */
	"\E[A", /* VK_UP */
	"\E[C", /* VK_RIGHT */
	"\E[B", /* VK_DOWN */
	"", /* VK_SELECT */
	"", /* VK_PRINT */
	"", /* VK_EXECUTE */
	"", /* VK_SNAPSHOT */
	"\E[2~", /* VK_INSERT */
	"\E[3~" /* VK_DELETE */
} ;
void input_function (int fd)
{
	BOOL bSuccess;
	HANDLE hStdIn, hStdOut;
	DWORD dwMode;
	INPUT_RECORD inputBuffer;
	DWORD dwInputEvents;
	COORD coordScreen;
	DWORD cCharsRead;
	CONSOLE_CURSOR_INFO cci;
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	/* let's put up a meaningful console title */
	bSuccess = SetConsoleTitle("Links - Console mode browser");

	/* get the standard handles */
	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	hStdIn = GetStdHandle(STD_INPUT_HANDLE);

	/* set up mouse and window input */
	bSuccess = GetConsoleMode(hStdIn, &dwMode);

	bSuccess = SetConsoleMode(hStdIn, (dwMode & ~(ENABLE_LINE_INPUT |
			ENABLE_ECHO_INPUT)) | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

	cci.dwSize = 100;
	cci.bVisible = TRUE ;
	bSuccess = SetConsoleCursorInfo(hStdOut, &cci);
	/* This is the main input loop. Read from the input queue and process */
	/* the events read */
	do
	{
		/* read an input events from the input event queue */
		bSuccess = ReadConsoleInput(hStdIn, &inputBuffer, 1, &dwInputEvents);
		switch (inputBuffer.EventType)
		{
		case KEY_EVENT:
			if (inputBuffer.Event.KeyEvent.bKeyDown)
			{
				char c = inputBuffer.Event.KeyEvent.uChar.AsciiChar ;
				if (!c) {
					int vkey = inputBuffer.Event.KeyEvent.wVirtualKeyCode ;
					if (vkey >= VK_PRIOR && vkey <= VK_DELETE)
					{
						char	*p = keymap[vkey - VK_PRIOR] ;
						if (*p)
							if (write (fd, p, strlen(p)) < 0)
									bSuccess = FALSE ;
					}
					break ;
				}
				if (write (fd, &c, 1) < 0)
					bSuccess = FALSE ;
			}
			break;
		case MOUSE_EVENT:
			if (inputBuffer.Event.MouseEvent.dwEventFlags == 0 &&
				inputBuffer.Event.MouseEvent.dwButtonState)
			{
				char	mstr[] = "\E[Mxxx" ;
				mstr[3] = ' ' | 0 ;
				mstr[4] = ' ' + 1 +
					inputBuffer.Event.MouseEvent.dwMousePosition.X ;
				mstr[5] = ' ' + 1 +
					inputBuffer.Event.MouseEvent.dwMousePosition.Y ;
				if (write (fd, mstr, 6) < 0)
					bSuccess = FALSE ;
				mstr[3] = ' ' | 3 ;
				if (write (fd, mstr, 6) < 0)
					bSuccess = FALSE ;
			}
			break;
		case WINDOW_BUFFER_SIZE_EVENT:
			write (fd, "\E[R", 3) ;
			break;
		} /* switch */
		/* when we receive an esc down key, drop out of do loop */
#if defined(HAVE_PTHREADS)
	} while (bSuccess && !b1EndThread) ;
#else
	} while (bSuccess) ;
	exit (0) ;
#endif /* defined(HAVE_PTHREADS) */
}

#if defined(HAVE_PTHREADS)
void input_function_p (int *pfd)
{
	input_function(*pfd);
}
#endif /* defined(HAVE_PTHREADS) */

/*
void handle_terminal_resize(int fd, void (*fn)())
{
		return ;
}

void unhandle_terminal_resize(int fd)
{
		return ;
}

int get_terminal_size(int fd, int *x, int *y)
{
	CONSOLE_SCREEN_BUFFER_INFO	s ;

	if (GetConsoleScreenBufferInfo (GetStdHandle (STD_OUTPUT_HANDLE), &s))
	{
		*x = s.dwSize.X - 1;
		*y = s.dwSize.Y - 1;
		return 0 ;
	}
	*x = 80;
	*y = 25;
	return 0 ;
}
*/

void terminate_osdep ()
{
#if defined(HAVE_PTHREADS)
	b1EndThread = TRUE;
#else
	kill (w32_input_pid, SIGINT) ;
#endif /* defined(HAVE_PTHREADS) */
}

void set_proc_id (int id)
{
	w32_input_pid = id ;
}

# endif

