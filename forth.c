#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "forth.h"


// word fields
// +-------+---------+------+---------+-----+-----+-----+
// | flags | counter | name | counter | LFA | CFA | PFA | 
// +-------+---------+------+---------+-----+-----+-----+
//    1b      1b        ?       1b      cell cell  ?cells
// CFA = NULL  - :
// CFA = ADR   - primitive


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
	push_sp(fc,*(size_t *)(*fc->RP));
	*fc->RP+=fc->cell;
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
	here=(char*)*fc->here_ptr;
	m=(size_t)here % fc->cell;
	if(m)
	{
		here+=(fc->cell-m);
	}
	*fc->here_ptr=(size_t)here;
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
	add_cell(fc,*fc->latest_ptr);
	*fc->latest_ptr=link;
}


void forth_main_loop(forth_context_type* fc)
{
	void (*dummy)(forth_context_type* fc);
	
	while(fc->stop==0)
	{
		if((*fc->pc)==0)  //: definition
		{
			fc->pc=(size_t*)*(fc->pc+1);			//pc = pfa@ 
			push_rp(fc,(size_t)fc->pc+2);		// push pfa+1
		}
		else
		{					// primitive
			dummy=(void*)*fc->pc;
			dummy(fc);
			fc->pc=(size_t*)*(size_t*)(*fc->RP++);
		}
	}
}

forth_context_type* forth_init(void)
{
	forth_context_type* fc;
	fc=malloc(sizeof(forth_context_type));  //should i check this&
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
	return fc;
}
