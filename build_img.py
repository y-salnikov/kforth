#!/usr/bin/python2
# -*- coding: utf-8 -*-

cell=8 # for 64bit
stack_depth=256 #cells


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


pvoc={	"0":		 	['0'],
		"1": 			['1'],
		"2": 			['2'],
		"3": 			['3'],
		"4": 			['4'],
		"5": 			['5'],
		"6": 			['6'],
		"7": 			['7'],
		"8": 			['8'],
		"9": 			['9'],
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
		"(2dup)":		["DtsDts"],
		"(if)":			["?", lambda: stack.append(len(img)), lambda:stack.append(2),lambda: add_cell(0)],
		"(then)":		[lambda: pair(2), lambda: write_cell(stack.pop(),len(img))],
		"(else)":		[lambda: pair(2),"b", lambda: add_cell(0), lambda: write_cell(stack.pop(),len(img)), lambda: stack.append(len(img)-cell), lambda:stack.append(2)],

		}


def compile_word(word):
	if word in pvoc.keys():
		actions=pvoc[word]
		for act in actions:
			if hasattr(act, "__call__"):        # execute
				act()
			else:								# compile
				if type(act) is str:
					for s in act: img.append(s)

def compile_str(s):
	for w in s.split(' '):
		w=w.strip()
		if len(w)>0:
			compile_word(w)

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
	pvoc[name]=img[code_adr:code_adr+code_length]

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
	add_cell(value)
	pvoc[name]=img[code_adr:code_adr+code_length]

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
	
	init_code_compile(sp0_val,rp0_val,0) # 0 - cfa+1 of init word
	print pvoc.keys()
main()


