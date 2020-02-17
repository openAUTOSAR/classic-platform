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
#ifndef _DATABASE_H
#define _DATABASE_H

#include <sqlite3.h>

// Create requested registers not already in the database
//
// On a register read
#define DB_CREATE_MISSING_READ 1
// On a register write
#define DB_CREATE_MISSING_WRITE 1

#define DB_OK          0
#define DB_FAIL_EXISTS 1
#define DB_FAIL_OPEN   2
#define DB_FAIL_CREATE 3
#define DB_FAIL_INSERT 4
#define DB_FAIL_SELECT 5

#define DB_DEFAULT_FILE_NAME "/tmp/arccore.sqlite3"

// mS Busy timeout. To allow for locked DB
#define DB_BUSY_TIMEOUT 250
//


#define DB_ECU_RUNTIME_REG_ADDRESS    1


#define DB_CHECK "SELECT * FROM arccore_cal"

#define DB_CREATE "CREATE TABLE IF NOT EXISTS 'arccore_cal' ( \
            'registeraddress' INTEGER NOT NULL, \
            'registervalue' INTEGER NOT NULL, \
            'writeable' INTEGER NOT NULL DEFAULT (1), \
            'tagname' TEXT, 'tagdescription' TEXT, \
            'sensortimestamp' DATETIME DEFAULT CURRENT_TIMESTAMP);" 

#define DB_CREATE_INDEX "CREATE UNIQUE INDEX 'main'.'arccore_cal_index' ON 'arccore_cal' ('registeraddress' ASC);"

#define DB_CREATE_TRIGGER "CREATE TRIGGER [UpdateLastTime] AFTER UPDATE ON arccore_cal \
                        FOR EACH ROW \
                        BEGIN \
                        UPDATE arccore_cal SET sensortimestamp = CURRENT_TIMESTAMP WHERE \
                        registeraddress = old.registeraddress; \
                        END"

#define DB_GET_REGISTER "SELECT %s FROM arccore_cal WHERE \
                registeraddress >= %d AND \
                registeraddress < ( %d + %d ) \
        %s \
                ORDER BY registeraddress ASC;"

#define DB_GET_SELECT "registeraddress, registervalue"


#define DB_SET_REGISTER "REPLACE INTO arccore_cal (registervalue, registeraddress, writeable) \
                VALUES (%d, %d, %d)"

#define DB_UPDATE_REGISTER "UPDATE arccore_cal SET registervalue = %d WHERE registeraddress = %d %s"

#define DB_UPDATE_NO_FORCE_REGISTER " AND writeable = 1"

#define DB_INC_REGISTER "UPDATE arccore_cal SET registervalue = registervalue + %d WHERE registeraddress = %d %s"

#define DB_TOUCH_REGISTER "UPDATE arccore_cal SET sensortimestamp = time('now') WHERE registeraddress = %d %s"


// Log function IMPROVEMENT

// CREATE TABLE "mlog" ("registeraddress" INTEGER NOT NULL ,"registervalue" INTEGER NOT NULL ,"tagname" TEXT,"sensortimestamp" DATETIME DEFAULT ((datetime('now','localtime'))) )

//

extern int debug; // debug is set in main program.

int db_init ( char *filename );

int db_get_register ( int register_address, int number_of_registers, 
            int (*callback)(char *, int,  char **, char **));

int callback_print_sql_result(char *NotUsed, int argc, char **argv, char **azColName);

int db_set_register ( int register_address, int register_value, int create_missing, int force_write, int writeable );

int db_inc_register ( int register_address, int register_inc_value, int create_missing, int force_write, int writeable );

int db_touch_register ( int register_address, int create_missing, int force_write, int writeable );

int db_close ( void );


#endif
