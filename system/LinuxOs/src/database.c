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

#include <stdio.h>
#include <string.h>
#include "database.h"
#include "linos_logger.h" // Logger function

#ifndef _WIN32
sqlite3 *db = 0;
#endif

int db_init ( char *filename )
{
#ifndef _WIN32
    char sql_cmd_create[] = DB_CREATE;
    char sql_cmd_index[] = DB_CREATE_INDEX;
    char sql_cmd_trigger[] = DB_CREATE_TRIGGER;
    char sql_cmd_check[] = DB_CHECK;


    char *reply = 0;


    if ( db )
        return DB_FAIL_EXISTS;

    if ( sqlite3_open ( filename, &db ) )
    {
        db = 0;
        return DB_FAIL_OPEN;
    }

    // Busy timeout. Waits a while on DB locked
    sqlite3_busy_timeout(db, DB_BUSY_TIMEOUT); // mS
    //

    // If modbus table missing, then create it.
    if ( sqlite3_exec ( db, sql_cmd_check, 0, 0, &reply ) != SQLITE_OK )
    {

        logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_DATABASE), LOG_INFO, "DB Creating arccore_cal DB in file %s\n",filename);

        if ( sqlite3_exec ( db, sql_cmd_create, 0, 0, &reply ) != SQLITE_OK )
        {
            logger(LOG_ERR, "ERROR: DB CREATE: Sqlite3: %s\n", reply);
            logger(LOG_ERR, "\n SQL was: %s\n", sql_cmd_create);
            sqlite3_close ( db );
            db = 0;
            return DB_FAIL_CREATE;
        }

        if ( sqlite3_exec ( db, sql_cmd_index, 0, 0, &reply ) != SQLITE_OK )
        {
            logger(LOG_ERR,"ERROR: DB create index Sqlite3: %s\n", reply);
            logger(LOG_ERR, "\n SQL was: %s\n", sql_cmd_index);
            sqlite3_close ( db );
            db = 0;
            return DB_FAIL_CREATE;
        }


        if ( sqlite3_exec ( db, sql_cmd_trigger, 0, 0, &reply ) != SQLITE_OK )
        {
            logger(LOG_ERR, "ERROR: DB create trigger Sqlite3: %s\n", reply);
            logger(LOG_ERR, "\n SQL was: %s\n", sql_cmd_trigger);
            sqlite3_close ( db );
            db = 0;
            return DB_FAIL_CREATE;
        }

    }
#endif
    return DB_OK;

} // END db_init


int callback_print_sql_result(char *NotUsed, int argc, char **argv, char **azColName){
    int i;
    NotUsed=0;

    logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_DATABASE), LOG_INFO, "callback_print_sql_result");
    for(i=0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}


int db_get_register ( int register_address, int number_of_registers,
            int (*callback)(char *, int,  char **, char **))
{
#ifndef _WIN32
  char sql_cmd_get_reg[255];
  char tmp_sql_cmd_get_reg[50];
  char *reply = NULL;
  int rc=0;

  if (!db)
    return DB_FAIL_EXISTS;

  tmp_sql_cmd_get_reg[0] = '\0';


  snprintf(sql_cmd_get_reg, sizeof(sql_cmd_get_reg), DB_GET_REGISTER,
        DB_GET_SELECT, register_address, register_address, number_of_registers, tmp_sql_cmd_get_reg);

  //logger(LOG_DEBUG, "SQL DB_GET_REGISTER [%s]\n", sql_cmd_get_reg);

  rc = sqlite3_exec(db, sql_cmd_get_reg, (sqlite3_callback) callback, NULL, &reply);

  if ( rc != SQLITE_OK )
    {
      logger(LOG_ERR, "DB GET REGISTER Sqlite3 error[%d]: %s\n", rc, reply);
      return DB_FAIL_SELECT;
    }
#endif
  return DB_OK;
}


/**
 *
 * db_touch_register
 *
 *
 * Updates the timestamp for a register in the database.
 *
 * PARAMS: int register_address, int create_missing=0, int force_write=0
 *
 * register_address ECU register address.
 *
 * register_value ECU register value.
 *
 * create_missing If = 1 then create missing registers, by INSERT (REPLACE)
 *
 * force_write IF = 1 then write to register even if writeable = 0 (readonly)
 *
*/

int db_touch_register ( int register_address, int create_missing, int force_write, int writeable )
{
#ifndef _WIN32
    char sql_cmd_touch_reg[255];
    char end_sql[25];
    char *reply = 0;

    int rc, num_of_rows;

    if (!db)
        return DB_FAIL_EXISTS;


    if ( force_write == 1 )
    {
        end_sql[0] = '\0';
    }
    else
    {
        strcpy( end_sql, DB_UPDATE_NO_FORCE_REGISTER);
    }

    snprintf(sql_cmd_touch_reg, 255, DB_TOUCH_REGISTER, register_address, end_sql);

    logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_DATABASE), LOG_DEBUG, "SQL DB_TOUCH_REGISTER [%s]\n", sql_cmd_touch_reg);

    rc = sqlite3_exec(db, sql_cmd_touch_reg, 0, 0, &reply);

    if ( rc != SQLITE_OK )
    {

        logger(LOG_ERR, "ERROR: DB TOUCH REGISTER Sqlite3 error[%d]: %s\n", rc, reply);
        return DB_FAIL_INSERT;
    }
    else
    {
        // Check that there was a row changed
        // otherwise  try SET (REPLACE)
        num_of_rows = sqlite3_changes(db);
        if ( num_of_rows != 1 )
        {
            if ( create_missing == 1 )
            {
                // Update didn't work try SET (REPLACE)
                snprintf(sql_cmd_touch_reg, 255, DB_SET_REGISTER, 0, register_address, writeable);
                rc = sqlite3_exec(db, sql_cmd_touch_reg, 0, 0, &reply);
                if ( rc != SQLITE_OK )
                {
                    // SET failed
                    logger(LOG_ERR, "ERROR: DB TOUCH REGISTER Sqlite3 error[%d]: %s\n", rc, reply);
                    return DB_FAIL_INSERT;
                }
            }
        }
    }
#endif
    return DB_OK;

} // END db_touch_register


/**
 *
 * db_inc_register
 *
 * Increment an ECU register value in the database.
 *
 * PARAMS: int register_address, int register_inc_value, int create_missing=0, int force_write=0
 *
 * register_address ECU register address.
 *
 * register_inc_value How much to increment the ECU register value.
 *
 * create_missing If = 1 then create missing registers, by INSERT (REPLACE)
 *
 * force_write IF = 1 then write to register even if writeable = 0 (readonly)
*/


int db_inc_register ( int register_address, int register_inc_value, int create_missing, int force_write, int writeable )
{
#ifndef _WIN32
  char sql_cmd_set_reg[255];
  char end_sql[25];
  char *reply = 0;

  int rc, num_of_rows;

  if (!db)
    return DB_FAIL_EXISTS;

  if ( force_write == 1 )
  {
    end_sql[0] = '\0';
  }
  else
  {
    strcpy( end_sql, DB_UPDATE_NO_FORCE_REGISTER);
  }

  snprintf(sql_cmd_set_reg, 255, DB_INC_REGISTER, register_inc_value, register_address, end_sql);

  //logger(LOG_DEBUG, "SQL DB_INC_REGISTER [%s]\n", sql_cmd_set_reg);

  rc = sqlite3_exec(db, sql_cmd_set_reg, 0, 0, &reply);

  if ( rc != SQLITE_OK )
  {

      logger(LOG_ERR, "ERROR: DB INC REGISTER Sqlite3 error[%d]: %s\n", rc, reply);
      return DB_FAIL_INSERT;
  }
  else
  {
    // Check that there was a row changed
    // otherwise  try SET (REPLACE)
    num_of_rows = sqlite3_changes(db);
    if ( num_of_rows != 1 )
    {
        if ( create_missing == 1 )
        {
            // Update didn't work try SET (REPLACE)
            snprintf(sql_cmd_set_reg, 255, DB_SET_REGISTER, register_inc_value, register_address, writeable);
            rc = sqlite3_exec(db, sql_cmd_set_reg, 0, 0, &reply);
            if ( rc != SQLITE_OK )
            {
                // SET failed
                logger(LOG_ERR, "ERROR: DB INC REGISTER Sqlite3 error[%d]: %s\n", rc, reply);
                return DB_FAIL_INSERT;
            }
          }
    }
  }
#endif
 return DB_OK;

}


/**
 *
 * db_set_register
 *
 * Set an ECU register value in the database.
 *
 * PARAMS: int register_address, int register_value, int create_missing=0, int force_write=0
 *
 * register_address ECU register address.
 *
 * register_value ECU register value.
 *
 * create_missing If = 1 then create missing registers, by INSERT (REPLACE)
 *
 * force_write IF = 1 then write to register even if writeable = 0 (readonly)
*/


int db_set_register ( int register_address, int register_value, int create_missing, int force_write, int writeable )
{
#ifndef _WIN32
    char sql_cmd_set_reg[255];
    char end_sql[25];
    char *reply = 0;

    int rc, num_of_rows;

    if (!db)
        return DB_FAIL_EXISTS;

    if ( force_write == 1 )
    {
        end_sql[0] = '\0';
    }
    else
    {
        strcpy( end_sql, DB_UPDATE_NO_FORCE_REGISTER);
    }

    snprintf(sql_cmd_set_reg, 255, DB_UPDATE_REGISTER, register_value, register_address, end_sql);

    logger(LOG_DEBUG, "SQL DB_SET_REGISTER [%s]\n", sql_cmd_set_reg);

    rc = sqlite3_exec(db, sql_cmd_set_reg, 0, 0, &reply);

    if ( rc != SQLITE_OK )
    {

        logger(LOG_ERR, "ERROR: DB SET REGISTER Sqlite3 error[%d]: %s\n", rc, reply);
        return DB_FAIL_INSERT;
    }
    else
    {
        // Check that there was a row changed
        // otherwise  try SET (REPLACE)
        num_of_rows = sqlite3_changes(db);
        if ( num_of_rows != 1 )
        {
            if ( create_missing == 1 )
            {
                // Update didn't work try SET (REPLACE)
                snprintf(sql_cmd_set_reg, 255, DB_SET_REGISTER, register_value, register_address, writeable);
                rc = sqlite3_exec(db, sql_cmd_set_reg, 0, 0, &reply);
                if ( rc != SQLITE_OK )
                {
                    // SET failed
                    logger(LOG_ERR, "ERROR: DB SET REGISTER Sqlite3 error[%d]: %s\n", rc, reply);
                    return DB_FAIL_INSERT;
                }
            }
        }
    }
#endif
    return DB_OK;

}




int db_close ( void )
{
#ifndef _WIN32
  if (!db)
    return DB_FAIL_EXISTS;

  sqlite3_close ( db );
  db = 0;
#endif
  return DB_OK;
}


