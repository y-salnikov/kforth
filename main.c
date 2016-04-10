#include <stdio.h>
#include "forth.h"

int main(void)
{
	char *here;
	forth_context_type* fc;
	fc=forth_init();
	here=(char*)*fc->here_ptr;
	printf("%ld\n",here-fc->begin);
	add_byte(fc,1);
	here=(char*)*fc->here_ptr;
	printf("%ld\n",here-fc->begin);
	add_cell(fc,2);
	here=(char*)*fc->here_ptr;
	printf("%ld\n",here-fc->begin);
	align(fc);
	here=(char*)*fc->here_ptr;
	printf("%ld\n",here-fc->begin);
	return 0;
}
