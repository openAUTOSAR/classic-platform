#!/usr/bin/env python
# -*- coding: latin-1 -*-

###
### Copyright 2017- ArcCore GmbH. All rights reserved.
###

"""ARCCORE MISRA Deviation Reporter
   PC-Lint to MISRA Mapper
   
   Parses the input rule file on line-by-line basis
   for rules/directives having the form:
   "-append(9047,[MISRA 2012 Rule 22.5, mandatory])"
   set the mapping tables

   returns the amount of such lines and the mapped rules/directives tables
   
   extractMappingInfo - Extracts the information
   countandMapLintMisraRules - Parses the input rule file on line-by-line basis
"""

__author__ = "ARCCORE"
__date__ = "06 May 2017"

__version__ = "$Revision: 2017-05-10 $"
__credits__ = """Copyright 2017 Arccore GmbH. All rights reserved.
"""

#----------------------------------------------------------

# Known bugs that can't be fixed here:
# N/A

#----------------------------------------------------------

import os, fileinput, re
import mdr_globals
import mdr_reporter

#----------------------------------------------------------
    
def extractMappingInfo(line):
    """
    Extracts the information from a line having the form
    
    line - content of the line to be analysed

    returns [Lint number, Misra,              Compliance level]
    e.g.    [950,         2012:Directive:2.1, required]
    """

    s1 = re.split(",",line)
    #s1 = ['-append(9047', '[MISRA 2012 Rule 22.5', ' mandatory])']
    s20 = re.split("\(",s1[0])
    #s20 = ['-append', '9047']
    s21 = re.split(" ",s1[1])
    #s21 = ['[MISRA', '2012', 'Rule', '22.5']
    s22 = re.split("\]",s1[2])
    #[' mandatory', ')']

    return [int(s20[1].strip()), s21[1].strip()+":"+s21[2].strip()+":"+s21[3].strip(), s22[0].strip()]

#----------------------------------------------------------
            
def countandMapLintMisraRules(filename):
    """
    Parses the input rule file on line-by-line basis
    for rules having the form: "-append(9047,[MISRA 2012 Rule 22.5, mandatory])"
    set the mapping tables
    
    filename - name of the file to be analysed

    returns [count, lintMisraRule, lintMisraType]
    count - Amount of PC-Lint numbers found
    lintMisraRule - Dictionary mapping PC-Lint numbers to MISRA rules/directives numbers
    lintMisraType - Dictionary mapping PC-Lint numbers to MISRA rules/directives categories
    """
    
    #DEBUG-Mode: reports the execution of this function
    if mdr_globals.DEBUG:
        mdr_reporter.reportPCLint2MisraMapper()
    
    #counter for the amount of lines
    count = 0
    #requirements dictionary, key: lint number, value: Rule, e.g. 22.2
    lintMisraRule = dict()
    #requirements dictionary, key: lint number, value: mandatory|required|advisory
    lintMisraType = dict()
    
    for line in fileinput.input(filename):    
        rule = re.findall("-append", line)
        isRuleNotCommentOut = True
        if "//" in line.split("-append")[0]:
            isRuleNotCommentOut = False
                
        if len(rule) > 0 and isRuleNotCommentOut:                
            count = count + 1
            [lintNr, misraRule, misraType] = extractMappingInfo(line)

            #DEBUG-Mode: reports the mapping of lint number to misar rules/directives number
            if mdr_globals.DEBUG:
                mdr_reporter.reportLint2MisraMap(line, lintNr, misraRule, misraType)

            #Map lint to MISRA rule
            if lintNr in lintMisraRule:
                #DEBUG-Mode: reports duplicated MISRA rule/directives
                if mdr_globals.DEBUG: 
                    print "INFO: already as key available, appendend: ", lintNr
                lintMisraRule[lintNr]= lintMisraRule[lintNr] + ","+misraRule
                lintMisraType[lintNr]= lintMisraType[lintNr] + ","+misraType
            else:
                lintMisraRule[lintNr]=misraRule
                lintMisraType[lintNr]=misraType
        elif len(rule) > 0:
            #DEBUG-Mode: reports comment out MISRA rule/directives
            if mdr_globals.DEBUG:
                print "----------------------------------"
                print line 
                print "\nINFO: rule/directive is comment out, not mapped: ", isRuleNotCommentOut   
          
 
    #DEBUG-Mode: reports the dictionary
    if mdr_globals.DEBUG:
        print lintMisraRule
        print lintMisraType
 
    #DEBUG-Mode: reports amount of total rules and rules without duplicates
    if mdr_globals.DEBUG: 
        mdr_reporter.reportMisraStatistics(count, lintMisraRule)

    return [count, lintMisraRule, lintMisraType]
    
#----------------------------------------------------------

if __name__ == '__main__':
        
    if mdr_globals.DEBUG: print "\n"
    if mdr_globals.DEBUG: print "=================================================="
    if mdr_globals.DEBUG: print "mdr - ARCCORE MISRA Deviation Reporter            "
    if mdr_globals.DEBUG: print "      PC-Lint to MISRA Mapper                     "
    if mdr_globals.DEBUG: print "=================================================="
    if mdr_globals.DEBUG: print "\n"

    if mdr_globals.DEBUG:print "\n"
    if mdr_globals.DEBUG:print "Rule filename: ../../pclint/lnt/au-misra3.lnt"
    if mdr_globals.DEBUG:print "\n"

    if mdr_globals.DEBUG:print "\n"
    if mdr_globals.DEBUG:print "=============================================="
    if mdr_globals.DEBUG:print "Counts lines with form e.g.                   "
    if mdr_globals.DEBUG:print "-append(9047,[MISRA 2012 Rule 22.5, mandatory]"
    if mdr_globals.DEBUG:print "and generates mapping of lint to misra rule   "
    if mdr_globals.DEBUG:print "=============================================="
    if mdr_globals.DEBUG:print "\n"

    [count, lintMisraRule, lintMisraType] = countandMapLintMisraRules("../../pclint/lnt/au-misra3.lnt")
    
    if mdr_globals.DEBUG:
        print lintMisraRule
        print lintMisraType

    if mdr_globals.DEBUG:print "\n"
    if mdr_globals.DEBUG:print "=============================================="
    if mdr_globals.DEBUG:print "Statistics                                    "
    if mdr_globals.DEBUG:print "Found:\t", count, "\tMISRA rules              "
    if mdr_globals.DEBUG:print "Removed duplicate mappings:                   " 
    if mdr_globals.DEBUG:print "Count:\t", len(lintMisraRule), "\tlint rules  "
    if mdr_globals.DEBUG:print "=============================================="