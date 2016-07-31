#include <linux/gfp.h>
#include <linux/slab.h>
#include "linux/types.h"
#include "forth_img.h"
#include "stddef.h"
#include <linux/circ_buf.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include "forth.h"


/* +------------------------------------+----------+---------------------------+-----------+
 * |                 NFA                |   LFA    |           CFA             |   PFA     |  
 * +-------+--------+----------+--------+----------+----------+----------+-----+-----------+
 * | flags | length |   name   | length |   link   | c_length |  code    | ret | code/data |
 * +-------+--------+----------+--------+----------+----------+----------+-----+-----------+
 * | 1b    |  1b    | length b |   1b   |  1 cell  |    1b    |c_length b| 1b  |   ?b      |
 * +-------+--------+----------+--------+----------+----------+----------+-----+-----------+
 */


static struct task_struct * forthThread = NULL;


static inline  void push(forth_context_type *fc, size_t val)
{
	fc->SP-=fc->cell;
	*(size_t *)(fc->mem+(fc->SP))=val;
}

static inline  size_t pop(forth_context_type *fc)
{
	size_t val;
	val=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
	return val;
}

static inline  void dup(forth_context_type *fc)
{
	   *(size_t *)(fc->mem+fc->SP-fc->cell)=*(size_t *)(fc->mem+fc->SP);
	   fc->SP-=fc->cell;
}

static inline  void swap_(forth_context_type *fc)
{
		size_t *c1,*c2,tmp;
		c1=(size_t *)(fc->mem+fc->SP+fc->cell);
		c2=(size_t *)(fc->mem+fc->SP);
		tmp=*c1;
		*c1=*c2;
		*c2=tmp;
}

static inline  size_t next_cell(forth_context_type *fc)
{
	size_t val,val_;
	char i;
	
	val=0;
	for(i=0;i<fc->cell;i++)
	{
		val_=(*(fc->mem+(fc->PC++)));
		val|=val_<<(8*i);
	}
	return val;
}

static inline  void add(forth_context_type *fc)
{
	
	*(size_t *)(fc->mem+(fc->SP+fc->cell))+=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

static inline  void sub(forth_context_type *fc)
{
	
	*(size_t *)(fc->mem+(fc->SP+fc->cell))-=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

static inline  void mul(forth_context_type *fc)
{
	
	*(ptrdiff_t *)(fc->mem+(fc->SP+fc->cell))*=*(ptrdiff_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

static inline  void div_(forth_context_type *fc)
{
	
	*(ptrdiff_t *)(fc->mem+(fc->SP+fc->cell))/=*(ptrdiff_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

static inline  void udiv(forth_context_type *fc)
{
	
	*(size_t *)(fc->mem+(fc->SP+fc->cell))/=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

static inline  void mod(forth_context_type *fc)
{
	
	*(ptrdiff_t *)(fc->mem+(fc->SP+fc->cell))%=*(ptrdiff_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

static inline  void umod(forth_context_type *fc)
{
	
	*(size_t *)(fc->mem+(fc->SP+fc->cell))%=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

static inline  void and(forth_context_type *fc)
{
	
	*(size_t *)(fc->mem+(fc->SP+fc->cell))&=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

static inline  void or(forth_context_type *fc)
{
	
	*(size_t *)(fc->mem+(fc->SP+fc->cell))|=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

static inline  void xor(forth_context_type *fc)
{
	
	*(size_t *)(fc->mem+(fc->SP+fc->cell))^=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

static inline  void branch(forth_context_type *fc)
{
	size_t adr;
	adr=next_cell(fc);
	fc->PC=adr;
}

static inline  void cbranch(forth_context_type *fc)
{
	size_t adr,val;
	val=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
	if(!val)
	{
		adr=next_cell(fc);
		fc->PC=adr;
	}
	else next_cell(fc);
}

static inline  void call(forth_context_type *fc)
{
	size_t adr;
	adr=next_cell(fc);
	fc->RP-=fc->cell;
	*(size_t *)(fc->mem+(fc->RP))=fc->PC;
	fc->PC=adr;
}

static inline  void ret(forth_context_type *fc)
{
	size_t adr;
	adr=*(size_t *)(fc->mem+(fc->RP));
	fc->RP+=fc->cell;
	fc->PC=adr;
}

static inline  void more(forth_context_type *fc)
{
	ptrdiff_t val1,val2;
	val2=*(ptrdiff_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
	val1=*(ptrdiff_t *)(fc->mem+(fc->SP));
	*(size_t *)(fc->mem+(fc->SP))= (val1 > val2) ? -1 : 0;
}

static inline  void less(forth_context_type *fc)
{
	ptrdiff_t val1,val2;
	val2=*(ptrdiff_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
	val1=*(ptrdiff_t *)(fc->mem+(fc->SP));
	*(size_t *)(fc->mem+(fc->SP))= (val1 < val2) ? -1 : 0;
}

static inline  void eq(forth_context_type *fc)
{
	size_t val1,val2;
	val2=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
	val1=*(size_t *)(fc->mem+(fc->SP));
	*(size_t *)(fc->mem+(fc->SP))= (val1 == val2) ? -1 : 0;
}

static inline  void at(forth_context_type *fc)
{
	size_t val;
	val=*(size_t *)(fc->mem+(fc->SP));
	*(size_t *)(fc->mem+(fc->SP))=*(size_t *)(fc->mem+val);
}

static inline  void cat(forth_context_type *fc)
{
	size_t val;
	val=*(size_t *)(fc->mem+(fc->SP));
	*(size_t *)(fc->mem+(fc->SP))=(*(size_t *)(fc->mem+val)) & 0xff;
}



static inline  void to(forth_context_type *fc)
{
	size_t adr,val;
	adr=*(size_t *)(fc->mem+(fc->SP)); fc->SP+=fc->cell;
	val=*(size_t *)(fc->mem+(fc->SP)); fc->SP+=fc->cell;
	
	*(size_t *)(fc->mem+adr)=val;
}

static inline  void cto(forth_context_type *fc)
{
	size_t adr,val;
	adr=*(size_t *)(fc->mem+(fc->SP)); 			fc->SP+=fc->cell;
	val=(*(size_t *)(fc->mem+(fc->SP))) & 0xff; fc->SP+=fc->cell;
	
	*(fc->mem+adr)=val;
}

static inline  void to_r(forth_context_type *fc)
{
	fc->RP-=fc->cell;
	*(size_t *)(fc->mem+(fc->RP))=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

static inline  void from_r(forth_context_type *fc)
{
	fc->SP-=fc->cell;
	*(size_t *)(fc->mem+(fc->SP))=*(size_t *)(fc->mem+(fc->RP));
	fc->RP+=fc->cell;
}

static inline  void in(forth_context_type *fc)
{
	fc->SP-=fc->cell;
	*(size_t *)(fc->mem+(fc->SP))=fc->in.buf[fc->in.tail];
	fc->in.tail=(fc->in.tail+1) & (TIB_SIZE-1);
}

static inline void in_ready(forth_context_type *fc)
{
	fc->SP-=fc->cell;
	*(size_t *)(fc->mem+(fc->SP))= (CIRC_CNT(fc->in.head,fc->in.tail,TIB_SIZE));
}


static inline  void out(forth_context_type *fc)
{
	fc->out.buf[fc->out.head]=0xff & (*(size_t *)(fc->mem+(fc->SP)) );
	fc->SP+=fc->cell;
	fc->out.head=(fc->out.head+1) & (TIB_SIZE-1);
}


static inline void out_ready(forth_context_type *fc)
{
	fc->SP-=fc->cell;
	*(size_t *)(fc->mem+(fc->SP))=CIRC_SPACE(fc->out.head,fc->out.tail,TIB_SIZE);
}


void put_to_in(forth_context_type *fc, char c)
{
	set_current_state(TASK_INTERRUPTIBLE);
	while (CIRC_SPACE(fc->in.head,fc->in.tail,TIB_SIZE)==0)
	{
		schedule_timeout(1);
		set_current_state(TASK_INTERRUPTIBLE);
		if(fc->stop) return;
	}
	set_current_state(TASK_RUNNING);
	fc->in.buf[fc->in.head]=c;
	fc->in.head=(fc->in.head+1) & (TIB_SIZE-1);
}

char read_from_out(forth_context_type *fc)
{
	char c;
	set_current_state(TASK_INTERRUPTIBLE);
	while (CIRC_CNT(fc->out.head,fc->out.tail,TIB_SIZE)==0)
	{
		schedule_timeout(1);
		set_current_state(TASK_INTERRUPTIBLE);
		if(fc->stop) return 0;
	}
	set_current_state(TASK_RUNNING);
	c=fc->out.buf[fc->out.tail];
	fc->out.tail=(fc->out.tail+1) & (TIB_SIZE-1);
	return c;
}



static inline  void shl(forth_context_type *fc)
{
	
	*(size_t *)(fc->mem+(fc->SP+fc->cell))<<=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

static inline  void shr(forth_context_type *fc)
{
	
	*(size_t *)(fc->mem+(fc->SP+fc->cell))<<=*(size_t *)(fc->mem+(fc->SP));
	fc->SP+=fc->cell;
}

static inline  void adr0(forth_context_type *fc)
{
	push(fc,(size_t)fc->mem);
}

static inline void kalsym_lookup(forth_context_type *fc)
{
	size_t *sp;
	char *str;
	sp=(size_t *)(fc->mem+(fc->SP));
	str=(char *)*sp;
	*sp=kallsyms_lookup_name(str);
//	printk("PAR:%lu,'%s'\n",strlen(str),str);
}

static inline void kcall(forth_context_type *fc) // arg1, arg2, arg3...argN, N, adr -- retval
{
	size_t N;
	size_t adr;
	size_t args[KARGS];
	size_t ret=0;
	int i;
	kall0 f0;
	kall1 f1;
	kall2 f2;
	kall3 f3;
	kall4 f4;
	kall5 f5;
	kall6 f6;
	kall7 f7;
	kall8 f8;
	kall9 f9;
	kall10 f10;
	kall11 f11;
	kall12 f12;
	kall13 f13;
	kall14 f14;
	kall15 f15;
	kall16 f16;
	
	adr=pop(fc);
	N=pop(fc);
	for(i=1;i<=N;i++)
	{
		args[N-i]=pop(fc);
	}
	switch(N)
	{
		case 0: f0=(kall0)adr;
				ret=f0();
				break;
		case 1: f1=(kall1)adr;
				ret=f1(args[0]);
				break;
		case 2: f2=(kall2)adr;
				ret=f2(args[0],args[1]);
				break;
		case 3: f3=(kall3)adr;
				ret=f3(args[0],args[1],args[2]);
				break;
		case 4: f4=(kall4)adr;
				ret=f4(args[0],args[1],args[2],args[3]);
				break;
		case 5: f5=(kall5)adr;
				ret=f5(args[0],args[1],args[2],args[3],args[4]);
				break;
		case 6: f6=(kall6)adr;
				ret=f6(args[0],args[1],args[2],args[3],args[4],args[5]);
				break;
		case 7: f7=(kall7)adr;
				ret=f7(args[0],args[1],args[2],args[3],args[4],args[5],args[6]);
				break;
		case 8: f8=(kall8)adr;
				ret=f8(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7]);
				break;
		case 9: f9=(kall9)adr;
				ret=f9(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8]);
				break;
		case 10: f10=(kall10)adr;
				ret=f10(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9]);
				break;
		case 11: f11=(kall11)adr;
				ret=f11(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10]);
				break;
		case 12: f12=(kall12)adr;
				ret=f12(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10],args[11]);
				break;
		case 13: f13=(kall13)adr;
				ret=f13(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10],args[11],args[12]);
				break;
		case 14: f14=(kall14)adr;
				ret=f14(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10],args[11],args[12],args[13]);
				break;
		case 15: f15=(kall15)adr;
				ret=f15(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10],args[11],args[12],args[13],args[14]);
				break;
		case 16: f16=(kall16)adr;
				ret=f16(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10],args[11],args[12],args[13],args[14],args[15]);
				break;
	}
	push(fc,ret);
}


static inline  void forth_vm_execute_instruction(forth_context_type *fc, char cmd)
{
//	printf("%c\n",cmd);
//	getchar();
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
		case 's': swap_(fc);					break; //swap
		case 'l': push(fc,next_cell(fc));	break; //lit
		case '+': add(fc);  				break; //+
		case '-': sub(fc);  				break; //-
		case '*': mul(fc);					break; //*
		case '/': div_(fc);					break; // /
		case '%': mod(fc);					break; //mod
		case '&': and(fc);  				break; // and
		case '|': or(fc);   				break; // or
		case '^': xor(fc);   				break; // xor
		case '>': more(fc);					break; // >
		case '<': less(fc);					break;  // <
		case '=': eq(fc);					break; // =
		case 'b': branch(fc);				break; // branch
		case '?': cbranch(fc);				break; // ?branch
		case 'c': call(fc);					break; // call
		case 'r': ret(fc);					break; // ret
		case 't': to_r(fc);					break; // >R
		case 'f': from_r(fc);				break; // R>
		case 'i': in(fc);					break; // in
		case 'o': out(fc);					break; // out
		case '_': fc->stop=1;				break; // stop
		case 'A': adr0(fc);					break; // @0
		case 1:	  push(fc,fc->SP);			break; // SP@
		case 2:	  fc->SP=pop(fc);			break; // SP!
		case 3:	  push(fc,fc->RP);			break; // RP@
		case 4:	  fc->RP=pop(fc);			break; // RP!
		case 5:	 shl(fc);					break; // <<
		case 6:	 shr(fc);					break; // >>
		case 7:  push(fc,*(size_t *)(fc->mem+fc->RP)); break; // i
		case 8:  cat(fc);					break; // c@
		case 9:  cto(fc);					break; // c!
		case 10: set_current_state(TASK_INTERRUPTIBLE); schedule_timeout(1);				break; // nop
		case 11: in_ready(fc);				break; // ?in
		case 12: out_ready(fc);				break; // ?out
		case 16: umod(fc);					break; // umod
		case 17: udiv(fc);					break; // u/
// kernel
		case 'K': kalsym_lookup(fc);		break; // lookup kallsym address
		case 18:  kcall(fc);				break; // kcall
	}
}

static int forth_vm_main_loop(void *data)
{
	forth_context_type *fc;
	char cmd;
	fc=data;
	if(fc==NULL) return 0;
	while(fc->stop==0)
	{
		if(kthread_should_stop()) break;
		cmd=*(fc->mem+fc->PC++);
		forth_vm_execute_instruction(fc,cmd);
	}
	//if(fc->stop) do_exit(0);
	return 0;
}

forth_context_type* forth_init(void)
{
	size_t i;
	forth_context_type* fc;
	fc=kmalloc(sizeof(forth_context_type),GFP_KERNEL);
	if(fc==NULL) return fc;
	fc->in.buf=kmalloc(TIB_SIZE,GFP_KERNEL);
	fc->out.buf=kmalloc(TIB_SIZE,GFP_KERNEL);
	fc->in.tail=0; fc->in.head=0;
	fc->out.tail=0; fc->out.head=0;
	fc->mem=kmalloc(MEM_SIZE,GFP_KERNEL);
	for(i=0;i<forth_img_length;i++) fc->mem[i]=forth_img[i];
	fc->cell=sizeof(size_t);
	fc->PC=0;
	fc->stop=0;
	fc->SP=128; // pre-init stacks
	fc->RP=256; //
	forthThread=kthread_run(forth_vm_main_loop,fc,"Kforth");
//	forth_vm_main_loop(fc);
	return fc;
}

void forth_done(forth_context_type *fc)
{
	if (fc==NULL) return;
	fc->stop=1;
	kthread_stop(forthThread);
	kfree(fc->mem);
	kfree(fc->out.buf);
	kfree(fc->in.buf);
	kfree(fc);
}
