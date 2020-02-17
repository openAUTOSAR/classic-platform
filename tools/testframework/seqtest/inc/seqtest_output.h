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


#ifndef SEQTEST_OUTPUT_H_
#define SEQTEST_OUTPUT_H_

void STest_XML_PrintHeader( STestPtr ref);
void STest_XML_PrintStart( STestPtr ref);
void STest_XML_PrintEnd( STestPtr ref);
void STest_XML_PrintOk( STestPtr ref);
void STest_XML_PrintFail(STestPtr, char *msg, char* file ,int line ,char* function);
void STest_XML_PrintStatistics( STestPtr ref);

void STest_Text_PrintHeader( STestPtr ref);
void STest_Text_PrintStart( STestPtr ref);
void STest_Text_PrintEnd( STestPtr ref);
void STest_Text_PrintOk( STestPtr ref);
void STest_Text_PrintFail(STestPtr, char *msg, char* file ,int line ,char* function);
void STest_Text_PrintStatistics( STestPtr ref);

#endif /* SEQTEST_OUTPUT_H_ */
