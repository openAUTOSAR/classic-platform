# -*- coding: utf-8 -*

# Convert a .rst to a 


import argparse
import re

if __name__ == "__main__":
    state = "toc" 

    parser = argparse.ArgumentParser()

    parser.add_argument("file")
    #parser.add_argument("-a", "--arch", dest = "arch", help="arc")
    #parser.add_argument("-t", "--type", dest = "type", help="type")
    #parser.add_argument("-m", "--module", dest = "module", help="module")

    args = parser.parse_args()

    file = open(args.file, "r") 
    for line in file:
        
    	#print line,
    	
        if ( state == "toc" ):

            # Grab some useful information and then remove it..
            # +------------------------------------+---------------------------------+-----------------------------------------------+
            # | **Document No: **\ 14-05-813-16010 | **Responsible**: Mårten Hildell | **Approver**: LakshmiNarayanan Chandrasekaran |
            # +------------------------------------+---------------------------------+-----------------------------------------------+
            # 
            # Extract

            #m = re.search('\*\*Document No: \*\* (.*)\|',line)
            m = re.search('Document No:.*(14.*?)\|',line)
            if m is not None:
                print "Yeeeh"
                print m.group(1)
                print "ASASDASD"
                print m.group(0)
                #print m.group(1)
            
            m = re.search('.. rubric:: Document References',line)
            if m is not None:
                state = "body"

        else:
           	# First remove lines we don't want            
            q = re.search('..raw.*', line )
            if q != None:
                continue

            q = re.sub('.. code:: syntaxhighlighter-pre', '.. code-block:: c', line )
            q = re.sub('<.*>', '', q )
            q = re.sub('^-','*', q )
            print q,
            
        #print line,
    
        """

        #elif ( state == "body" ):
        #    print line,



        #print line,
		"""



