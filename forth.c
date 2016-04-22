#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include "forth.h"

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
		case 'd': fc->SP+=fc->cell;			break; //drop
		case 'D': dup(fc);					break; //dup
		case 's': swap(fc);					break; //swap
		case 'l': push(fc,next_cell(fc));	break; //lit
		case '+': add(fc);  				break; //+
		case '-': sub(fc);  				break; //-
		case '&': and(fc);  				break; // and
		case '|': or(fc);   				break; // or
		
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
	size_t init_cfa;
	fc=malloc(sizeof(forth_context_type));
	if(fc)
	{
		fc->mem=malloc(MEM_SIZE);
		fc->cell=sizeof(size_t);
		
		fc->stop=0;
	}
	return fc;
}
