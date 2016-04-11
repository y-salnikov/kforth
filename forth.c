#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "forth.h"


// word fields
// +-------+---------+------+---------+------+-----+-----+-----+
// | flags | counter | name | counter | align| LFA | CFA | PFA | 
// +-------+---------+------+---------+------+-----+-----+-----+
//    1b      1b        ?       1b            cell cell  ?cells
// CFA = NULL  - :
// CFA = ADR   - primitive
// PFA = [CFA1,CFA2,P2,CFA3...]
//         ^
//         PC


void push_sp(forth_context_type* fc, size_t val)
{
	fc->SP--;
	*fc->SP=val;
}

void push_rp(forth_context_type* fc, size_t val)
{
	fc->RP--;
	*fc->RP=val;
}

void sp_adr_read(forth_context_type* fc)
{
	push_sp(fc,(size_t)fc->SP);
}

void rp_adr_read(forth_context_type* fc)
{
	push_sp(fc,(size_t)fc->RP);
}

void lit(forth_context_type* fc)
{
	size_t *tmp;
	tmp=(size_t*)*fc->RP;
	push_sp(fc,*tmp);
	*fc->RP=(size_t)(tmp+1);
}

void endw(forth_context_type* fc)
{
	fc->RP++;
}

void add_byte(forth_context_type* fc, char b)
{
	char *here;
	here=(char*)*fc->here_ptr;
	*here=b;
	here++;
	*fc->here_ptr=(size_t)here;
}

void add_cell(forth_context_type* fc, size_t b)
{
	size_t *here;
	here=(size_t*)*fc->here_ptr;
	
	*here=b;
	here++;
	*fc->here_ptr=(size_t)here;
}

void align(forth_context_type* fc)
{
	char m;
	char *here;
	int i;
	here=(char*)*fc->here_ptr;
	m=((size_t)here) % fc->cell;
	if(m)
	{
		for(i=0;i<fc->cell-m;i++)
		{
			*here++=0;
		}
	}
	*fc->here_ptr=(size_t)here;
}

void here(forth_context_type* fc)
{
	push_sp(fc,(size_t)*fc->here_ptr);
}

void add_header(forth_context_type* fc, const char *name, char flags)
{
	char counter;
	size_t link;
	char *here;
	
	here=(char*)*fc->here_ptr;
	align(fc);
	link=(size_t)here;
	counter=strlen(name);
	add_byte(fc,flags);
	add_byte(fc,counter);
	strcpy(here,name);
	here+=counter;
	*fc->here_ptr=(size_t)here;
	add_byte(fc,counter);
	align(fc);
	add_cell(fc,*fc->latest_ptr);
	*fc->latest_ptr=link;
}

size_t add_primitive(forth_context_type* fc, const char *name, char flags, size_t func)
{
	size_t CFA;
	add_header(fc,name,flags);
	CFA=(size_t)*fc->here_ptr;
	add_cell(fc,func);
	return CFA;
}

void interpret_primitive(forth_context_type* fc, size_t f)
{
	switch(f)
	{
		case 1:		//lit
			lit(fc);
		break;
		case 2:		//here
			here(fc);
		break;
		case 3:		//endw
			endw(fc);
		break;
			//nop
	}
}

void forth_main_loop(forth_context_type* fc)
{
	size_t CFA;
	
	while(fc->stop==0)
	{
		CFA=*(size_t*)(*fc->PC);
		push_rp(fc,(size_t)fc->PC+1);		// PC+1 -> RP
		if(CFA==0)  //: definition
		{
			fc->PC=(size_t*)(CFA+fc->cell);  //PC=PFA
		}
		else
		{					// primitive
			interpret_primitive(fc,CFA);
			fc->PC=(size_t*)(*fc->RP++);   // PC <- RP
		}
	}
}



void make_words(forth_context_type* fc)
{
	size_t lit_cfa=add_primitive(fc,"lit",0,1);
	size_t here_cfa=add_primitive(fc,"here",0,2);
}

forth_context_type* forth_init(void)
{
	forth_context_type* fc;
	fc=malloc(sizeof(forth_context_type));  //should i check this?
	fc->mem=malloc(MEM_SIZE);
	fc->cell=sizeof(size_t);
	fc->stop=0;
	fc->SP=(size_t*)(fc->mem+STACK_DEPTH*fc->cell);
	fc->RP=(size_t*)(fc->mem+STACK_DEPTH*fc->cell*2);
	fc->begin=(char*)fc->RP+fc->cell;
	fc->here_ptr=(size_t*)fc->begin;
	fc->latest_ptr=(size_t*)fc->begin+fc->cell;
	*fc->latest_ptr=0;
	*fc->here_ptr=(size_t)fc->begin+fc->cell*2;
	make_words(fc);
	return fc;
}
