#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include "forth.h"

/* +------------------------------------+----------+---------------------------+-----------+
 * |                 NFA                |   LFA    |           CFA             |   PFA     |  
 * +-------+--------+----------+--------+----------+----------+----------+-----+-----------+
 * | flags | length |   name   | length |   link   | c_length |  code    | ret | code/data |
 * +-------+--------+----------+--------+----------+----------+----------+-----+-----------+
 * | 1b    |  1b    | length b |   1b   |  1 cell  |    1b    |c_length b| 1b  |   ?b      |
 * +-------+--------+----------+--------+----------+----------+----------+-----+-----------+
 */


void push(forth_context_type *fc, size_t val)
{
	fc->SP-=fc->cell;
	*(size_t *)(fc->mem+(fc->SP))=val;
}

size_t pop(forth_context_type *fc)
{
	size_t val;
	val=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
	return val;
}

void dup(forth_context_type *fc)
{
	   *(size_t *)(fc->mem+fc->SP-fc->cell)=*(size_t *)(fc->mem+fc->SP);
	   fc->SP-=fc->cell;
}

void swap(forth_context_type *fc)
{
		size_t *c1,*c2,tmp;
		c1=(size_t *)(fc->mem+fc->SP+fc->cell);
		c2=(size_t *)(fc->mem+fc->SP);
		tmp=*c1;
		*c1=*c2;
		*c2=tmp;
}

size_t next_cell(forth_context_type *fc)
{
	size_t val;
	char i;
	
	val=0;
	for(i=0;i<fc->cell;i++) val|=*(fc->mem+(fc->PC++))<<(8*i);
	return val;
}

void add(forth_context_type *fc)
{
	
	*(size_t *)(fc->mem+(fc->SP+fc->cell))+=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

void sub(forth_context_type *fc)
{
	
	*(size_t *)(fc->mem+(fc->SP+fc->cell))-=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

void mul(forth_context_type *fc)
{
	
	*(size_t *)(fc->mem+(fc->SP+fc->cell))*=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

void div_(forth_context_type *fc)
{
	
	*(size_t *)(fc->mem+(fc->SP+fc->cell))/=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

void mod(forth_context_type *fc)
{
	
	*(size_t *)(fc->mem+(fc->SP+fc->cell))%=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}


void and(forth_context_type *fc)
{
	
	*(size_t *)(fc->mem+(fc->SP+fc->cell))&=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

void or(forth_context_type *fc)
{
	
	*(size_t *)(fc->mem+(fc->SP+fc->cell))|=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}


void branch(forth_context_type *fc)
{
	size_t adr;
	adr=next_cell(fc);
	fc->PC=adr;
}

void cbranch(forth_context_type *fc)
{
	size_t adr,val;
	val=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
	if(!val)
	{
		adr=next_cell(fc);
		fc->PC=adr;
	}
}

void call(forth_context_type *fc)
{
	size_t adr;
	adr=next_cell(fc);
	fc->RP-=fc->cell;
	*(size_t *)(fc->mem+(fc->RP))=fc->PC;
	fc->PC=adr;
}

void ret(forth_context_type *fc)
{
	size_t adr;
	adr=*(size_t *)(fc->mem+(fc->RP));
	fc->RP+=fc->cell;
	fc->PC=adr;
}

void more(forth_context_type *fc)
{
	size_t val1,val2;
	val2=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
	val1=*(size_t *)(fc->mem+(fc->SP));
	*(size_t *)(fc->mem+(fc->SP))= (val1 > val2) ? -1 : 0;
}

void less(forth_context_type *fc)
{
	size_t val1,val2;
	val2=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
	val1=*(size_t *)(fc->mem+(fc->SP));
	*(size_t *)(fc->mem+(fc->SP))= (val1 < val2) ? -1 : 0;
}

void eq(forth_context_type *fc)
{
	size_t val1,val2;
	val2=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
	val1=*(size_t *)(fc->mem+(fc->SP));
	*(size_t *)(fc->mem+(fc->SP))= (val1 == val2) ? -1 : 0;
}

void at(forth_context_type *fc)
{
	size_t val;
	val=*(size_t *)(fc->mem+(fc->SP));
	*(size_t *)(fc->mem+(fc->SP))=*(size_t *)(fc->mem+val);
}

void to(forth_context_type *fc)
{
	size_t adr,val;
	adr=*(size_t *)(fc->mem+(fc->SP)); fc->SP+=fc->cell;
	val=*(size_t *)(fc->mem+(fc->SP)); fc->SP+=fc->cell;
	
	*(size_t *)(fc->mem+adr)=val;
}


void to_r(forth_context_type *fc)
{
	fc->RP-=fc->cell;
	*(size_t *)(fc->mem+(fc->RP))=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

void from_r(forth_context_type *fc)
{
	fc->SP-=fc->cell;
	*(size_t *)(fc->mem+(fc->SP))=*(size_t *)(fc->mem+(fc->RP));
	fc->RP+=fc->cell;
}

void in(forth_context_type *fc)
{
	push(fc,getchar());
}

void out(forth_context_type *fc)
{
	putchar(pop(fc));
}


void forth_vm_execute_instruction(forth_context_type *fc, char cmd)
{
	switch(cmd)
	{
		case '0': push(fc,0);				break;
		case '1': push(fc,1);				break;
		case '2': push(fc,2);				break;
		case '3': push(fc,3);				break;
		case '4': push(fc,4);				break;
		case '5': push(fc,5);				break;
		case '6': push(fc,6);				break;
		case '7': push(fc,7);				break;
		case '8': push(fc,8);				break;
		case '9': push(fc,9);				break;
		case '@': at(fc);					break; //@
		case '!': to(fc);					break; //!
		case 'd': fc->SP+=fc->cell;			break; //drop
		case 'D': dup(fc);					break; //dup
		case 's': swap(fc);					break; //swap
		case 'l': push(fc,next_cell(fc));	break; //lit
		case '+': add(fc);  				break; //+
		case '-': sub(fc);  				break; //-
		case '*': mul(fc);					break; //*
		case '/': div_(fc);					break; // /
		case '%': mod(fc);					break; //mod
		case '&': and(fc);  				break; // and
		case '|': or(fc);   				break; // or
		case '>': more(fc);					break; // >
		case '<': less(fc);					break;  // <
		case '=': eq(fc);					break; // =
		case 'b': branch(fc);				break; // branch
		case '?': cbranch(fc);				break; // ?branch
		case 'c': call(fc);					break; // call
		case 'r': ret(fc);					break; // ret
		case 't': to_r(fc);					break; // >R
		case 'f': from_r(fc);				break; // R>
		case 'i': in(fc);					break; // key
		case 'o': out(fc);					break; // emit
		case '_': fc->stop=1;				break; // stop
		case 1:	  push(fc,fc->SP);			break; // SP@
		case 2:	  fc->SP=pop(fc);			break; // SP!
		case 3:	  push(fc,fc->RP);			break; // RP@
		case 4:	  fc->RP=pop(fc);			break; // RP!
	}
}

void forth_vm_main_loop(forth_context_type *fc)
{
	char cmd;
	while(fc->stop==0)
	{
		cmd=*(fc->mem+fc->PC++);
		forth_vm_execute_instruction(fc,cmd);
	}
}

forth_context_type* forth_init(void)
{
	forth_context_type* fc;
	fc=malloc(sizeof(forth_context_type));
	if(fc)
	{
		fc->mem=malloc(MEM_SIZE);
		fc->cell=sizeof(size_t);
		
		fc->stop=0;
	}
	return fc;
}
