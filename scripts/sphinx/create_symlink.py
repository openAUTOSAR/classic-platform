#!/usr/bin/python

import argparse
#import os
import os
import shutil

	
if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument('pairs', metavar='N', nargs='+',
                    help='an integer for the accumulator')
	
	args = parser.parse_args()
	
	for key,value in vars(args).iteritems():
		for val in value:
			dst,src=val.split(",")
			print src
			print dst
			shutil.copyfile(src, dst)
			#os.symlink(str(src), str(dst))
