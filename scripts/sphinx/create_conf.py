
import argparse

app = []
sec = []
bla = []

name_map =  [
  ("um","User Manual"),
  ("dd","Design Description"),
  ("sm","Safety Manual")
  ]

userm = ""
	
if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument("-a", "--arch", dest = "arch", help="arc")
	parser.add_argument("-t", "--type", dest = "type", help="type")
	parser.add_argument("-m", "--module", dest = "module", help="module")
	
	args = parser.parse_args()
	
	for x in name_map:
		if ( x[0] == args.type ): 
			userm = x[1]
			
	if (userm != ""):
		module = args.module.upper()
		doc_name = "%s %s" % (module,userm) 
				
		if (args.arch != None ):
			doc_name = "%s for %s" %(doc_name, args.arch.upper())
			latex_name="%s_%s_%s.tex" % (args.module,args.type,args.arch)
		else:
			latex_name="%s_%s.tex" % (args.module,args.type)	
			
		
		print "project = u\'%s'" % (doc_name)
		print ""
		print "latex_documents = [(master_doc, u\'%s\', u\'%s\',u\'Arccore\', \'manual\'),]" % (latex_name, doc_name) 
		print ""
		
		if (args.arch != None ):
			print "rst_epilog = \"\"\"\n.. |mcu| replace:: %s\n\"\"\"" % (args.arch)
		