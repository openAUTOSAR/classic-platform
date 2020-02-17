#!/usr/bin/env python
# -*- coding: latin-1 -*-

###
### Copyright 2017- ArcCore GmbH. All rights reserved.
###

"""ARCCORE MISRA Deviation Reporter
   Collect Files

   Parses the input folder for files with .h/.c endings
   Removes the folders which are mentioned in the exception list
    
   returns the file names as an file name list
   
   readExcpetionsFoldernames - Reads the folder names of the exception folders from an external file
   collectFiles - Parses the input folder for files with .h/.c endings
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

import os
import mdr_globals
import mdr_reporter

#----------------------------------------------------------

def readExcpetionsFoldernames(ExceptionsName):
    """
    Reads the folder names of the exception folders from an external file
    
    ExceptionsName - File which contains folder/files to be excluded from the analysis
    
    returns elist
    elist - the folder names as an exception list
    """

    #list of folders shall be excluded from MISRA deviation check, e.g. utest, etc.
    elist=[]

    try:
        lines = open(ExceptionsName).readlines()
        for line in lines:
            elist.append(line.strip())
    except:
        print "ERROR: CAN NOT OPEN: ", ExceptionsName

    #DEBUG-Mode: reports the collected folder/files to be excluded
    if mdr_globals.DEBUG:       
        mdr_reporter.reportListAndLen(elist, "Exceptions")

    return elist

#----------------------------------------------------------

def collectFiles(folderName, ExceptionsName):
    """
    Parses the input folder for files with .h/.c endings
    Removes the folders which are mentioned in the exception list
    
    folderName - Root folder recursively scanned for files with ending .h/.c
    ExceptionsName - File which contains folder/files to be excluded from the analysis
    
    returns rlist
    rlist - the file names as an file name list
    """

    #DEBUG-Mode: reports the execution of this function
    if mdr_globals.DEBUG:    
        mdr_reporter.reportCollectFiles()
    
    #The root folder is set to default="./" in case the name is empty
    if folderName == "":
        folderName = "./"
            
    #list of files shall be excluded from MISRA deviation check, e.g. test files, etc., default=empty
    if ExceptionsName != "":
        elist = readExcpetionsFoldernames(ExceptionsName)
    else:
        elist=[]

    #list of files to be investigated, currently only files with .c or .h are checked
    flist=[]
    for root, dirs, files in os.walk(folderName):
        for file in files:
            if file.endswith(".h") or file.endswith(".c"):
                #DEBUG-Mode: report the found file name
                if mdr_globals.DEBUG: 
                    print(os.path.join(root, file))
                flist.append(os.path.join(root, file))

    #remove files which are on exception list
    rlist=[]
    for file in flist:
        check = True
        for elem in elist:
            if file.find(elem) >= 0:
                #DEBUG-Mode: report the removed file name
                if mdr_globals.DEBUG: 
                    print "removed: ", file
                check = False

        if check:
            rlist.append(file)

    #DEBUG-Mode: reports the final collected files to be analysed
    if mdr_globals.DEBUG:
        mdr_reporter.reportListAndLen(rlist, "Analysis")

    return rlist