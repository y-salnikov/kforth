#!/usr/bin/python2
# -*- coding: utf-8 -*-

cell=8 # for 64bit


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
		"(else)":		[lambda: pair(2),"b", lambda: add_cell(0), lambda: write_cell(stack.pop(),len(img)), lambda: stack.append(len(img)-cell), stack.append(2)],

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

def main():
	pass

main()


