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
/* language.c
 * (c) 2002 Mikulas Patocka
 * This file is a part of the Links program, released under GPL.
 */

#include "links.h"

struct translation {
	int code;
	unsigned char *name;
};

struct translation_desc {
	struct translation *t;
};

unsigned char dummyarray[T__N_TEXTS];

#include "language.inc"

unsigned char **translation_array[N_LANGUAGES][N_CODEPAGES];

int current_language;
int current_lang_charset;

void init_trans(void)
{
	int i, j;
	for (i = 0; i < N_LANGUAGES; i++)
		for (j = 0; j < N_CODEPAGES; j++)
			translation_array[i][j] = NULL;
	current_language = 0;
	current_lang_charset = 0;
}

void shutdown_trans(void)
{
	int i, j, k;
	for (i = 0; i < N_LANGUAGES; i++)
		for (j = 0; j < N_CODEPAGES; j++) if (translation_array[i][j]) {
			for (k = 0; k < T__N_TEXTS; k++) if (translation_array[i][j][k])
				mem_free(translation_array[i][j][k]);
			mem_free(translation_array[i][j]);
		}
}

unsigned char *get_text_translation(unsigned char *text, struct terminal *term)
{
	unsigned char **current_tra;
	struct conv_table *conv_table;
	unsigned char *trn;
	static int utf8_charset = -1;
	int charset;
	if (!term) charset = 0;
	else if (term->spec) charset = term->spec->charset;
	else {
		if (utf8_charset == -1) utf8_charset = get_cp_index("UTF-8");
		charset = utf8_charset;
	}
	if (text < dummyarray || text > dummyarray + T__N_TEXTS) return text;
	if ((current_tra = translation_array[current_language][charset])) {
		unsigned char *tt;
		if ((trn = current_tra[text - dummyarray])) return trn;
		tr:
		if (!(tt = translations[current_language].t[text - dummyarray].name)) {
			trn = stracpy(translation_english[text - dummyarray].name);
		} else {
			struct document_options l_opt;
			memset(&l_opt, 0, sizeof(l_opt));
			l_opt.plain = 0;
			l_opt.cp = charset;
			conv_table = get_translation_table(current_lang_charset, charset);
			trn = links_convert_string(conv_table, tt, strlen(tt), &l_opt);
		}
		current_tra[text - dummyarray] = trn;
	} else {
		if (current_lang_charset && charset != current_lang_charset) {
			current_tra = translation_array[current_language][charset] = mem_alloc(sizeof (unsigned char **) * T__N_TEXTS);
			memset(current_tra, 0, sizeof (unsigned char **) * T__N_TEXTS);
			goto tr;
		}
		if (!(trn = translations[current_language].t[text - dummyarray].name)) {
			trn = translations[current_language].t[text - dummyarray].name = translation_english[text - dummyarray].name;	/* modifying translation structure */
		}
	}
	return trn;
}

unsigned char *get_english_translation(unsigned char *text)
{
	if (text < dummyarray || text > dummyarray + T__N_TEXTS) return text;
	return translation_english[text - dummyarray].name;
}

int n_languages(void)
{
	return N_LANGUAGES;
}

unsigned char *language_name(int l)
{
	return translations[l].t[T__LANGUAGE].name;
}

int language_index(unsigned char *name)
{
	int a;
	for (a=0;a<N_LANGUAGES;a++)
		if (!strcmp(name, translations[a].t[T__LANGUAGE].name))return a;
	return 0;
}

void set_language(int l)
{
	int i;
	unsigned char *cp;
	for (i = 0; i < T__N_TEXTS; i++) if (translations[l].t[i].code != i) {
		internal("Bad table for language %s. Run script synclang.", translations[l].t[T__LANGUAGE].name);
		return;
	}
	current_language = l;
	cp = translations[l].t[T__CHAR_SET].name;
	i = get_cp_index(cp);
	if (i == -1) {
		internal("Unknown charset for language %s.", translations[l].t[T__LANGUAGE].name);
		i = 0;
	}
	current_lang_charset = i;
}
