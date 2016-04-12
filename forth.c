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

void rd(forth_context_type* fc)
{
	size_t *adr;
	
	adr=(size_t*)*fc->SP;
	*fc->SP=*adr;
}

void wr(forth_context_type* fc)
{
	size_t *adr, val;
	
	adr=(size_t*)*(fc->SP++);
	val=*(fc->SP++);
	*adr=val;
}

void to_rp(forth_context_type* fc)
{
	size_t adr;
	
	adr=*fc->RP;
	*fc->RP=*(fc->SP++);
	*(--fc->RP)=adr;
}

void from_rp(forth_context_type* fc)
{
	size_t adr;
	
	adr=*(fc->RP++);
	*(--fc->SP)=*fc->RP;
	*fc->RP=adr;
}

void at_rp(forth_context_type* fc)
{
	*(--fc->SP)=*(fc->RP+1);
}

void var(forth_context_type* fc)
{
	push_sp(fc,*fc->PC+fc->cell);
}

void dup(forth_context_type* fc)
{
	fc->SP--;
	*fc->SP=*(fc->SP+1);
}

void swap(forth_context_type* fc)
{
	size_t val1,val2;
	val2=*(fc->SP++);
	val1=*fc->SP;
	*fc->SP=val2;
	*(--fc->SP)=val1;
}

void lit(forth_context_type* fc)
{
	size_t *tmp;
	tmp=(size_t*)*fc->RP;
	push_sp(fc,*tmp);
	*fc->RP=(size_t)(tmp+1);
}

void cmpl(forth_context_type* fc)
{
	size_t *tmp;
	tmp=(size_t*)*fc->RP;
	add_cell(fc,*tmp);
	*fc->RP=(size_t)(tmp+1);
}

void branch(forth_context_type* fc)
{
	size_t *tmp;
	tmp=(size_t*)*fc->RP;
	*fc->RP=*tmp;
}

void pbranch(forth_context_type* fc)
{
	size_t *tmp;
	tmp=(size_t*)*fc->RP;
	
	if(*(fc->SP++))	*fc->RP=(size_t)(tmp+1);
	else *fc->RP=*tmp;
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

size_t add_variable(forth_context_type* fc, const char *name, char flags, size_t val)
{
	size_t CFA;
	add_header(fc,name,flags);
	CFA=(size_t)*fc->here_ptr;
	add_cell(fc,4);     // var
	add_cell(fc,val);
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
		case 4:		//variable
			var(fc);
		break;
		case 5:		//compile
			cmpl(fc);
		break;
		case 6:		//@
			rd(fc);
		break;
		case 7:		//!
			wr(fc);
		break;
		case 8:		//>r
			to_rp(fc);
		break;
		case 9:		//r>
			from_rp(fc);
		break;
		case 10:	//r@
			at_rp(fc);
		break;
		case 11:	//dup
			dup(fc);
		break;
		case 12:	//swap
			swap(fc);
		break;
		case 13:	//branch
			branch(fc);
		break;
		case 14:	//?branch
			pbranch(fc);
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
	size_t state_cfa =	add_variable (fc,"state",0,0);
	size_t lit_cfa =	add_primitive(fc,"lit",0,1);
	size_t here_cfa =	add_primitive(fc,"here",0,2);
	size_t rd_cfa =		add_primitive(fc,"@",0,6);
	size_t wr_cfa =		add_primitive(fc,"!",0,7);
	size_t compile_cfa= add_primitive(fc,"compile",0,5);
	size_t to_rp_cfa= 	add_primitive(fc,">r",0,8);
	size_t from_rp_cfa= add_primitive(fc,"r>",0,9);
	size_t at_rp_cfa= 	add_primitive(fc,"r@",0,10);
	size_t dup_cfa= 	add_primitive(fc,"dup",0,11);
	size_t swap_cfa= 	add_primitive(fc,"swap",0,12);
	size_t branch_cfa= 	add_primitive(fc,"branch",0,13);
	size_t pbranch_cfa=	add_primitive(fc,"?branch",0,14);
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
