#!/usr/bin/python2
# -*- coding: utf-8 -*-

img=bytearray()
stack=[]

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
		"(stop)":		['_'],	# ---------------------------------------
		"(over)":		["sDtsf"],
		"(2dup)":		["DtsDts"],
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

