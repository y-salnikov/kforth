
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


forth_context_type* forth_init(void);

void put_to_in(forth_context_type *fc, char c);
char read_from_out(forth_context_type *fc);
void forth_done(forth_context_type *fc);
