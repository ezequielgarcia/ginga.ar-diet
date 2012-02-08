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
/* pomocny.c
 * (c) 2002 Martin 'PerM' Pergel
 * This file is a part of the Links program, released under GPL.
 */

#include "cfg.h"

#ifdef JS

#include "struct.h"

extern vrchol* previous;
extern vrchol* js_last;
extern long c_radku;

char* js_temp_var_for_stracpy1;

static void smaz(vrchol*v)
{	v->arg[1]=v->arg[2]=v->arg[3]=v->arg[4]=v->arg[5]=v->arg[0]=0;
	v->in=0;
	v->lineno=c_radku;
}

vrchol * terminal(void)
{	return neterminal();
/*vrchol * a=js_mem_alloc(sizeof(vrchol));
	memset(a, 0, sizeof(vrchol));
//	a->term=TERM;
	smaz(a);
	js_last=a;
	return a;*/
}
vrchol * neterminal(void)
{	vrchol * a=js_mem_alloc(sizeof(vrchol));
	memset(a, 0, sizeof(vrchol));
/*	a->term=NETERM;*/
	a->prev=previous;
	previous=a;
	smaz(a);
	js_last=a;
	return a;
}

void js_spec_vykill_timer(js_context*context,int i)
{
	if(context->upcall_timer==-1)
		internal("Upcallovy timer uz byl vykillen!\n");
	if(i)
	{	kill_timer(context->upcall_timer);
		switch(context->upcall_typek)
		{	case TYPEK_2_STRINGY:
				if(((struct fax_me_tender_2_stringy*)context->upcall_data)->string1)js_mem_free(((struct fax_me_tender_2_stringy*)context->upcall_data)->string1);
				if(((struct fax_me_tender_2_stringy*)context->upcall_data)->string2)js_mem_free(((struct fax_me_tender_2_stringy*)context->upcall_data)->string2);
				js_mem_free(context->upcall_data);
			break;
			case TYPEK_INT_STRING:
				if(((struct fax_me_tender_int_string*)context->upcall_data)->string)js_mem_free(((struct fax_me_tender_int_string*)context->upcall_data)->string);
				js_mem_free(context->upcall_data);
			break;
			case TYPEK_STRING_2_LONGY:
				if(((struct fax_me_tender_string_2_longy*)context->upcall_data)->string)js_mem_free(((struct fax_me_tender_string_2_longy*)context->upcall_data)->string);
				js_mem_free(context->upcall_data);
			break;
			case TYPEK_STRING:
				if(((struct fax_me_tender_string*)context->upcall_data)->string)js_mem_free(((struct fax_me_tender_string*)context->upcall_data)->string);
				js_mem_free(context->upcall_data);
			break;
			case TYPEK_NIC:
				js_mem_free(context->upcall_data);
			break;
			default:	internal("Neexistujici typ dat!\n");
			break;
		}
	}
	context->upcall_timer=-1;
}

void* js_js_temp_var1;
size_t js_js_temp_var2;
long js_zaflaknuto_pameti=0;

void my_itoa(char*vysl,long cislo)
{	char x[4*sizeof(long)+1];
	int j,i=1;
	x[j=0]='0'+cislo%10;
	while(cislo/=10)
		x[i++]='0'+cislo%10;
	while(i--)
		vysl[j++]=x[i];
	vysl[j]='\0';
}

#endif
