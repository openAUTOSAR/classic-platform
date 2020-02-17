/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with
 * the terms contained in the written license agreement between you and ArcCore,
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as
 * published by the Free Software Foundation and appearing in the file
 * LICENSE.GPL included in the packaging of this file or here
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/


#include "seqtest.h"

static char *stylesheet_;

void STest_XML_PrintHeader( STestPtr ref) {
#ifndef XML_OUTPUTTER_SKIP_HEADER
	printf("<?xml version=\"1.0\" encoding='shift_jis' standalone='yes' ?>\n");
	if (stylesheet_)
		printf("<?xml-stylesheet type=\"text/xsl\" href=\"%s\" ?>\n",stylesheet_);
#endif
	printf("<TestRun>\n");
}

void STest_XML_PrintStart( STestPtr ref ) {
	printf("<%s>\n",STest_GetMajorName(ref));
}

void STest_XML_PrintEnd( STestPtr ref) {
	printf("</%s>\n",STest_GetMajorName(ref));
}

void STest_XML_PrintOk( STestPtr ref) {
	printf("<Test id=\"%d\">\n",STest_GetMajor(ref));
	printf("<Name>%s</Name>\n",STest_GetMajorName(ref));
	printf("</Test>\n");

}

void STest_XML_PrintFail(STestPtr ref , char *msg, char* file ,int line ,char* function) {
	printf("<FailedTest id=\"%d\">\n",STest_GetMajor(ref));
	printf("<Name>%s</Name>\n",STest_GetMajorName(ref));
	printf("<Location>\n");
	printf("<File>%s</File>\n",file);
	printf("<Line>%d</Line>\n",line);
	printf("</Location>\n");
	printf("<Message>%s</Message>\n",msg);
	printf("</FailedTest>\n");
}

void STest_XML_PrintStatistics( STestPtr ref) {
	printf("<Statistics>\n");
	printf("<Tests>%d</Tests>\n",STest_GetOkCount(ref));
	if (STest_GetFailCount(ref)) {
		printf("<Failures>%d</Failures>\n",STest_GetFailCount(ref));
	}
	printf("</Statistics>\n");
	printf("</TestRun>\n");
}



void STest_Text_PrintHeader( STestPtr ref) {
	(void)ref;
}

void STest_Text_PrintStart( STestPtr ref ) {
	printf("- %s\n",STest_GetMajorName(ref));
}

void STest_Text_PrintEnd( STestPtr ref) {
}

void STest_Text_PrintOk( STestPtr ref) {
#ifndef CFG_MINIMAL_OUTPUT
	printf("%d) OK %s\n", runCount, Test_name(test));
#endif
}

void STest_Text_PrintFail(STestPtr ref , char *msg, char* file ,int line ,char* function) {
	(void)function;
#ifdef CFG_MINIMAL_OUTPUT
	printf("%d) NG %s (%s %d)\n %s\n", ref->testMajor, STest_GetMajorName(ref), file, line, msg);
#else
	printf("%d) NG %s (%s %d) %s\n", ref->testMajor, STest_GetMajorName(ref), file, line, msg);
#endif

}

void STest_Text_PrintStatistics( STestPtr ref) {
	if (STest_GetFailCount(ref) + STest_GetNotRunCount(ref) ) {
		printf("\nrun %d failures %d notrun %d\n",ref->testMajor,STest_GetFailCount(ref), STest_GetNotRunCount(ref));
	} else {
		printf("\nOK (%d tests)\n",STest_GetOkCount(ref));
	}
}


