#!/usr/bin/env python
# -*- coding: latin-1 -*-

###
### Copyright 2017- ArcCore GmbH. All rights reserved.
###

"""ARCCORE MISRA Deviation Reporter
   Analyser

   Parses the found lint message for proper information
   In ideal world the following shall be reported:
   File,Line,MISRA Rule,Level,Category,Justification
   Apa.c,23, 2004 Rule 11.1,required,HARDWARE_ACCESS,N/A

   returns the the values defined as above
   returns boolean value, if it has to be reported or not
   only MISRA 2012 Rules/Directive shall be reported
   
   extractReportingInformation - Parses the found lint message for proper information
"""

__author__ = "ARCCORE"
__date__ = "06 May 2017"

__version__ = "$Revision: 2017-05-07 $"
__credits__ = """Copyright 2017 Arccore GmbH. All rights reserved.
"""

#----------------------------------------------------------

# Known bugs that can't be fixed here:
# N/A

#----------------------------------------------------------

import re
import mdr_globals

#----------------------------------------------------------
                
def extractReportingInformation(line, lintMisraRule, lintMisraType):
    """
    Parses the found lint message for proper information
        
    line - content of the line to be analysed
    lintMisraRule - Dictionary mapping PC-Lint numbers to MISRA rules/directives numbers
    lintMisraType - Dictionary mapping PC-Lint numbers to MISRA rules/directives categories

    returns [Lint number, Misra,              Compliance level, Category,         Justification ]
    e.g.    [950,         2012:Directive:2.1, required        , "FALSE_POSITIVE", "nice comment"]

    return [report, lin, mis, lev, cat, jus]
    report - True, if it has to be reported, False otherwise, only MISRA 2012 Rules/Directive shall be reported
    lin - Lint number
    mis - MISRA rule/directive
    lev - Compliance level
    cat - Category for the deviation
    just - Justification comment
    """

    lin = []
    mis = ""
    lev = ""
    cat = ""
    jus = ""
    report = False

    # Try to extract lint numbers
    if mdr_globals.DEBUG: print "ANALYSE:70 ", line
    #remove the right part of a comment */
    line = line.split("*/")[0]
    if mdr_globals.DEBUG: print "ANALYSE:73 ", line

    #remove the left part of a comment /*lint
    splitLine = re.split("lint", line)
    if len(splitLine) < 2:
        return [report, lin, mis, lev, cat, jus]

    if mdr_globals.DEBUG: print "ANALYSE:80 ", splitLine
    #remove right part of text after Lint number, if correct used starts with MISRA
    test = splitLine[1].split("MISRA")
    if mdr_globals.DEBUG: print "ANALYSE:83 ",test

    #ty to get all numbers out
    list = re.findall("[0-9].*,*[0-9]",test[0])
    if mdr_globals.DEBUG: print "ANALYSE:87 ", list

    countDigits = 0
    for elem in list:
        check = elem.replace(' ','')
        if check.isdigit():
            countDigits = countDigits + 1

    if mdr_globals.DEBUG: print "ANALYSE:95 ", countDigits
				
    # Did not work for that example: /*lint -e553  STANDARD DEV SELECT. Undefined preprocessor variable, assumed 0			
    if countDigits == 0:
	list = re.findall("[0-9]*",test[0])
	if mdr_globals.DEBUG: print "ANALYSE:100 ",list
        for elem in list:
            check = elem.replace(' ','')
	    if check.isdigit():
		list=[]
		list.append(elem)
		break;
					
    if (len(list) == 0):
        return [report, lin, mis, lev, cat, jus]

    #split them up
    test = list[0].split(",")

    #lint without number, e.g. /*lint -restore */
    if mdr_globals.DEBUG: print "ANALYSE:109 ",test
    if len(test) == 0:
        return [report, lin, mis, lev, cat, jus]

    #list = re.findall("[0-9]*",test[0])
    #if mdr_globals.DEBUG: print "Check here:", list

    list = test

    for elem in list:
        check = elem.replace(' ','')
        if check.isdigit():
            lin.append(int(check))

    if mdr_globals.DEBUG: print "ANALYSE:123 ", list

    for elem in  mdr_globals.EXECPTIONS_CATEGORIES:
        if re.findall(elem, line):
            if cat == "":
                cat = cat + elem
            else:
                cat = cat + " : " + elem

    if cat == "":
        cat = "WARNING: Nothing found"

	jus = "not supported yet, see comment"
	
    for elem in lin:
        if lintMisraRule.has_key(elem):
            if mis == "":
                mis = lintMisraRule[elem]
                lev = lintMisraType[elem]
                jus = "N/A"
            else:
                mis = mis + " : " + lintMisraRule[elem]
                lev = lev + " : " + lintMisraType[elem]
                jus = jus + " : " + "N/A"
        else:
            if mis == "":
                mis = "N/A in MISRA 2012"
                lev = "N/A"
                jus = "seems to be MISRA 2004"
            else:
                mis = mis + " : " + "N/A in MISRA 2012"
                lev = lev + " : " + "N/A"
                jus = jus + " : " + "seems to be MISRA 2004"
				
	report = True
 
    if mdr_globals.DEBUG:print "(ANALYSE: 159)====================================="
    if mdr_globals.DEBUG:print "Report:\t\t", report
    if mdr_globals.DEBUG:print "PC_Lint:\t\t", lin
    if mdr_globals.DEBUG:print "MISRA:\t\t\t", mis
    if mdr_globals.DEBUG:print "Level:\t\t\t", lev
    if mdr_globals.DEBUG:print "Category:\t\t", cat
    if mdr_globals.DEBUG:print "Justification:\t\t", jus
    if mdr_globals.DEBUG:print "(ANALYSE: 166)====================================="
    
    return [report, lin, mis, lev, cat, jus]
