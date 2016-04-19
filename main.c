#include <stdio.h>
#include "forth.h"

int main(void)
{
	forth_context_type* fc;
	fc=forth_init();
	if(!fc) fprintf(stderr,"kforth init  error\n");
	return 0;
}
