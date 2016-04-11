
typedef struct forth_context_struct
{
	char *mem;
	char cell;
	size_t mem_size;
	size_t *SP;
	size_t *RP;
	char *begin;
	size_t *here_ptr;
	size_t *latest_ptr;
	size_t *pc;
	char	stop;
	
}forth_context_type;


#define MEM_SIZE 1024*1024*1 //1M initial memory size
#define STACK_DEPTH 256		// in cells
#define PAD_SIZE	1024

forth_context_type* forth_init(void);


