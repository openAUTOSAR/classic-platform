#!/usr/bin/env python
# -*- coding: latin-1 -*-

###
### Copyright 2009-2017 ArcCore GmbH. All rights reserved.
###

"""ARCCORE MISRA Test Report Generator
"""

__author__ = "ARCCORE"
__date__ = "05 May 2017"

__version__ = "$Revision: 2017-05-08 $"
__credits__ = """Copyright 2009-2017 Arccore GmbH. All rights reserved.
"""

# Known bugs that can't be fixed here:
# N/A


import sys
import os
import subprocess

# Global definitions

MISRADEVREPLOC   = "../"
MISRARULESLOC    = "../../pclint/lnt/"
MISRAEXCEPTSLOC  = "test_config/"
MISRAFOLDERLOC   = "test_folder"
MISRAREPORTLOC   = "Reports/"

# Global used command definitions

cmd_misra_report            =                    "python "
cmd_misra_report            = cmd_misra_report + MISRADEVREPLOC
cmd_misra_report            = cmd_misra_report + "misraDeviationReporter.py"
cmd_misra_report            = cmd_misra_report + " -r "
cmd_misra_report            = cmd_misra_report + MISRARULESLOC
cmd_misra_report            = cmd_misra_report + "au-misra3.lnt"
cmd_misra_report            = cmd_misra_report + " -e "
cmd_misra_report            = cmd_misra_report + MISRAEXCEPTSLOC
cmd_misra_report            = cmd_misra_report + "mdr_exceptions.txt"
cmd_misra_report            = cmd_misra_report + " -i "
cmd_misra_report            = cmd_misra_report + MISRAFOLDERLOC
cmd_misra_report            = cmd_misra_report + " -o "
cmd_misra_report            = cmd_misra_report + MISRAREPORTLOC
cmd_misra_report            = cmd_misra_report + " -d"
cmd_misra_report            = cmd_misra_report + " -a"
cmd_misra_report            = cmd_misra_report + " -c"
print cmd_misra_report

print "\n"
print "===================================="
print "Start to generate the reports       "
print "===================================="
print "\n"

#enables to execute the script by double click on the explorer
os.chdir(os.path.dirname(os.path.realpath(__file__)))

try:
    print cmd_misra_report
    os.system(cmd_misra_report)
    print "INFO: MISRA Test Report Generator - reporting SUCCESSFULLY done"
except:
    print "ERROR: MISRA Test Report Generator - reporting FAILURE"
    
print "\n"
print "===================================="
print "Compare the reports       "
print "===================================="
print "\n"

if os.path.exists("compare0.txt"):
    os.system("rm compare0.txt")
if os.path.exists("compare1.txt"):
    os.system("rm compare1.txt")
if os.path.exists("compare2.txt"):
    os.system("rm compare2.txt")
if os.path.exists("compare3.txt"):
    os.system("rm compare3.txt")
    
try:     
    os.system("diff Reports/mdr_analyse_report.txt Reports_Ref/mdr_analyse_report.txt > compare0.txt")
    os.system("diff Reports/mdr_analyse_mandatory_report.txt Reports_Ref/mdr_analyse_mandatory_report.txt > compare1.txt")
    os.system("diff Reports/mdr_check_deviation_report.txt Reports_Ref/mdr_check_deviation_report.txt > compare2.txt")
    os.system("diff Reports/mdr_check_error_report.txt Reports_Ref/mdr_check_error_report.txt > compare3.txt")
    
    check = False
    if os.stat("compare0.txt").st_size != 0:
        check = True
    if os.stat("compare1.txt").st_size != 0:
        check = True
    if os.stat("compare2.txt").st_size != 0:
        check = True
    if os.stat("compare3.txt").st_size != 0:
        check = True
        
    if check:
        print "!!!ERROR - Generated files and expected files differ!!!"
    
    print "INFO: Compare Test Report Generator - reporting SUCCESSFULLY done"
except:
    print "ERROR: Compare Test Report Generator - reporting FAILURE"

    
print "\n"
print "===================================="
print "Finished to generate the reports    "
print "===================================="
print "\n"

if __name__ == '__main__':
    pass
