
typedef struct forth_context_struct
{
	unsigned char *mem;
	char cell;
	size_t mem_size;
	size_t SP;
	size_t RP;
	size_t PC;
	size_t stop;
	struct circ_buf in;
	struct circ_buf out;
}forth_context_type;


#define MEM_SIZE 1024*1024*1 //1M initial memory size
#define TIB_SIZE	1024    // max line length. Must be power of 2 for ring buffers

#define KARGS	16			// maximum arguments of kallsyms

forth_context_type* forth_init(void);

void put_to_in(forth_context_type *fc, char c);
char read_from_out(forth_context_type *fc);
void forth_done(forth_context_type *fc);

typedef size_t (*kall0)(void);
typedef size_t (*kall1)(size_t);
typedef size_t (*kall2)(size_t,size_t);
typedef size_t (*kall3)(size_t,size_t,size_t);
typedef size_t (*kall4)(size_t,size_t,size_t,size_t);
typedef size_t (*kall5)(size_t,size_t,size_t,size_t,size_t);
typedef size_t (*kall6)(size_t,size_t,size_t,size_t,size_t,size_t);
typedef size_t (*kall7)(size_t,size_t,size_t,size_t,size_t,size_t,size_t);
typedef size_t (*kall8)(size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t);
typedef size_t (*kall9)(size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t);
typedef size_t (*kall10)(size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t);
typedef size_t (*kall11)(size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t);
typedef size_t (*kall12)(size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t);
typedef size_t (*kall13)(size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t);
typedef size_t (*kall14)(size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t);
typedef size_t (*kall15)(size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t);
typedef size_t (*kall16)(size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t);

