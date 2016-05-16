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
		"(i+)":			["ffDtst"],
		"(j)":			["fffDtstst"],
		"(<<)":			[chr(5)],
		"(>>)":			[chr(6)],
		"(xor)":		['^'],
		"(c@)":			[chr(8)],
		"(c!)":			[chr(9)],
		"(dummy)":		["b", lambda: add_cell(0)],
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
		img.append(len(value) & 0xff)
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
	for i in xrange(10): add_cell(-1)
	current0=headless_var("current0")
	context0=headless_var("context0")
	current=headless_var("current",current0.adr)
	context=headless_var("context",context0.adr)
	dp=headless_var("dp",len(img)+cell)
	add_const("bl",32,0)
	add_const("current",current.adr)
	add_const("context",context.adr)
	add_const("current0",current0.adr)
	add_const("context0",context0.adr)
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
	add_primitive("i+"   ,0,    "(i+)"    )
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
	add_var("case_sensitive",0)
	add_var("state",0)
	
	add_word("c,",0," here c! here 1 + dp !")
	add_word(",",0,"  here ! here cell + dp !")
	add_word("depth", 0, "SP@ sp0 @ swap - cell /" )
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
	add_word("n>link",0," 1 + dup c@ + 2 + ")
	add_word("c_lower",0, " dup 64 > >r dup 91 < r> and (if) 32 + (then) ")
	add_word("c=",0,"case_sensitive @ 	0 = (if)  												\
											c_lower swap c_lower 								\
										(then) =")
#	(word_adr NFA -- 0 | 1 | 2)
	add_word("(compare)",0," dup c@ >r over c@ over 1 + c@ = 									\
						  (if) 																\
								dup 1 + c@ 0 (do)	2dup 2 + i + c@ swap 1 + i + c@ c= 0 = 	\
								(if)														\
									drop -1 (leave)											\
								(then)		(loop)											\
								-1 = (if) r> drop drop 0 (else) drop 1 r> + (then)			\
						  (else)															\
								r> drop	 drop drop 0 										\
						  (then)															\
							")
#	( adr latest -- adr nfa flags+1 | 0)
	add_word("(find)",0,"	(begin)							\
								2dup (compare) dup 0 =			\
									(if)					\
										drop n>link @		\
										dup 0 = (if)		\
													0 1		\
												(else) 0	\
												(then)		\
									(else)					\
										1					\
									(then)					\
							(until)							\
							")
#	( -- (CFA 1+flags) | (0))
	add_word("-find",0,"  bl word here context @ @ 							\
							(find)											\
							dup 0 = (if)									\
										drop drop current @ @				\
										(find)								\
										dup 0 = (if)						\
													drop drop drop 0		\
												(else)						\
													>r >r drop r> 			\
													n>link cell + r>		\
												(then)					 	\
									(else)									\
										>r >r drop r> n>link cell + r>		\
									(then)\
							")
	interpret_stub_cfa=add_word("_interpret_",0,"(dummy)")
	add_var("msgs","MSG#  0: Unrecognized word.     MSG#  1: Empty Stack.           MSG#  2: Stack overflow.        MSG#  3:                        MSG#  4: Word redefined.        MSG#  5:                        MSG#  6:                        MSG#  7:                        MSG#  8:                        MSG#  9:                        MSG# 10:                        MSG# 11:                        MSG# 12:                        MSG# 13:                        MSG# 14:                        MSG# 15:                        MSG# 16:                        MSG# 17: Compilation only.      MSG# 18: Execution only.        MSG# 19: Unpaired operators.    MSG# 20: Unfinished definition. MSG# 21:                        MSG# 22:                        MSG# 23:                        MSG# 24:                        MSG# 25:                        MSG# 26: Divided by zero.       ")
	add_var("warning",1)
	add_var("erb",0)
	
	add_word("message",0," 32 * msgs 1 + + warning @ (if) 32 (else) 7 (then) type cr")
	
	add_var("ok_msg"," OK")
	add_word("quit",0," 0 state ! \
							(begin) \
								rp0 @ RP! query _interpret_ \
								state @ 0 = (if) ok_msg count type cr (then) \
								0 (until)")
	add_word("abort",0," sp0 @ SP! 10 base ! context0 context ! current0 current ! quit ")
	
	add_word("error",0," here count type bl emit 63 emit bl emit 	\
						 erb @ (if) 0 erb ! drop 					\
						 (else) message sp0 @ SP! quit (then)			")
	add_var("csp",0)
	add_word("!csp",0,"SP@ csp !")
	add_word("?csp",0,"SP@ csp @ - (if) 20 error (then)")
	add_word("?pair",0," - (if) 19 error (then)")
	add_word("?comp",0," state @ 0 = (if) 17 error (then)")
	add_word("?exec",0," state @  (if) 18 error (then)")
	add_word("?stack",0," SP@ sp0 @ > (if) 1 error (else) SP@ cell < (if) 2 error (then) (then) ")
	add_word("pow",0," 	dup 0 = (if) drop drop 1 (ret) (then)\
						dup 1 = (if) drop (ret) (then)\
						swap >r 1 swap 0 (do) j * (loop) r> drop")
	add_var("number_res",0)
	add_word("number",0,"0 number_res ! 													\
						 count >r dup c@ 45 = (if) 1 + 1 r> 1 - >r  (else) 0 (then) swap 	\
	                     dup c@ 48 = 								\
	                     (if) 													\
							i 1 = (if) r> drop drop drop 0 (ret)								\
							(else)	\
								dup 1 + c@ 88 = >r dup 1 + c@ 120 = r> or 					\
								(if)	\
									2 + r> 2 - >r 16	\
								(else)	\
									1 + r> 1 - >r 8	\
								(then)	\
							(then)		\
						 (else)		\
							base @	\
	                     (then) \
	                     r> swap >r \
	                     0  \
	                     (do)			\
							dup i+ + i - 1 - c@	\
							48 - dup 9 > (if) 7 - (then)	\
							dup 32 > (if) 32 - (then)		\
							dup j 1 - > (if) 0 error (then)	\
							j i pow *  \
							number_res @ \
								+\
							number_res ! \
	                     (loop)	drop		\
	                     r> drop number_res @ swap (if) -1 xor 1 + (then)\
	                     ")
	add_word("execute",0," 1 + >r")
	add_word("cfa,",0," count over + swap (do) i c@ c, (loop)")
	add_word("'",0," -find (if) (ret) (then) 0 error")
	add_word("[']",1," ' 108 c, ,")  # 108 = 'l' = lit
	add_word("[compile]",1," ' cfa, ")
	add_word("literal",0," state @ (if) 108 c, , (then)")
	interpret_cfa=add_word("interpret",0,"	(begin)\
									-find dup (if) \
										1 - 0 > state @ 0 = or \
										(if)\
											execute	\
										(else)\
											cfa, \
										(then)\
									(else)\
										drop here number \
										literal \
									(then)\
									?stack \
								0 (until)")
	add_word(chr(0),1,"r> drop r> drop" )
	write_cell(interpret_stub_cfa+3+cell+1,interpret_cfa+3+cell)
	


	cfa=add_word("init",0,"current @ context !  quit" )
	init_code_compile(sp0_val,rp0_val,cfa+1) # cfa+1 of init word
	output_to_h(img)
	
main()


