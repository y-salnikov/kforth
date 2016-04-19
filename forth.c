#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
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
	push_sp(fc,(*fc->PC)+fc->cell);
}

void cnst(forth_context_type* fc)
{
	push_sp(fc,*(size_t *)((*fc->PC)+fc->cell));
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
	size_t* tmp;
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

void plus(forth_context_type* fc)
{
	size_t tmp;
	tmp=*(fc->SP++);
	*fc->SP+=tmp;
}

void minus(forth_context_type* fc)
{
	size_t tmp;
	tmp=*(fc->SP++);
	*fc->SP-=tmp;
}

void mult(forth_context_type* fc)
{
	size_t tmp;
	tmp=*(fc->SP++);
	*fc->SP*=tmp;
}

void div_(forth_context_type* fc)
{
	size_t tmp;
	tmp=*(fc->SP++);
	*fc->SP/=tmp;
}

void mod_(forth_context_type* fc)
{
	size_t tmp;
	tmp=*(fc->SP++);
	*fc->SP%=tmp;
}

void key(forth_context_type* fc)
{
	*(--fc->SP)=getc(stdin);
}

void emit(forth_context_type* fc)
{
	putc(*(fc->SP++),stdout);
}

void die(forth_context_type* fc)
{
	fc->stop=1;
}

void or(forth_context_type* fc)
{
	size_t tmp;
	tmp=*(fc->SP++);
	*fc->SP|=tmp;
}

void and(forth_context_type* fc)
{
	size_t tmp;
	tmp=*(fc->SP++);
	*fc->SP&=tmp;
}

void xor(forth_context_type* fc)
{
	size_t tmp;
	tmp=*(fc->SP++);
	*fc->SP^=tmp;
}

void crd(forth_context_type* fc)
{
	size_t *adr;
	adr=(size_t*)*fc->SP;
	*fc->SP=(*adr) & 0xff;
}

void cwr(forth_context_type* fc)
{
	size_t val;
	char *cadr;
	cadr=(char*)*(fc->SP++);
	val=*(fc->SP++);
	*cadr=(val & 0xff);
}

void allot(forth_context_type* fc)
{
	*fc->here_ptr+=*(fc->SP++);
}

void coma(forth_context_type* fc)
{
	add_cell(fc,*(fc->SP++));
}

void word(forth_context_type* fc)
{
	char del, *buf, *here, *count;
	size_t offset,blk;
	here=(char*)*fc->here_ptr;
	count=here++;
	del=*(fc->SP++); //delimeter
	offset=*(size_t *)(fc->in_cfa+fc->cell);  // >in @
	blk=*(size_t *)(fc->blk_cfa+fc->cell);
	if (blk) buf=fc->block_buf;
	else buf=(char*)(fc->tib_cfa+fc->cell); // tib 
	*count=0;
	while( (buf[offset]==del) && (offset<=TIB_SIZE) && (buf[offset]!=0)) offset++;
	while( (buf[offset]!=del) && (offset<=TIB_SIZE) && (buf[offset]!=0))
	{
		*(here++)=buf[offset++];
		(*count)++;
	}
	*fc->here_ptr=(size_t)here;
	*(size_t*)(fc->in_cfa+fc->cell)=offset;
	*(--fc->SP)=(size_t)count;
}

void type(forth_context_type* fc)
{
	size_t length;
	char *adr;
	
	length=*(fc->SP++);
	adr=(char *)*(fc->SP++);
	while(length--)
	{
		*(--fc->SP)=*(adr++);
		emit(fc);
	}
}

void find(forth_context_type* fc)
{
	char *word_ptr;
	char flags, count, *name,m,*tmpl_name,tmpl_count, *tmpl;
	size_t CFA,*LFA;
	word_ptr=(char *)*(fc->latest_ptr);
	
	tmpl=(char*)*(fc->SP++);
	tmpl_count=*tmpl;
	tmpl_name=(tmpl+1);
	
	while(word_ptr!=NULL)
	{
		flags=*(word_ptr++);
		count=*(word_ptr++);
		name=word_ptr; word_ptr+=count;
		word_ptr++;
		m=((size_t)word_ptr) % fc->cell;
		if(m)	word_ptr+=(fc->cell-m);
		LFA=(size_t*)word_ptr;  word_ptr+=fc->cell;
		CFA=(size_t)word_ptr;
		word_ptr=(char*)*LFA;
		if(count==tmpl_count)
		{
			if(strncasecmp(name,tmpl_name,tmpl_count)==0)
			{
				break;
			}
		}
	}
	if(word_ptr==NULL)
	{
		*(--fc->SP)=(size_t)tmpl;
		*(--fc->SP)=0;
	}
	else
	{
		*(--fc->SP)=CFA;
		if(flags & IMMEDIATE) *(--fc->SP)=1;
		else *(--fc->SP)=(size_t)(-1);
	}
}

void add_header(forth_context_type* fc, const char *name, char flags)
{
	char counter;
	size_t link;
	char *here;
	
	align(fc);
	here=(char*)*fc->here_ptr;
	link=(size_t)here;
	counter=strlen(name);
	add_byte(fc,flags);
	add_byte(fc,counter);
	here=(char*)*fc->here_ptr;
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

size_t add_variable(forth_context_type* fc, const char *name, size_t val)
{
	size_t CFA;
	add_header(fc,name,0);
	CFA=(size_t)*fc->here_ptr;
	add_cell(fc,4);     // var
	add_cell(fc,val);
	return CFA;
}

size_t add_definition(forth_context_type* fc, const char *name, char flags, int num, ...)
{
	size_t CFA;
	va_list ap;
	int i;
	
	add_header(fc,name,flags);
	CFA=(size_t)*fc->here_ptr;
	add_cell(fc,0);     // :
	va_start (ap, num);
	for(i=0;i<num;i++)
		add_cell(fc,va_arg(ap,size_t));
	va_end(ap);
	return CFA;
}

size_t add_constant(forth_context_type* fc, const char *name, size_t val)
{
	size_t CFA;
	add_header(fc,name,0);
	CFA=(size_t)*fc->here_ptr;
	add_cell(fc,23);     // cnst
	add_cell(fc,val);
	return CFA;
}




void interpret_primitive(forth_context_type* fc, size_t f)
{
	switch(f)
	{
		case 1:					lit(fc); 		break; //lit
		case 2:		 			here(fc); 		break; //here
		case 3:		 			endw(fc); 		break; //endw
		case 4:		  			var(fc); 		break; //variable
		case 5:					cmpl(fc);		break; //compile
		case 6:					rd(fc);			break; //@
		case 7:					wr(fc); 		break; //!
		case 8:					to_rp(fc);		break; //>r
		case 9:					from_rp(fc);	break; //r>
		case 10:				at_rp(fc);		break; //r@
		case 11:				dup(fc);		break; //dup
		case 12:	 			swap(fc); 		break; //swap
		case 13:				branch(fc);		break; //branch
		case 14:				pbranch(fc);	break; //?branch
		case 15:				plus(fc);		break; //+
		case 16:				minus(fc);		break; //-
		case 17:				mult(fc);		break; //*		
		case 18:				div_(fc);		break; // /	
		case 19:				mod_(fc);		break; // mod	
		case 20:				key(fc);		break; //key	
		case 21:				emit(fc);		break; //emit	
		case 22:				die(fc);		break; //die	
		case 23:				cnst(fc);		break; //const	
		case 24:				or(fc);			break; //or	
		case 25:				and(fc);		break; //and	
		case 26:				xor(fc);		break; //xor	
		case 27:				crd(fc);		break; //c@	
		case 28:				cwr(fc);		break; //c!	
		case 29:				allot(fc);		break; //allot
		case 30:				coma(fc);		break; //,
		case 31:				word(fc);		break; //word
		case 32:				type(fc);		break; // type
			//nop
	}
}

void forth_main_loop(forth_context_type* fc)
{
	size_t CFA;
	
	while(fc->stop==0)
	{
//		printf("PC=0x%lX\n",(size_t)fc->PC);
		CFA=*(size_t*)(*fc->PC);
//		printf("CFA=%ld\n",CFA);
		push_rp(fc,(size_t)(fc->PC+1));		// PC+1 -> RP
		if(CFA==0)  //: definition
		{
			fc->PC=(size_t*)((*fc->PC)+fc->cell);  //PC=PFA
//			printf("new PC=0x%lX\n",(size_t)fc->PC);
		}
		else
		{					// primitive
			interpret_primitive(fc,CFA);
			fc->PC=(size_t*)(*(fc->RP++));   // PC <- RP
		}
	}
}

void forth_execute_word(forth_context_type* fc, size_t CFA)
{
	size_t wrds[2];
	wrds[0]=CFA;
	wrds[1]=fc->die_cfa;
	
	*(--fc->RP)=(size_t)&wrds[1];
	fc->PC=wrds;
	if(*(size_t*)CFA)
	{
		interpret_primitive(fc,*(size_t*)CFA);
	}
	else
	{
		fc->stop=0;
		forth_main_loop(fc);
	}
}

void forth_print_cells(size_t* adr, size_t N)
{
	while(N--)	printf("0x%lX\n",(unsigned long int)*(adr++));
}

size_t make_words(forth_context_type* fc)
{
	size_t state_cfa =	add_variable (fc,"state",0);
	fc->blk_cfa=		add_variable (fc,"blk",0);
	fc->in_cfa=			add_variable (fc,">in",0);
	size_t sp0_cfa=		add_constant (fc, "sp0", (size_t)fc->SP);
	size_t rp0_cfa=		add_constant (fc, "rp0", (size_t)fc->RP);
	size_t lit_cfa =	add_primitive(fc,"lit",0,1);
	size_t here_cfa =	add_primitive(fc,"here",0,2);
	size_t endw_cfa =	add_primitive(fc,";s",0,3);
	size_t rd_cfa =		add_primitive(fc,"@",0,6);
	size_t wr_cfa =		add_primitive(fc,"!",0,7);
	size_t compile_cfa=	add_primitive(fc,"compile",0,5);
	size_t to_rp_cfa= 	add_primitive(fc,">r",0,8);
	size_t from_rp_cfa=	add_primitive(fc,"r>",0,9);
	size_t at_rp_cfa= 	add_primitive(fc,"r@",0,10);
	size_t dup_cfa= 	add_primitive(fc,"dup",0,11);
	size_t swap_cfa= 	add_primitive(fc,"swap",0,12);
	size_t branch_cfa= 	add_primitive(fc,"branch",0,13);
	size_t pbranch_cfa=	add_primitive(fc,"?branch",0,14);
	size_t plus_cfa=	add_primitive(fc,"+",0,15);
	size_t minus_cfa=	add_primitive(fc,"-",0,16);
	size_t mul_cfa=		add_primitive(fc,"*",0,17);
	size_t div_cfa=		add_primitive(fc,"/",0,18);
	size_t mod_cfa=		add_primitive(fc,"mod",0,19);
	size_t key_cfa=		add_primitive(fc,"key",0,20);
	size_t emit_cfa=	add_primitive(fc,"emit",0,21);
	fc->die_cfa=		add_primitive(fc,"die",0,22);
	size_t or_cfa= 		add_primitive(fc,"or",0,24);
	size_t and_cfa= 	add_primitive(fc,"and",0,25);
	size_t xor_cfa= 	add_primitive(fc,"xor",0,26);
	size_t crd_cfa=		add_primitive(fc,"c@",0,27);
	size_t cwr_cfa=		add_primitive(fc,"c!",0,28);
	size_t latest_cfa=	add_constant (fc, "latest",(size_t)fc->latest_ptr);
	size_t cell_cfa=	add_constant (fc,"cell",fc->cell);
	size_t allot_cfa=	add_primitive(fc,"allot",0,29);
	size_t coma_cfa=	add_primitive(fc,",",0,30);
	fc->tib_cfa=		add_variable (fc, "tib", 0);
	*(--fc->SP)=TIB_SIZE;
	forth_execute_word(fc,allot_cfa);
	size_t word_cfa=	add_primitive(fc,"word",0,31);
	size_t type_cfa=	add_primitive(fc,"type",0,32);
	
	// : immediate IMMEDIATE_FLAG latest @ c@ or latest @ c! ; 
	size_t immediate_cfa= add_definition(fc,"immediate",IMMEDIATE, 10 , lit_cfa, IMMEDIATE, latest_cfa, rd_cfa, crd_cfa, or_cfa, 
	                                                                    latest_cfa, rd_cfa, cwr_cfa, endw_cfa );
	// : >mark here cell allot ; 
	size_t to_mark_cfa=   add_definition(fc,">mark", 0 , 4 , here_cfa, cell_cfa, allot_cfa, endw_cfa);
	// : >resolve here swap ! ;
	size_t to_resolve_cfa=add_definition(fc,">resolve", 0 , 4 , here_cfa, swap_cfa, wr_cfa, endw_cfa);
	// : <mark here ;
	size_t from_mark_cfa =add_definition(fc,"<mark", 0 , 2 , here_cfa, endw_cfa);
	// : <resolve , ; 
	size_t from_resolve_cfa= add_definition(fc,"<resolve", 0 , 2, coma_cfa, endw_cfa);
	
	size_t init_cfa= add_definition(fc,"init",0,1,endw_cfa);
	return init_cfa;
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
		fc->SP=(size_t*)(fc->mem+STACK_DEPTH);
		fc->RP=(size_t*)(fc->mem+STACK_DEPTH*2);
		fc->begin=(char*)fc->RP+1;
		fc->here_ptr=(size_t*)fc->begin;
		fc->latest_ptr=(size_t*)fc->begin+1;
		*fc->latest_ptr=0;
		*fc->here_ptr=(size_t)fc->begin+fc->cell*2;
		init_cfa=make_words(fc);
	
		forth_execute_word(fc,init_cfa);
	}
	return fc;
}
