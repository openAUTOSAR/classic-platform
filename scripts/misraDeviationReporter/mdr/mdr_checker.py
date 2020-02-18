#!/usr/bin/env python
# -*- coding: latin-1 -*-

###
### Copyright 2017- ArcCore GmbH. All rights reserved.
###

"""ARCCORE MISRA Deviation Reporter
   Checker

   Parses the found lint message for proper information

   In ideal world the following shall be found in the code:
   #MISRA : CONFIGURATION|...|OTHER : <OPTIONAL: additional comment or explanation> : [MISRA 2012 Rule|Directive X.Y, advisory|mandatory|required]
   #LINT  : CONFIGURATION|...|OTHER : <OPTIONAL: additional comment or explanation>

    returns [Error,      Report,     Lint number, Misra,              Compliance level, Category,         Justification ]
    e.g.    [True|False, True|False, 950,         2012:Directive:2.1, required        , "FALSE_POSITIVE", "nice comment"]

   returns the the values defined as above
   returns boolean value report , if it has to be reported or not
   only MISRA 2012 Rules/Directive shall be reported
   returns boolean value error, if it has correct deviation information as required by guideline
   
   extractReportingInformation - Parses the found lint message for proper information
"""

__author__ = "ARCCORE"
__date__ = "06 May 2017"

__version__ = "$Revision: 2017-05-18 $"
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

    In ideal world the following shall be found in the code:
    #MISRA : CONFIGURATION|...|OTHER : <OPTIONAL: additional comment or explanation> : [MISRA 2012 Rule|Directive X.Y, advisory|mandatory|required]
    #LINT  : CONFIGURATION|...|OTHER : <OPTIONAL: additional comment or explanation>
    
    line - content of the line to be analysed
    lintMisraRule - Dictionary mapping PC-Lint numbers to MISRA rules/directives numbers
    lintMisraType - Dictionary mapping PC-Lint numbers to MISRA rules/directives categories

    returns [Error,      Report,     Lint number, Misra,              Compliance level, Category,         Justification ]
    e.g.    [True|False, True|False, 950,         2012:Directive:2.1, required        , "FALSE_POSITIVE", "nice comment"]

    return [error, report, lin, mis, lev, cat, jus]
    error - True, if it has NOT a correct deviation information as required by guideline, False otherwise
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
    error = ""

	#Example rule
	#/*lint !e9016 MISRA:PERFORMANCE:pointer arithmetic other than array indexing used, working with pointer in a controlled way results more simplified and readable code:[MISRA 2012 Rule 18.4, advisory]*/
    #split line with delimeter ":"
    splitLineWithDelimeter = line.split(":")
    if mdr_globals.DEBUG: print "VISUALIZE:84 splitLineWithDelimeter=", splitLineWithDelimeter
    
    #check if LINT is used, in that case no report required
	#check if -restore is used, in that case no report required, end of block
    if ("-restore" in line):
        return ["", report, lin, mis, lev, cat, jus]
		
    #check if /*lint in line, if not, in that case no report required, may only MISRA statement,
	#but that line will be part of analyser report
    if ("/*lint" in line) or ("//lint" in line):
        pass
    else:
        return ["", report, lin, mis, lev, cat, jus]	

    #check if at least 4 elements are available
    if not((len(splitLineWithDelimeter) < 5) and (len(splitLineWithDelimeter) > 2)):
        return ["Misra deviation comment form not according to style guide", report, lin, mis, lev, cat, jus]
    
    #check if Exception Category is present
    for elem in  mdr_globals.EXECPTIONS_CATEGORIES:
        if re.findall(elem, line):
            if cat == "":
                cat = cat + elem
            else:
                cat = cat + " : " + elem

    if cat == "" or cat != splitLineWithDelimeter[1]:
        return ["Exception category should be CONFIGURATION,STANDARDIZED_INTERFACE,HARDWARE_ACCESS,ARGUMENT_CHECK,PERFORMANCE,FALSE_POSITIVE,EXTERNAL_FILE or OTHER. It's neither of mentioned categories and not according to style guide", report, lin, mis, lev, cat, jus]

    #check if MISRA Category is present
    for elem in  mdr_globals.MISRA_CATEGORIES:
        if re.findall(elem, line):
            if lev == "":
                lev = lev + elem
            else:
                lev = lev + " : " + elem

    if lev == "":
        return ["MISRA category should be required,advisory or mandatory. It's neither of mentioned category and not according to style guide", report, lin, mis, lev, cat, jus]
        
    #check if mandatory and only FALSE_POSITIVE is combined
    if ('mandatory' in lev):
        if ('POSITIVE' in cat):
            pass
        else:
            return ["MISRA category mandatory can only be used together with 'FALSE_POSITIVE' as justification", report, lin, mis, lev, cat, jus]
    
    #check if required and OTHER is combined
    if ('required' in lev) and ('OTHER' in cat):
        return ["MISRA category 'required' can not be used together with 'OTHER' as justification. 'required' word should not be used in explanation part of comment because even if MISRA Category is advisory or mandatory,it checks for word required", report, lin, mis, lev, cat, jus]

    #check if MISRA form is present
    for elem in  mdr_globals.MISRA_FORM:
        if re.findall(elem, line):
            if mis == "":
                mis = mis + elem
            else:
                mis = mis + " : " + elem
            
    if mis == "":
        return ["Keyword MISRA is missing in deviation comment", report, lin, mis, lev, cat, jus]

    #check is Misra rule/directive is present and category
    lev2 = ""
    try:
        splitMisraRule = splitLineWithDelimeter[3].split("]")
		#['[MISRA 2012 Rule 18.4, advisory', '*/\n']
        if mdr_globals.DEBUG: print "VISUALIZE:144 splitMisraRule=", splitMisraRule
        splitMisraRule = splitMisraRule[0].split("[")
		#['', 'MISRA 2012 Rule 18.4, advisory']
        if mdr_globals.DEBUG: print "VISUALIZE:147 splitMisraRule=", splitMisraRule
        splitMisraRule = splitMisraRule[1].split(",")
		#['MISRA 2012 Rule 18.4', ' advisory']
        if mdr_globals.DEBUG: print "VISUALIZE:150 splitMisraRule=", splitMisraRule
        mis = splitMisraRule[0].strip()
        lev2 = splitMisraRule[1].strip()
		#MISRA 2012 Rule 18.4 advisory
        if mdr_globals.DEBUG: print "VISUALIZE:154 splitMisraRule=", mis, lev2
    except:
        return ["Keyword rule or directive is missing in deviation comment", report, lin, mis, lev, cat, jus]

    if lev2 == "" or lev2 != lev:
        return ["Keyword advisory|required|mandatory is missing in deviation comment or you do not follow MISRA Devition Guide Format", report, lin, mis, lev, cat, jus]

    #assign justification
    jus = splitLineWithDelimeter[2]
	#pointer arithmetic other than array indexing used, working with pointer in a controlled way results more simplified and readable code
    if mdr_globals.DEBUG: print "VISUALIZE:164 jus=", jus
    
    # Try to extract lint numbers
    #remove the left part of a comment /*lint
    splitLineLint = re.split("lint", splitLineWithDelimeter[0])
	#......./*', ' !e9016 MISRA']
    if mdr_globals.DEBUG: print "VISUALIZE:170 splitLineLint=", splitLineLint

    if len(splitLineLint) < 2:
        return ["There seems no PC-Lint number in deviation comment", report, lin, mis, lev, cat, jus]

    #ty to get all numbers out
    splitLineLint = re.findall("[0-9].*,*[0-9]",splitLineLint[1])
	#['9016']
    if mdr_globals.DEBUG: print "VISUALIZE:178 splitLineLint=", splitLineLint

    if len(splitLineLint) == 0:
        return ["There seems no PC-Lint number in deviation comment", report, lin, mis, lev, cat, jus]

    #split them up
    splitLineLint = splitLineLint[0].split(",")
	#['9016']
    if mdr_globals.DEBUG: print "VISUALIZE:186 splitLineLint=", splitLineLint

    if len(splitLineLint) == 0:
        return ["There seems no PC-Lint number in deviation comment", lin, mis, lev, cat, jus]

    for elem in splitLineLint:
        check = elem.replace(' ','')
        if check.isdigit():
            lin.append(int(check))
	#[9016]
	if mdr_globals.DEBUG: print "VISUALIZE:196 lin=", lin

    report = False
    report2 = True
    for elem in lin:
        if lintMisraRule.has_key(elem):
		    #2012:Rule:18.4
            if mdr_globals.DEBUG:print "VISUALIZE:203 lintMisraRule: ", lintMisraRule[elem]
            check = lintMisraRule[elem].split(",")
            for elem3 in check:
                check2 = elem3.split(":")
		#['2012', 'Rule', '18.4']
                if mdr_globals.DEBUG:print "VISUALIZE:208 check2: ", check2
                report2 = True
                for elem2 in check2:
		    #check2: elem2 2012, elem2 Rule, elem2 18.4
                    if mdr_globals.DEBUG:print "VISUALIZE:212 check2 elem2", elem2
                    if elem2 not in mis:
			#never reached for current example
                        if mdr_globals.DEBUG:print "VISUALIZE:215 check2 mis:", elem2, mis
                        report2 = False

                if lev not in lintMisraType[elem]:
		    #never reached for current example
                    if mdr_globals.DEBUG:print "VISUALIZE:220 check2 cat", cat, lintMisraType[elem]
                    report2 = False
            
                if report2 == True:
                    report = True;
 
    if mdr_globals.DEBUG:print "PC_Lint:\t\t", lin
    if mdr_globals.DEBUG:print "MISRA:\t\t\t", mis
    if mdr_globals.DEBUG:print "Level:\t\t\t", lev
    if mdr_globals.DEBUG:print "Category:\t\t", cat
    if mdr_globals.DEBUG:print "Justification:\t\t", jus
    
    #If 2004 Info is used then it should pass
    if mis == "MISRA 2004 Info":
        report = True;
    
    if report == False:
        return ["The MISRA rule in comment is not according to PC-Lint mapping, e.g.\n you choose category mandatory, but is is required, compare mdr_analyse_report.txt", report, lin, mis, lev, cat, jus]

    if mdr_globals.DEBUG:print "PC_Lint:\t\t", lin
    if mdr_globals.DEBUG:print "MISRA:\t\t\t", mis
    if mdr_globals.DEBUG:print "Level:\t\t\t", lev
    if mdr_globals.DEBUG:print "Category:\t\t", cat
    if mdr_globals.DEBUG:print "Justification:\t\t", jus
    
    return [error, report, lin, mis, lev, cat, jus]
