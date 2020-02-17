#!/usr/bin/env python
# -*- coding: latin-1 -*-

###
### Copyright 2017- ArcCore GmbH. All rights reserved.
###

"""ARCCORE MISRA Deviation Reporter
   Requires at input a Lint file, e.g. au-misra3.Int and generates a text file with mapping from Lint to MISRA
   e.g. python mdr.py -r XXXXXXXXXXXXXXXXX > XXX.txt

    +-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    MISRA 2012 compliance - allowed deviations
    +-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    MISRA 2012 has three levels of rules.
    ARCCORE PC-lint configuration includes the configuration file au-misra3.lnt which will make PC-lint print the MISRA rule that is breached.
    To make a deviation to a MISRA rule you need to check up the MISRA rule to see if you are allowed to make a deviation:
    
    +-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    MANDATORY: You are not allowed to make deviations to this rule unless it falls within the category 'FALSE POSITIVE'.
    Even if you are convinced that the code works correct, this warning usually is a sign of code that is hard to read.
    Consider if the code can be rewritten before adding deviation. 

    REQUIRED: You can make deviations to this rule if it falls within any of the categories below, except 'OTHER'.
    If you still think a deviation is needed, consult with product management if a new deviation category shall be added. 

    ADVISORY: You can make deviations to this rule if necessary, following the deviation procedure below. 
    If you make a deviation to a MISRA rule, add the MISRA rule to the comment so it is visible for reviewers and other readers of the code, e.g. [MISRA 2012 Rule 10.3, required].
    +-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    
    If the project follows MISRA 2004, follow the same procedure for advisory and required rules (mandatory does not exist). 
    PC-lint will also make warnings that are not derived from a MISRA rule, in that case the deviation procedure as for MISRA advisory rules shall be followed.

    +-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    Deviation Procedure
    +-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    It is allowed to make exceptions from the MISRA rules by stating a comment why the exception is made.
    Each exception must be categorized into one of the categories below. The exception is approved as a part of the peer to peer review process.

    +-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    CONFIGURATION 
    Configuration parameters are used in expression and checks.
    The MISRA checker may complain that the expression is always false or true since the configuration is considered static.
    The configuration will be different and the result of the expression may change. An exception is allowed.
    
    STANDARDIZED INTERFACE 
    Many modules are based on the AUTOSAR standard which defines the public functions for the modules. This will sometimes break the MISRA rules e.g. an argument pointer is not const.
    The standard should always be followed and an exception from the MISRA rule shall be made.
    
    HARDWARE ACCESS 
    Hardware accesses may often cause various MISRA warnings. Add a short motivation why this deviation is applicable / needed if not obvious.
    
    ARGUMENT CHECK 
    MISRA states that each function may only have on exit point i.e one return statement. It is allowed to make exceptions from this rule for arguments checks in the beginning of the function.
    The rationale is that the it is much easier to read the function without additional if cases that is needed to fullfil the rule. This is typically used for DET errors.
    
    PERFORMANCE 
    Exceptions may be justified to increase performance. This exception shall be used with care since maintainability is often prioritized.
    One example can be that a "goto" may be justified in the OS to increase the speed of task switches. Add a short motivation why this deviation is applicable / needed if not obvious.
    
    FALSE POSITIVE 
    The static code analyzer tools sometimes indicate an error falsely. It is then justified to disable the rule.
    
    EXTERNAL FILE 
    Files that are developed by a third party may be excluded from checks.
    This will however mean that the overall static code analysis level of the module may not be set to C3 if an exception is made and only C1 level is checked.
    
    OTHER 
    This category is used when an exception is proposed but it does not belong into any of the categories above.
    A description that explains why an exception is made.It should not describe lint rule that is disabled.
    +-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    runMISRADeviationReporter - Parses the collectes files in the list on line-by-line basis and writes deviation report
    printHelp - print usage and configuration parameter for MISRA Deviation Reporter
    cli - command line interface of the misra deviation reporter
    
    -h/--help       Print out help message
    
    Mandatory
    -r/--rules      <RuleName>: Name of the rule file, e.g. au-misra3.lnt'
    
    Optional
    -i/--input      <InputFolderName>: Name of the folder to read the input data'
                                       default="./"'
    -e/--exceptions <ExceptionName>: Name of the file which contains the folders'
                                     to exclude from reporting, default=""'
    -o/--output     <OutputFolderName>: Name of folder to write the output data'
                                        default="Reports"'
    -d/--debug      Enables the ouptut of debugging data to the console'
    -a/--analyse    Enables the analyser mode and the corresponding output files'
    -c/--check      Enables the checker mode and the corresponding output files'
    """

__author__ = "ARCCORE"
__date__ = "25 Apr 2017"

__version__ = "$Revision: 2017-05-26 $"
__credits__ = """Copyright 2017 Arccore GmbH. All rights reserved.
"""

#----------------------------------------------------------

# Known bugs that can't be fixed here:
# extractReportingInformation_NEW works only on single MISRA deviations
# needs to be updated for serveral ones, if needed

#----------------------------------------------------------

import os, fileinput, re, sys, csv, getopt
import mdr.mdr_pclint2misra
import mdr.mdr_globals
import mdr.mdr_reporter
import mdr.mdr_collectfiles
import mdr.mdr_analyser
import mdr.mdr_checker
        
#----------------------------------------------------------

def runMISRADeviationReporter(flist, lintMisraRule, lintMisraType, OutputFolderName):
    """
    Parses the collectes files in the list on line-by-line basis
    and writes deviation report
    
    flist - List files to be analysed
    lintMisraRule - Dictionary mapping PC-Lint numbers to MISRA rules/directives numbers
    lintMisraType - Dictionary mapping PC-Lint numbers to MISRA rules/directives categories
    OutputFolderName - Name of the folder to report to

    returns [errorFound, deviationsCount_ana, deviationsCount_chk]
    errorFound - True, if an error from MISRA Deviation Guide was found, only in CHECK-MODE
                 False, otherwise
    deviationsCount_ana - 0, if no MISRA deviation was found, otherwise > 0, only for ANALYSE-MODE
    deviationsCount_chk - 0, if no MISRA deviation was found, otherwise > 0, only for CHECK-MODE
    """

    #DEBUG-Mode: reports the execution of this function
    if mdr.mdr_globals.DEBUG:
        mdr.mdr_reporter.reportARCCORECopyright()

    #Initialise the statistical counter
    deviationsCount_ana = 0 #ANALYSE-Mode: counts all found deviations
    deviationsCount_chk = 0 #CHECK-Mode: counts all found deviations
    mandatoryCount_ana = 0  #ANALYSE-Mode: counts all found mandatory deviations
    mandatoryCount_chk = 0  #CHECK-Mode: counts all found mandatory deviations
    requiredCount_ana = 0   #ANALYSE-Mode: counts all found required deviations
    requiredCount_chk = 0   #CHECK-Mode: counts all found required deviations
    advisoryCount_ana = 0   #ANALYSE-Mode: counts all found advisory deviations
    advisoryCount_chk = 0   #CHECK-Mode: counts all found advisory deviations
    errorFound = False      #Indicator, if an ERROR was found, In this case the ERROR file is created
    modeEnabled = False     #Indicator for either enabled ANALYSIS or CHECK mode
                                
    modeEnabled = mdr.mdr_globals.ANALYSE or mdr.mdr_globals.CHECK

    #ANALYSE, CHECK-MODE: prepare the reporting folder and files
    if modeEnabled:
        #The output folder is set to default="./Reports" in case the name is empty    
        if OutputFolderName == "":
            OutputFolderName = "./Reports"

        #Creates the output folder in case it is not available yet, e.g. created by unit_report tool            
        if not os.path.isdir(OutputFolderName):
            os.makedirs(OutputFolderName)

        #ANALYSE-Mode: open file to report in analyse mode, everything
        if mdr.mdr_globals.ANALYSE:        
            fanalyse_all = open(OutputFolderName + "/mdr_analyse_report.txt", 'w')
            mdr.mdr_reporter.writeARCCORECopyright(fanalyse_all)

            #reporting only mandatory MISRA Rules/Directives deviations
            fanalyse_mandatory = open(OutputFolderName + "/mdr_analyse_mandatory_report.txt", 'w')
            mdr.mdr_reporter.writeARCCORECopyright(fanalyse_mandatory)

        #ANALYSE-Mode: open file to report in check mode, everything
        if mdr.mdr_globals.CHECK: 
            #reporting based on new extract
            fcheck = open(OutputFolderName + "/mdr_check_deviation_report.txt", 'w')
            mdr.mdr_reporter.writeARCCORECopyright(fcheck)
            
            if mdr.mdr_globals.CSV:
                fcheck_csv = open(OutputFolderName + "/mdr_check_deviation_report.csv", 'w')
                mdr.mdr_reporter.writeColumnNamesCSV(fcheck_csv)

            #remove error report file
            try:
                if os.path.exists(OutputFolderName + "/mdr_check_error_report.txt"):
                    #cmd = "rm " + OutputFolderName + "/mdr_check_error_report.txt"
                    #os.system(cmd)
					os.remove(OutputFolderName + "/mdr_check_error_report.txt")
            except:
                pass
        
        
        
        ###Disabled for now
        #reporting csv format for unit tool integration
        # fcsv_all = open(os.getcwd() + "\Reports\mdr_deviation_report.csv", 'w')
        # fieldNames = ('File', 'Line', 'PC-Lint', 'MISRA', 'Level', 'Category', 'Justification', 'Comment')
        # csv.excel.delimiter=';'
        # csv.excel.lineterminator='\n'
        # csvWriter = csv.writer(fcsv_all, dialect=csv.excel)
        # csvWriter.writerow(fieldNames)
                
    #Run file-by-file and analyse/check for deviations
    for fileName in flist:
        lines = open(fileName).readlines()
        lineNr = 0  
        check_ana = True    #ANALYSE-Mode: Indicator for found deviations, True - none found
        check_chk = True    #CHECK-Mode: Indicator for found deviations, True - none found

        #DEBUG-Mode: Report start of analysing a file
        if mdr.mdr_globals.DEBUG:        
            mdr.mdr_reporter.reportHeaderFilename(fileName)

        #ANALYSE-Mode: Report start of analysing a file
        if mdr.mdr_globals.ANALYSE:            
            mdr.mdr_reporter.writeHeaderFilename(fanalyse_all, fileName)

        #Run line-by-line for the dedicated file and analyse/check for deviations         
        for line in lines:
            lineNr = lineNr + 1 #count the line number for proper reporting
            #Check for the key words lint or MISRA (ANALYSE mode) in the line.
            if '/*lint' in line or "MISRA" in line:
            
                #ANALYSE-Mode: Reports all found deviations, independent of the defined MISRA Deviation Guide to allow developers a quick check of their code
                if mdr.mdr_globals.ANALYSE:
                    [report_ana, lin_ana, mis_ana, lev_ana, cat_ana, jus_ana] = mdr.mdr_analyser.extractReportingInformation(line, lintMisraRule, lintMisraType)

                    if (len(lin_ana) > 0 and report_ana == True):
                        #DEBUG-Mode: Report found deviations to console
                        if mdr.mdr_globals.DEBUG:
                            mdr.mdr_reporter.reportInformation(fileName.split("\\")[-1], lineNr, lin_ana, mis_ana, lev_ana, cat_ana, jus_ana, line)
                            
                        mdr.mdr_reporter.writeInformation(fanalyse_all, fileName.split("\\")[-1], lineNr, lin_ana, mis_ana, lev_ana, cat_ana, jus_ana, line)
                        
                        
                        
                        ###Disabled for now
                        #if mdr.mdr_globals.VERBOSE: csvWriter.writerow((fileName.split("\\")[-1], lineNr, lin, mis, lev, cat, jus, line))
                        check_ana = False
                        
                        #Update the deviations counter
                        deviationsCount_ana = deviationsCount_ana + 1
                        if "mandatory" in lev_ana:
                            mandatoryCount_ana = mandatoryCount_ana + 1
                            mdr.mdr_reporter.writeInformation(fanalyse_mandatory, fileName.split("\\")[-1], lineNr, lin_ana, mis_ana, lev_ana, cat_ana, jus_ana, line)
                        elif "required" in lev_ana:
                            requiredCount_ana = requiredCount_ana + 1
                        elif "advisory" in lev_ana:
                            advisoryCount_ana = advisoryCount_ana + 1

                #CHECK-Mode: Reports all found deviations, which follow the defined MISRA Deviation Guide and
                #            Reports all found error due to MISRA Deviation Guide
                if mdr.mdr_globals.CHECK:
                    [error_chk, report_chk, lin_chk, mis_chk, lev_chk, cat_chk, jus_chk] = mdr.mdr_checker.extractReportingInformation(line, lintMisraRule, lintMisraType)

                    #report deviations from defined MISRA Deviation Guide
                    if error_chk != "":
                        #reporting error based on new extract
                        if errorFound == False:
                            ferror = open(OutputFolderName + "/mdr_check_error_report.txt", 'w')
                            errorFound = True
                        else:
                            ferror = open(OutputFolderName + "/mdr_check_error_report.txt", 'a')

                        mdr.mdr_reporter.writeInformation(ferror, fileName.split("\\")[-1], lineNr, lin_chk, mis_chk, lev_chk, cat_chk, jus_chk, line.strip(), error_chk)

                        ferror.close()
                    else:
                        #report deviations following the defined MISRA Deviation Guide
                        if (len(lin_chk) > 0 and report_chk == True):
                            mdr.mdr_reporter.writeInformation(fcheck, fileName.split("\\")[-1], lineNr, lin_chk, mis_chk, lev_chk, cat_chk, jus_chk, line)
                            
                            if mdr.mdr_globals.CSV:
                                mdr.mdr_reporter.writeInformationCSV(fcheck_csv, fileName.split("\\")[-1], lineNr, lin_chk, mis_chk, lev_chk, cat_chk, jus_chk, line)
                            
                            check_chk = False
                            
                            deviationsCount_chk = deviationsCount_chk + 1
                            if "mandatory" in lev_chk:
                                mandatoryCount_chk = mandatoryCount_chk + 1
                            elif "required" in lev_chk:
                                requiredCount_chk = requiredCount_chk + 1
                            elif "advisory" in lev_chk:
                                advisoryCount_chk = advisoryCount_chk + 1

        if mdr.mdr_globals.ANALYSE:
            if check_ana:
                #DEBUG-Mode: Report if no devations have been found
                if mdr.mdr_globals.DEBUG: 
                    print "No MISRA deviation found in this file"
                    fanalyse_all.write("\nNo MISRA deviation found in this file")

##        if mdr.mdr_globals.VERBOSE:
##            if check_new:
##                if mdr.mdr_globals.DEBUG: print "No MISRA deviation found in this folder/file"
##                fcheck.write("\nNo MISRA deviation found in this folder/file")

        #ANALYSE-Mode: Reports the file based statistics
        if mdr.mdr_globals.ANALYSE:
            mdr.mdr_reporter.writeDeviationsStatistic(fanalyse_all, fileName.split("\\")[-1].strip(), deviationsCount_ana)

    #ANALYSE-Mode: Reports the folder based statistics for all analysed files
    if mdr.mdr_globals.ANALYSE:
        #DEBUG-Mode: reports the statistical parameter and the end of this function
        if mdr.mdr_globals.DEBUG:    
            mdr.mdr_reporter.reportMISRAStatistic(deviationsCount_ana, mandatoryCount_ana, requiredCount_ana, advisoryCount_ana)
            
        mdr.mdr_reporter.writeMISRAStatistic(fanalyse_all, deviationsCount_ana, mandatoryCount_ana, requiredCount_ana, advisoryCount_ana)
        mdr.mdr_reporter.writeMISRAStatistic(fanalyse_mandatory, mandatoryCount_ana, mandatoryCount_ana, 0, 0)
        
        if deviationsCount_ana == 0:
            #DEBUG-Mode: Report if no devations have been found
            if mdr.mdr_globals.DEBUG:
                print "No MISRA deviation found in this folder/file"
            fanalyse_all.write("\nNo MISRA deviation found in this folder/file")

    #CHECK-Mode: Reports the folder based statistics for all checked files
    if mdr.mdr_globals.CHECK: 
        #DEBUG-Mode: reports the statistical parameter and the end of this function
        if mdr.mdr_globals.DEBUG:    
            mdr.mdr_reporter.reportMISRAStatistic(deviationsCount_chk, mandatoryCount_chk, requiredCount_chk, advisoryCount_chk)
            
        mdr.mdr_reporter.writeMISRAStatistic(fcheck, deviationsCount_chk, mandatoryCount_chk, requiredCount_chk, advisoryCount_chk)

        if deviationsCount_chk == 0:
            #DEBUG-Mode: Report if no devations have been found
            if mdr.mdr_globals.DEBUG:
                print "No MISRA deviation found in this folder/file"
            fcheck.write("\nNo MISRA deviation found in this folder/file")

    #ANALYSE-Mode: Close the files in analyse mode
    if mdr.mdr_globals.ANALYSE:
        fanalyse_all.close()
        fanalyse_mandatory.close()
        #if mdr.mdr_globals.VERBOSE: fcsv_all.close()
        
    #CHECK-Mode: Close the files in check mode
    if mdr.mdr_globals.CHECK:         
        fcheck.close()
        
        if mdr.mdr_globals.CSV:
            fcheck_csv.close()
        
        
    return [errorFound, deviationsCount_ana, deviationsCount_chk]

#----------------------------------------------------------

def printHelp():
    """
    print usage and configuration parameter for MISRA Deviation Reporter
    """

    print '\n-h/--help       Print out this message'
    print '\nMandatory'
    print '-r/--rules      <RuleName>: Name of the rule file, e.g. au-misra3.lnt'
    print '\nOptional'
    print '-i/--input      <InputFolderName>: Name of the folder to read the input data'
    print '                                   default="./"'
    print '-e/--exceptions <ExceptionName>: Name of the file which contains the folders'
    print '                                 to exclude from reporting, default=""'
    print '-o/--output     <OutputFolderName>: Name of folder to write the output data'
    print '                                    default="Reports"'
    print '-d/--debug      Enables the ouptut of debugging data to the console'
    print '-a/--analyse    Enables the analyser mode and the corresponding output files'
    print '-c/--check      Enables the checker mode and the corresponding output files'
    print '-C/--csv        Generates report in CSV format'

#----------------------------------------------------------

def cli(argv):
    """mdr - ARCCORE MISRA Deviation Reporter"""

    if mdr.mdr_globals.DEBUG:
        mdr.mdr_reporter.reportHeader()

    InputFolderName = ''            #Name of the folder to read the input data
    RuleName = ''                   #Name of the rule file
    ExceptionsName = ''             #Name of the file which contains the folders to exclude from reporting
    OutputFolderName = './Reports'  #Name of the folder to write the output data
    checkRuleName = False           #Check for mandatory input configuration parameter RuleName

    try:
        opts, args = getopt.getopt(argv,"hr:e:i:o:dac",["help", "rules=", "exceptions=", "input=", "output=", "debug", "analyse", "check", "csv"])
    except getopt.GetoptError:
        printHelp()
        return
                                   
    for opt, arg in opts:
        if opt in ("-h", "--help"):
            printHelp()
            return
        elif opt in ("-r", "--rules"):
            RuleName = arg
            checkRuleName = True
        elif opt in ("-e", "--exceptions"):
            ExceptionsName = arg
        elif opt in ("-i", "--input"):
            InputFolderName = arg
        elif opt in ("-o", "--output"):
            OutputFolderName = arg
        elif opt in ("-d", "--debug"):
            mdr.mdr_globals.DEBUG = True
        elif opt in ("-a", "--analyse"):
            mdr.mdr_globals.ANALYSE = True
        elif opt in ("-c", "--check"):
            mdr.mdr_globals.CHECK = True
        elif opt in ("-C""--csv"):#
            mdr.mdr_globals.CSV = True

    #DEBUG-Mode: reports the configuration parameter to console
    if mdr.mdr_globals.DEBUG:
        mdr.mdr_reporter.reportConfigurationParameter(RuleName, ExceptionsName, InputFolderName, OutputFolderName, mdr.mdr_globals.DEBUG, mdr.mdr_globals.ANALYSE, mdr.mdr_globals.CHECK, mdr.mdr_globals.CSV)

    #Check for mandatory parameter
    if (checkRuleName):
        #Maps PC-Lint number to MISRA Rule/Directive, One PC-Lint number can be mapped to multiple MISRA Rules/Directives (1:N)
        #lintMisraRule: requirements dictionary, key: lint number, value: Rule, e.g. 22.2
        #lintMisraType: requirements dictionary, key: lint number, value: mandatory|required|advisory
        [count, lintMisraRule, lintMisraType] = mdr.mdr_pclint2misra.countandMapLintMisraRules(RuleName)
 
        #Collects recursively all files to investigate from the defined input folder
        flist = mdr.mdr_collectfiles.collectFiles(InputFolderName, ExceptionsName)

        #Runs the Misra Deviation Reporter including Anaylser and Checker of the MISRA Rules/Directives       
        [errorFound, deviationsCount_ana, deviationsCount_chk] = runMISRADeviationReporter(flist, lintMisraRule, lintMisraType, OutputFolderName)
        
        #DEBUG-Mode: reports the configuration parameter to console
        if mdr.mdr_globals.DEBUG:
            mdr.mdr_reporter.reportResultValues(errorFound, deviationsCount_ana, deviationsCount_chk)        
        
    else:
       printHelp()

#----------------------------------------------------------

if __name__ == '__main__':
        
    cli(sys.argv[1:])
