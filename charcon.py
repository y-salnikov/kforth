#!/usr/bin/python2
# -*- encoding=UTF-8 -*-

import termios, sys, os, select, Queue, time

default_device="/dev/kforth"

def main():
	fd = sys.stdin.fileno()
	dev= os.open(default_device,os.O_RDWR)
	old = termios.tcgetattr(fd)
	new = termios.tcgetattr(fd)
	new[3]=new[3] & ~termios.ICANON
	is_readable=[dev,sys.stdin]
	is_writable=[dev,sys.stdout]
	is_error=[]
	inbuf=Queue.Queue()
	outbuf=Queue.Queue()
	try:
		termios.tcsetattr(fd, termios.TCSADRAIN, new)
		while 1:
			r,w,e = select.select(is_readable,is_writable,is_error)
			if sys.stdin in r: inbuf.put_nowait(sys.stdin.read(1))
			if dev in r:	   outbuf.put_nowait(os.read(dev,1))
			if ((sys.stdout in w) and ( not outbuf.empty() )): sys.stdout.write(outbuf.get_nowait())
			if (( dev in w )      and ( not inbuf.empty() ) ): os.write(dev,inbuf.get_nowait())
			time.sleep(0.0001)
	finally:
		termios.tcsetattr(fd, termios.TCSADRAIN, old)
		

main()
