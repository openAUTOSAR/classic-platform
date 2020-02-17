#!/usr/bin/env python
# -*- coding: latin-1 -*-

###
### Copyright 2017- ArcCore GmbH. All rights reserved.
###

"""ARCCORE MISRA Deviation Reporter
   Reporter

   reportHeader - Function Header Report printted to console
   reportConfigurationParameter - Configuration Parameter Report printted to console
   reportPCLint2MisraMapper - PC-Lint-MISRA-Mapping Report printted to console
   reportMisraStatistics - MISRA Rule/Directive Report printted to console
   reportCollectFiles - Collected Files Report printted to console
   reportListAndLen - Name, Lenght of a list and the content of the list printted to console
   reportARCCORECopyright - ARCCORE Copyright printted to console
   writeARCCORECopyright - ARCCORE Copyright written to a file
   reportMISRAStatistic - Amount of global MISRA deviations printted to console
   writeMISRAStatistic - Amount of global MISRA deviations written to a file
   writeDeviationsStatistic - Amount of MISRA deviations  written to a file
   reportInformation - MISRA deviation report printted to console
   writeInformation - MISRA deviation report written to a file
   reportHeaderFilename - File Header Report printted to console
   writeHeaderFilename - File Header Report written to a file
   reportResultValues - Final results of misraDeviationsReporter printted to console
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

def reportHeader(): 
    """
    Formatted Header File output to console
    """  
    print "\n"
    print "=================================================="
    print "mdr - ARCCORE MISRA Deviation Reporter            "
    print "=================================================="
    print "\n"

#----------------------------------------------------------

def reportConfigurationParameter(RuleName, ExceptionsName, InputFolderName, OutputFolderName, DEBUG, ANALYSE, CHECK): 
    """
    Formatted Configuration Parameter output to console
    """ 
    print "\n"
    print "Rule filename:       ", RuleName
    print "Exceptions filename: ", ExceptionsName
    print "Folder name:         ", InputFolderName
    print "Output foldername:   ", OutputFolderName
    print "DEBUG enabled:       ", DEBUG
    print "ANALYSE enabled:     ", ANALYSE
    print "CHECK enabled:       ", CHECK
    print "\n"
        
#----------------------------------------------------------

def reportPCLint2MisraMapper(): 
    """
    Formatted Function output to console
    """       
    print "\n"
    print "=============================================="
    print "Counts lines with form e.g.                   "
    print "-append(9047,[MISRA 2012 Rule 22.5, mandatory]"
    print "and generates mapping of lint to misra rule   "
    print "=============================================="
    print "\n"

#----------------------------------------------------------

def reportLint2MisraMap(line, lintNr, misraRule, misraType):
    """
    Formatted PC-Lint-MISRA-Map output to console
    """  
    print "----------------------------------"
    print line 
    print "lint      :\t", lintNr
    print "misra     :\t", misraRule
    print "misra type:\t", misraType
                
#----------------------------------------------------------    

def reportMisraStatistics(count, lintMisraRule):
    """
    Formatted MISRA Statistic output to console
    """     
    print "\n"
    print "=============================================="
    print "Statistics                                    "
    print "Found:\t", count, "\tMISRA rules              "
    print "Removed duplicate mappings:                   " 
    print "Count:\t", len(lintMisraRule), "\tlint rules  "
    print "=============================================="
     
#----------------------------------------------------------    

def reportCollectFiles(): 
    """
    Formatted Function output to console
    """     
    print "\n"
    print "=============================================="
    print "Collect all files from defined folder         "
    print "=============================================="
    print "\n"
 
#----------------------------------------------------------   
    
def reportListAndLen(list, name):
    """
    Formatted Statistic output to console
    """ 
    print "\n"
    print "=============================================="
    print "Statistics: ", name
    print "List:\t", len(list), "\tfiles       "
    print "=============================================="
    print list
    print "\n"

#----------------------------------------------------------     
    
def reportARCCORECopyright():
    """
    Formatted file header
    """  
    print "\n"
    print "=============================================="
    print "Copyright (C) 2013-2017                       "
    print "ArcCore AB, Sweden, www.arccore.com.          "
    print "Contact: <contact@arccore.com>                "
    print "\n"
    print "MISRA Deviation Report                        "
    print "=============================================="
    print "\n"

def writeARCCORECopyright(f):
    """
    Formatted file header
    """  
    f.write("\n")
    f.write("\n==============================================")
    f.write("\nCopyright (C) 2013-2017                       ")
    f.write("\nArcCore AB, Sweden, www.arccore.com.          ")
    f.write("\nContact: <contact@arccore.com>                ")
    f.write("\n")
    f.write("\nMISRA Deviation Report                        ")
    f.write("\n==============================================")
    f.write("\n")
    
#----------------------------------------------------------     
    
def reportMISRAStatistic(deviationsCount, mandatoryCount, requiredCount, advisoryCount):
    """
    Formatted Deviations output to console
    """         
    print "\n"
    print "=============================================="
    print "Deviations reported: ", deviationsCount
    print "Mandatory deviations: ", mandatoryCount
    print "Required deviations: ", requiredCount
    print "Advisory deviations: ", advisoryCount
    print "END OF MISRA deviation report                 "
    print "=============================================="
    print "\n"

def writeMISRAStatistic(f, deviationsCount, mandatoryCount, requiredCount, advisoryCount):
    """
    Formatted Deviations report for a file
    """
    f.write("\n")
    f.write("\n==============================================")
    f.write("\nDeviations reported: " + str(deviationsCount))
    f.write("\nMandatory deviations: " + str(mandatoryCount))
    f.write("\nRequired deviations: " + str(requiredCount))
    f.write("\nAdvisory deviations: " + str(advisoryCount))
    f.write("\nEND OF MISRA deviation report                 ")
    f.write("\n==============================================")
    f.write("\n")
     
#----------------------------------------------------------
    
def writeDeviationsStatistic(f, fileName, count):
    """
    Formatted Deviations report
    """
    f.write("\n")
    f.write("\n==============================================")
    f.write("\nDeviations reported: " + str(count))
    f.write("\nEND OF MISRA deviation report for " + fileName)
    f.write("\n----------------------------------------------------------")
    f.write("\n")
    
#----------------------------------------------------------

def reportInformation(fil, lin, pcl, mis, lev, cat, jus, com, error=""):
    """
    Formatted reporting of the found MISRA deviation
    
    File:           Apa.c
    Line:           237
    PC-Lint:        960
    MISRA:          2004 Rule 11.1
    Level:          required
    Category:       HARDWARE_ACCESS
    Justification:  N/A
    Comment:        e.g. the found line: /*lint -e{960} FALSE_POSITIVE*/
    """
    print "\n"
    if error != "":
        print "\n=============================================="
        print "\nERROR: " + str(error)
    print "=============================================="
    print "File:\t\t\t", fil
    print "Line:\t\t\t", str(lin)
    print "PC_Lint:\t\t", str(pcl)
    print "MISRA:\t\t\t", mis
    print "Level:\t\t\t", lev
    print "Category:\t\t", cat
    print "Justification:\t", jus
    print "\n"
    print "Comment:\t", com.strip()
    print "=============================================="

def writeInformation(f, fil, lin, pcl, mis, lev, cat, jus, com, error=""):
    """
    Foratted reporting of the found MISRA deviation to a file
    
    File:           Apa.c
    Line:           237
    PC-Lint:        960
    MISRA:          2004 Rule 11.1
    Level:          required
    Category:       HARDWARE_ACCESS
    Justification:  N/A
    Comment:        e.g. the found line: /*lint -e{960} FALSE_POSITIVE*/
    """
    f.write("\n")
    if error != "":
        f.write("\n==============================================")
        f.write("\nERROR: " + str(error))
    f.write("\n==============================================")
    f.write("\nFile:\t\t\t" + fil)
    f.write("\nLine:\t\t\t" + str(lin))
    f.write("\nPC_Lint:\t\t" + str(pcl))
    f.write("\nMISRA:\t\t\t" + mis)
    f.write("\nLevel:\t\t\t" + lev)
    f.write("\nCategory:\t\t" + cat)
    f.write("\nJustification:\t" + jus)
    f.write("\n")
    f.write("\nComment: " + com.strip())
    f.write("\n==============================================")
    
    
def writeInformationCSV(f, fil, lin, pcl, mis, lev, cat, jus, com, error=""):
    """
    MISRA deviation in CSV format to a file.
    
    Format:
    File;Line;PC-Lint;MISRA;Level;Category;Justification;Comment
    
    Ex:
    Apa.c;237;960;2004 Rule 11.1;required;HARDWARE_ACCESS;N/A;e.g. the found line: /*lint -e{960} FALSE_POSITIVE*/
    """
    if error != "":
        f.write("\n==============================================")
        f.write("\nERROR: " + str(error))
    f.write(fil + ";")
    f.write(str(lin) + ";")
    f.write(str(pcl) + ";")
    f.write(mis + ";")
    f.write(lev + ";")
    f.write(cat + ";")
    f.write(jus + ";")
    f.write(com.strip())
    f.write("\n")
    
def writeColumnNamesCSV(filename):  
    filename.write("File;Line;PC-Lint;MISRA;Level;Category;Justification;Comment\n")
#----------------------------------------------------------    
 
def reportHeaderFilename(fileName):
    """
    Formatted Checking output to console
    """  
    print "\n\n----------------------------------------------------------"
    print "Checking: ", fileName.strip(), "\n"
        
def writeHeaderFilename(f, fileName):
    """
    Formatted Checking output to a file
    """ 
    f.write("\n\n----------------------------------------------------------")
    f.write("\nChecking: " + fileName.strip() + "\n")
    
#----------------------------------------------------------

def reportResultValues(errorFound, deviationsCount_ana, deviationsCount_chk):
    """
    Formatted Result output to console
    """  
    print "Found errors ?:\t\t\t", errorFound
    print "ANALYSE-MODE: Found deviations:\t", deviationsCount_ana
    print "CHECK-MODE: Found deviations:\t", deviationsCount_chk
    print "----------------------------------------------------------\n\n"    