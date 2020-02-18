#!/usr/bin/env python
# -*- coding: latin-1 -*-

###
### Copyright 2017- ArcCore GmbH. All rights reserved.
###

"""ARCCORE MISRA Deviation Reporter
   Global definitions
   
   DEBUG, VERBOSE
   MISRA_FORM, MISRA_CATEGORIES, EXECPTIONS_CATEGORIES
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

#Global DEBUG flag, True-debug on, else off
DEBUG = False

#Global VERBOSE flag, True-Verbose on, else off
ANALYSE = False

#Global VERBOSE flag, True-Verbose on, else off
CHECK = False

#Global CSV flag, True-CSV generation on, else off
CSV = False

#----------------------------------------------------------
                
#Misra provides Rules and Directives
MISRA_FORM = ['MISRA 2012 Rule','MISRA 2012 Directive','MISRA 2004 Info']

#Misra categories
MISRA_CATEGORIES = ['advisory','required','mandatory']

#ARCCOREs justification categories to comment on deviations
EXECPTIONS_CATEGORIES = ['CONFIGURATION',\
                         'STANDARDIZED INTERFACE', 'STANDARDIZED_INTERFACE',\
                         'HARDWARE ACCESS', 'HARDWARE_ACCESS',\
                         'ARGUMENT CHECK', 'ARGUMENT_CHECK',\
                         'PERFORMANCE',\
                         'FALSE POSITIVE', 'FALSE_POSITIVE',\
                         'EXTERNAL FILE', 'EXTERNAL_FILE',\
                         'OTHER']