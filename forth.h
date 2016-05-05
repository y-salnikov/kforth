
typedef struct forth_context_struct
{
	unsigned char *mem;
	char cell;
	size_t mem_size;
	size_t SP;
	size_t RP;
	size_t PC;
	size_t stop;

}forth_context_type;


#define MEM_SIZE 1024*1024*1 //1M initial memory size
#define TIB_SIZE	1024    // max line length
#define IMMEDIATE	1

forth_context_type* forth_init(void);


