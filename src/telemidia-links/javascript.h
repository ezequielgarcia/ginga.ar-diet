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
#ifndef YYSTYPE
#define YYSTYPE int
#endif
#define	BREAK	258
#define	CASE	259
#define	CATCH	260
#define	CONTINUE	261
#define	DEFAULT	262
#define	DELETE	263
#define	DO	264
#define	ELSE	265
#define	FINALLY	266
#define	FOR	267
#define	FUNCTION	268
#define	IF	269
#define	IN	270
#define	INSTANCEOF	271
#define	NEW	272
#define	RETURN	273
#define	SWITCH	274
#define	THIS	275
#define	THROW	276
#define	TYPEOF	277
#define	TRY	278
#define	VAR	279
#define	VOID	280
#define	WHILE	281
#define	WITH	282
#define	LEXERROR	283
#define	THREERIGHTEQUAL	284
#define	IDENTIFIER	285
#define	NULLLIT	286
#define	FALSELIT	287
#define	TRUELIT	288
#define	NUMLIT	289
#define	STRINGLIT	290
#define	REGEXPLIT	291
#define	BUGGY_TOKEN	292
#define	PLUSPLUS	11051
#define	MINMIN	11565
#define	SHLEQ	15676
#define	SHREQ	15678
#define	SHLSHL	15420
#define	SHRSHR	15934
#define	SHRSHRSHR	293
#define	EQEQ	15677
#define	EXCLAMEQ	15649
#define	EQEQEQ	294
#define	EXCLAMEQEQ	295
#define	ANDAND	9766
#define	OROR	31868
#define	PLUSEQ	15659
#define	MINEQ	15661
#define	TIMESEQ	15658
#define	MODEQ	15653
#define	DIVEQ	15663
#define	ANDEQ	15654
#define	OREQ	15740
#define	XOREQ	15710
#define	SHLSHLEQ	296
#define	SHRSHREQ	297


extern YYSTYPE yylval;
