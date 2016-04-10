compiler:= gcc
cflags:= -O2 -pipe -Wall
includes:= -I ./
libdir:= -L ./
link:= -lm 
target:= kforth

source_files:=$(wildcard *.c)
object_files:=$(patsubst %.c,%.o,$(source_files))


$(target): $(object_files)
	$(compiler) $(cflags) $^ $(libdir) $(link) -o $@

%.o: %.c
	$(compiler) $(cflags) $(includes)  -c $<
	
clean: 
	rm -f $(target)
	rm -f $(object_files)



