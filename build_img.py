#!/usr/bin/python2
# -*- coding: utf-8 -*-

cell=8 			# for 64bit - 8 or 4 for 32bit
stack_depth=256 # cells
tib_size=1024   # text buffer for 1 input line

IMMEDIATE=1

img=bytearray()
stack=[]

def pair(n):
	if n!=stack.pop():
		print("unpaired operator")
		exit(1)

def add_cell(val):
	tmp=val
	for i in xrange(cell):
		img.append(tmp>>(i*8) & 0xff)

def write_cell(adr,val):
	tmp=val
	for i in xrange(cell):
		img[adr+i]=(tmp>>(i*8) & 0xff)

class headless_var():
	var_adr={}
	def __init__(self,name,value=0):
		self.adr=len(img)
		self.name=name
		headless_var.var_adr[name]=self.adr
		add_cell(value)


pvoc={	"(0)":		 	['0'],
		"(1)": 			['1'],
		"(2)": 			['2'],
		"(3)": 			['3'],
		"(4)": 			['4'],
		"(5)": 			['5'],
		"(6)": 			['6'],
		"(7)": 			['7'],
		"(8)": 			['8'],
		"(9)": 			['9'],
		"(@)":			['@'],
		"(!)":			['!'],
		"(drop)":		['d'],
		"(dup)":		['D'],
		"(swap)":		['s'],
		"(lit)":		['l'],
		"(+)":			['+'],
		"(-)":			['-'],
		"(*)":			['*'],
		"(/)":			['/'],
		"(mod)":		['%'],
		"(and)":		['&'],
		"(or)":			['|'],
		"(>)":			['>'],
		"(<)":			['<'],
		"(=)":			['='],
		"(branch)":		['b'],
		"(?branch)":	['?'],
		"(cal)":		['c'],
		"(ret)":		['r'],
		"(>r)":			['t'],
		"(r>)":			['f'],
		"(key)":		['i'],
		"(emit)":		['o'],
		"(SP@)":		[chr(1)],
		"(SP!)":		[chr(2)],
		"(RP@)":		[chr(3)],
		"(RP!)":		[chr(4)],
		"(stop)":		['_'],	# ---------------------------------------
		"(over)":		["sDtsf"],
		"(2dup)":		["DtsDtsff"],
		"(if)":			["?", 							lambda: stack.append(len(img)), lambda:stack.append(2),						lambda: add_cell(0)],
		"(then)":		[lambda: pair(2), 				lambda: write_cell(stack.pop(),	len(img))],
		"(else)":		[lambda: pair(2),"b", 			lambda: add_cell(0), 			lambda: write_cell(stack.pop(),len(img)), 	lambda: stack.append(len(img)-cell), lambda:stack.append(2)],
		"(begin)":		[lambda: stack.append(len(img)),lambda: stack.append(1)],
		"(until)":		[lambda: pair(1),				"?",							lambda:add_cell(stack.pop())],
		"(do)":			["stt",							lambda:stack.append(len(img)), 	lambda:stack.append(3)],
		"(loop)":		[lambda: pair(3),				"f1+DfDtsf=stst?",				lambda:add_cell(stack.pop()), "ffdd"],
		"(leave)":		["fdfDt1-t"],
		"(i)":			[chr(7)],
		"(j)":			["fffDtstst"],
		"(<<)":			[chr(5)],
		"(>>)":			[chr(6)],
		"(xor)":		['^'],
		"(c@)":			[chr(8)],
		"(c!)":			[chr(9)],
		}


def compile_word(word):
	offset=len(img)
	if word in pvoc.keys():
		actions=pvoc[word]
		for act in actions:
			if hasattr(act, "__call__"):        # execute
				act()
			else:								# compile
				if type(act) is str:
					for s in act: img.append(s)
	else:
		if (word.isdigit()) or ((word[0]=='-') and (word[1:].isdigit())):
			img.append('l')
			add_cell(int(word))
		else: print "unrecognized word: %s" %(word)
	return len(img)-offset

def compile_str(s):
	length=0;
	s=s.replace(chr(9),' ')
	for w in s.split(' '):
		w=w.strip()
		if len(w)>0:
			length+=compile_word(w)
	return length

def init_code_compile(sp0,rp0,word):
	adr=0;
	img[adr]='l'
	adr+=1
	write_cell(adr,sp0)
	adr+=cell
	img[adr]=2 #sp!
	adr+=1
	img[adr]='l'
	adr+=1
	write_cell(adr,rp0)
	adr+=cell
	img[adr]=4 #sp!
	adr+=1
	img[adr]='c'
	adr+=1
	write_cell(adr,word)
	adr+=cell
	img[adr]='_'
	adr+=1

# memory map
# 0 init code
# stack
# r_stack
# dictionary
# ...

def read_cell(adr):
	val=0
	for i in xrange(cell):
		val|=(img[adr+i]<<(8*i))
	return val


def add_header(name,flags,link=None):
	NFA=len(img)
	img.append(flags)
	img.append(len(name))
	for s in name: img.append(s)
	img.append(len(name))
	if link==None:
		link=read_cell(read_cell(current.adr))
	add_cell(link)
	write_cell(read_cell(current.adr),NFA)
	write_cell(dp.adr,len(img))


def add_const(name,value,link=None):
	add_header(name,0,link)
	code_length=cell+2
	img.append(code_length)
	code_adr=len(img)
	img.append('l')
	adr=len(img)
	add_cell(0)
	img.append('@')
	img.append('r')
	write_cell(adr,len(img))
	add_cell(value)
	pvoc[name]=[str(img[code_adr:code_adr+code_length])]
	write_cell(dp.adr,len(img))

def add_var(name,value,link=None):
	add_header(name,0,link)
	code_length=cell+1
	img.append(code_length)
	code_adr=len(img)
	img.append('l')
	adr=len(img)
	add_cell(0)
	img.append('r')
	write_cell(adr,len(img))
	if type(value) is str:
		img.append(len(value))
		for i in value: img.append(i)
		img.append(0)
	else: add_cell(value)
	pvoc[name]=[str(img[code_adr:code_adr+code_length])]
	write_cell(dp.adr,len(img))

def add_array(name,length,link=None):
	add_header(name,0,link)
	code_length=cell+1
	img.append(code_length)
	code_adr=len(img)
	img.append('l')
	adr=len(img)
	add_cell(0)
	img.append('r')
	write_cell(adr,len(img))
	for i in xrange(length): img.append(0)
	pvoc[name]=[str(img[code_adr:code_adr+code_length])]
	write_cell(dp.adr,len(img))


def add_primitive(name,flags,string):
	add_header(name,flags)
	code_length_adr=len(img)
	img.append(0)
	code_length=compile_str(string)
	if code_length>255:
		print "Primitive code is too long."
		exit(1)
	img[code_length_adr]=code_length;
	img.append('r')
	pvoc[name]=[str(img[code_length_adr+1:code_length_adr+1+code_length])]
	write_cell(dp.adr,len(img))
	return code_length_adr

def add_word(name,flags,string):
	add_header(name,flags)
	code_length=cell+1
	img.append(code_length)
	code_adr=len(img)
	img.append('c')
	add_cell(len(img)+cell+1)
	img.append('r')
	compile_str(string)
	img.append('r')
	pvoc[name]=[str(img[code_adr:code_adr+code_length])]
	write_cell(dp.adr,len(img))
	return code_adr-1


def output_to_h(arr):
	f=open("forth_img.h","w")
	f.write("//forth image\n")
	f.write("char forth_img[]={ \n")
	s=""
	for i in xrange(len(arr)):
		if (arr[i]>31) and (arr[i]<ord('z')) and (arr[i] not in [34,39,92]): s+=" '%c'," %(arr[i])
		else: 	s+="0x%02X," %(arr[i])
		if (i%16)==15: s+="\n"
		else: s+=" "
	f.write(s[:-2])
	f.write("\n};\n")
	f.write("size_t forth_img_length=%d;\n" %(len(arr)))
	f.close()

def main():
	global sp0
	global rp0
	global current
	global context
	global dp
	for i in xrange(stack_depth): add_cell(0)
	sp0_val=len(img)
	for i in xrange(stack_depth): add_cell(0)
	rp0_val=len(img)
	sp0=headless_var("sp0",sp0_val)
	rp0=headless_var("rp0",rp0_val)
	current0=headless_var("current0")
	context0=headless_var("context0")
	current=headless_var("current",current0.adr)
	context=headless_var("context",context0.adr)
	dp=headless_var("dp",len(img)+cell)
	add_const("bl",32,0)
	add_const("current",current.adr)
	add_const("context",context.adr)
	add_const("dp",dp.adr)
	add_const("sp0",sp0.adr)
	add_const("rp0",rp0.adr)
	add_primitive("here",0,"dp (@)")
	add_const("cell",cell)
	add_var("base",10)
	add_primitive("0",0,"(0)")
	add_primitive("1",0,"(1)")
	add_primitive("2",0,"(2)")
	add_primitive("3",0,"(3)")
	add_primitive("4",0,"(4)")
	add_primitive("5",0,"(5)")
	add_primitive("6",0,"(6)")
	add_primitive("7",0,"(7)")
	add_primitive("8",0,"(8)")
	add_primitive("9",0,"(9)")
	add_primitive("@"   ,0,    "(@)"    )
	add_primitive("!"	  ,0,    "(!)"	)
	add_primitive("drop",0,    "(drop)" )
	add_primitive("dup" ,0,    "(dup)"  )
	add_primitive("swap",0,    "(swap)" )
	add_primitive("lit" ,0,    "(lit)"  )
	add_primitive("+"	  ,0,    "(+)"	)
	add_primitive("-"	  ,0,    "(-)"	)
	add_primitive("*"   ,0,    "(*)"    )
	add_primitive("/"   ,0,    "(/)"    )
	add_primitive("mod" ,0,    "(mod)"  )
	add_primitive("and" ,0,    "(and)"  )
	add_primitive("or"  ,0,    "(or)"   )
	add_primitive("xor"  ,0,    "(xor)"   )
	add_primitive(">"   ,0,    "(>)"    )
	add_primitive("<"   ,0,    "(<)"    )
	add_primitive("="   ,0,    "(=)"    )
	add_primitive(">r"  ,0,    "(>r)"   )
	add_primitive("r>"  ,0,    "(r>)"   )
	add_primitive("key" ,0,    "(key)"  )
	add_primitive("emit",0,    "(emit)" )
	add_primitive("SP@" ,0,    "(SP@)"  )
	add_primitive("SP!" ,0,    "(SP!)"  )
	add_primitive("RP@" ,0,    "(RP@)"  )
	add_primitive("RP!" ,0,    "(RP!)"  )
	add_primitive("over",0,    "(over)" )
	add_primitive("2dup",0,    "(2dup)" )
	add_primitive("i"   ,0,    "(i)"    )
	add_primitive("j"   ,0,    "(j)"    )
	add_primitive("<<",0,		"(<<)")
	add_primitive(">>",0,		"(>>)")
	add_primitive("cr",0,		"10 emit")
	add_primitive("c@",0,		"(c@)")
	add_primitive("c!",0,		"(c!)")
	
	add_array("tib",tib_size)
	img.append(0)
	img.append(" ")
	img.append(0)
	add_const("tib_size",tib_size)
	add_var(">in",0)
	add_var("span",0)

	add_word("c,",0," here c! here 1 + dp !")
	add_word(",",0,"  here ! here cell + dp !")
	add_word("depth", 0, "sp0 @ SP@ - cell /" )
	add_word("u.",0,	" 0 swap (begin) dup base @ mod dup 9 > (if) 7 + (then) 48 + swap base @ / dup 0 = 	(until)	drop (begin) emit dup 0 = (until) drop ")
	add_word("x.",0,	" 48 emit 120 emit 16 base ! u. 10 base ! ")
	add_word(".",0,		"	dup 1 cell 8 * 1 - << and (if) -1 xor  1 + 45 emit (then) u. ")
	add_word("type",0,	" dup 0 > (if) over + swap (do) i c@ emit (loop) (then)")
	add_word("count",0,	" dup 1 + swap c@")
	add_word("expect",0," 0 span ! >r dup r> over + swap (do)								\
													key dup 10 = >r 						\
														dup 13 = >r 						\
														dup 0  = r> r> or or 				\
														(if)  								\
															 drop (leave)					\
														(else) 								\
															i c! span @ 1 + span !			\
														(then) 								\
												(loop) 										\
												span @ + >r 32 i c! 0 i 1 + c! 32 r> 2 + c! ")

	add_word("trailing",0," (begin) dup tib >in @ + c@ = (if) >in @ 1 + >in ! 0 (else) 1 (then) (until) drop")
	
	add_word("word",0,	" dup trailing 																				\
						   here 0 c, swap																			\
						  (begin) dup tib >in @ + c@  = 		(if) 												\
																		drop dup dup 1 + here swap - swap c! dp ! 1 \
																(else)  											\
																		tib >in @ dup 1 + >in !  + c@ c, 0 			\
																													\
																(then)   (until) ")
	add_word("query",0, "tib tib_size expect 0 >in !")
	
	add_var("msg","g-gurdissimo")
	cfa=add_word("test",0,"									\
							10 0 (do)						\
								i 5 - . cr					\
								i 8 = (if) (leave)  (then) 	\
						  (loop)							\
						 depth . cr							\
						 query bl word here count type cr \
						 depth . cr							\
						  ")

	init_code_compile(sp0_val,rp0_val,cfa+1) # cfa+1 of init word
	output_to_h(img)
	
main()


