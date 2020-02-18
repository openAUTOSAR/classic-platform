
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

#include <setjmp.h>
#ifndef _WIN32
#include <unistd.h> /* sleep */
#include <stdlib.h> /* exit */
#include <ctype.h>  /* isprint */
#include <errno.h>
#include <signal.h>
#include <fcntl.h>  /* open */
#include <asm/param.h> /* HZ */
#endif

#include "linos_logger.h" /* Logger functions */

#include "linos_time.h"

#include <time.h>
#include <sys/stat.h> // umask

#include "version.h"
#include "EcuM.h"

#include <os_i.h>    /* GEN_TASK_HEAD  */
#include <os_sys.h>  /* OS_SYS_PTR */

#include "LinuxOs_StbM.h"

#include "os_main.h"

#if defined(USE_LINOS_DB) && !defined(_WIN32)
  #include "database.h"
#endif

#include "../pmc_sync_status_reader/pmc_sync_status_reader.h"

#include <math.h>

#define NSEC_PER_SEC    1000000000

/* Global variables */

int daemonise = 1;

extern const OsTaskConstType  Os_TaskConstList[OS_TASK_CNT];

#if defined(ANDROID_NDK)
#define F_TLOCK		LOCK_EX
#define F_LOCK		LOCK_EX
#define FULOCK		LOCK_UN
int lockf(int fd, int cmd, off_t ignored_len) {
	return flock(fd, cmd);
}
#endif


#if defined(USE_TCPIP)
extern int gnulinuxTcpBindAny;
#endif

ThreadTaskType ThreadTasks[(OS_TASK_CNT+GNULINUX_TASK_CNT)]; /* Normal tasks and special GNULinux tasks */

int loops = 999; /* Infinate loops */

int DebugMask = 0; /* IMPROVEMENT SimonG Use this mask for debugging */

char *progname = NULL ; // Gets set in main(). Portable implementation of GNUs program_invocation_short_name

char pid_file_name[64]; // /var/run/X.pid

char bit_event_file_name[64]; // File to store the event bit signal in.

#if defined(USE_LINOS_DB)
char db_file_name[64];
#endif

/*     */
int stop_at_runtime = 0;

extern void EcuM_Init(void);

/*
 * Normalise timespec
 *
 * If tv_nsec >= NSEC_PER_SEC then
 * add to tv_sec and reduce tv_nsec
 * until tv_nsec < NSEC_PER_SEC
 *
*/
void tsnorm(struct timespec *ts)  {
#ifndef _WIN32
    while (ts->tv_nsec >= NSEC_PER_SEC)  {
        ts->tv_nsec -= NSEC_PER_SEC;
        ts->tv_sec++;
    }
#endif
}

#ifndef _WIN32
/* Used by WaitEvent in os_event.c */
struct timespec timesdiff(struct timespec *start, struct timespec *end) {

    struct timespec temp;
    if ((end->tv_nsec-start->tv_nsec)<0) {
        temp.tv_sec = end->tv_sec-start->tv_sec-1;
        temp.tv_nsec = 1000000000+end->tv_nsec-start->tv_nsec;
    } else {
        temp.tv_sec = end->tv_sec-start->tv_sec;
        temp.tv_nsec = end->tv_nsec-start->tv_nsec;
    }
    return temp;
}
#endif

/*
 *
 *
 *  gnulinux_timer (Runs in seperate thread)
 *
 *  Increment the counter OsTickFreq times per second. Max 1000Hz (1ms) ??
 *
 *  This sleeps different amounts of time to ensure accuracy.
 *
 *
*/

void gnulinux_timer(void) {
#ifndef _WIN32
    unsigned int msec = (1000/OsTickFreq); /* How long to wait for every tick */
    unsigned int usecs = msec * 1000; /* Wait time in microseconds */

    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (usecs * 1000);

    struct timespec r;

    while(1) {
        while (clock_nanosleep(CLOCK_MONOTONIC, 0, &t, &r) != 0) {
            if (errno == EINTR) {
                t.tv_sec = r.tv_sec;
                t.tv_nsec = r.tv_nsec;
            } else {
                /* Exiting on permanent errors. */
                logger(LOG_ERR, "vECU exiting on gnulinux_timer permanent sleep error.");
                // Improvement: do cleanup
                exit(1);
            }
        }
        IncrementCounter(0);
        t.tv_sec = 0;
        t.tv_nsec = (usecs * 1000);
    }
#endif
}

void gnulinux_pmc_sync_status_reader(void) {
#ifndef _WIN32
    if (!init_sync_status_reader()) {
        logger(LOG_ERR, "failed to initialize pmc_sync_status_reader");
        return;
    }
    const size_t sync_status_len = (size_t)sizeof(collective_sync_status);
    collective_sync_status recv_sync_status;
    sync_status_reader_conn_state conn_state;
    int recv_len = sync_status_len;

    while (1) {
        conn_state = connect_sync_status_reader();
        if(conn_state == SYNC_STATUS_PERM_DISCONNECTED) {
            logger(LOG_DEBUG, "permanently disconnected pmc_sync_status_reader");
            return;
        } else if (conn_state == SYNC_STATUS_CONNECTED) {
            while (1) {
                /* Design note:
                 * ------------
                 * recv will anyway fail if the socket is closed.
                 * Calling disable_gnulinux_pmc_sync_status_reader makes use of
                 * this by calling disconnect_sync_status_reader(1) that
                 * sets conn_state to SYNC_STATUS_PERM_DISCONNECTED and never
                 * allows setting it to SYNC_STATUS_TEMP_DISCONNECTED.
                 */
                recv_len = recv(reader_uds_skt, &recv_sync_status,
                        sync_status_len, 0);
                if (recv_len == sync_status_len) {
                    logger(LOG_DEBUG, "ptp (in-sync, offset): (%d, %f)",
                            recv_sync_status.in_sync,
                            recv_sync_status.offsetFromMaster);
                    Linos_StbM_BusSetGlobalTime(0, NULL, NULL, 0,
                            recv_sync_status.in_sync,
                            recv_sync_status.offsetFromMaster);
                } else {
                    logger(LOG_ERR,
                            "receiving error or partially sent sync_status");
                    break;
                }
            }
            disconnect_sync_status_reader(0);
        }
        /* Avoid high CPU consumption, sleep before attempting to reconnect. */
        sleep(1);
    }
#endif
}

void disable_gnulinux_pmc_sync_status_reader() {
    /* Design note:
     * ------------
     * Because gnulinux_pmc_sync_status_reader is implemented as a linos Task
     * on top of a pthread, we can not directly use pthread_cancel and
     * associated cleanup handlers. So we attempt closing the connection
     * whenever there is a possibility for leaving it open.
     * The possibility here is that gnulinux_pmc_sync_status_reader is blocked
     * on recv or is sleeping. */
    disconnect_sync_status_reader(1);
}

/*
 *
 *
 *
 *
 *     MAIN
 *
 *
 *
 *
 *
*/

/* All tasks will get to this function at least once.
 *
 * If Alarm type is "ALARM_ACTION_ACTIVATETASK" the thread will spin in this
 * loop until "pthread_status" get changed to GNULINUX_PTHREAD_FUNCTION_STARTED.
 * Mutex is required as "gnulinux_timer" thread will change the "pthread_status" to
 * "GNULINUX_PTHREAD_FUNCTION_STARTED" which means we need to protect the read.
 *
 * If Alarm type is "ALARM_ACTION_SETEVENT" the thread will jump to the function pointed to
 * by "entry()" and it will never return. The generated Rte implements the loop that handles
 * the WaitEvent, GetEvent and ClearEvent. The SetEvent is handled by gnulinux_timer thread.
 *
 * For non Autosar tasks (gnulinux_timer, gnulinux_pmc_sync_status_reader) the behavior is
 * implementation specific.
 * */
void TaskWrapper(ThreadTaskType *threadtask) {
    if ( NULL == threadtask ) {
        logger(LOG_ERR, "Pointer to thread variable is NULL in TaskWrapper, exiting thread.");
        pthread_exit(NULL);
    }

    while(1) {
        pthread_mutex_lock(&threadtask->mutex_lock);
        int thread_is_started = (threadtask->pthread_status == GNULINUX_PTHREAD_FUNCTION_STARTED);
        pthread_mutex_unlock(&threadtask->mutex_lock);
        if (thread_is_started) {
#ifndef _WIN32
        	threadtask->entry(); /* Call the function defined in the task entry */
#endif
        } else {
            //logger(LOG_INFO, "TaskWrapper waiting to re-activate task");
            usleep(1000); /* Sleep 1 ms */
        }
    }
}

/* All tasks will get to this function at least once.
 *
 * This is the first function called when threads are created. This function
 * checks that the task is meant to be started. If this is the case it will
 * change the thread status to GNULINUX_PTHREAD_FUNCTION_STARTED and then jump
 * to TaskWrapper, TaskWrapper will never return.
 *
 * We need to have protection on pthread_status because if this thread is delay
 * and gnulinux_timer thread is started there may be unsynced read/writes to this pthread_status.
 * */
void* StartTask(void *arg) {
#ifndef _WIN32
	ThreadTaskType *threadtask;

	if ( NULL == arg ) {
		logger(LOG_ERR, "Pointer to thread variable is NULL in StartTask, exiting thread.");
		pthread_exit(NULL);
	}

    threadtask = (ThreadTaskType *)arg;

    if (&ThreadTasks[TASK_ID_OsIdle] == threadtask) {
        logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_MAIN), LOG_INFO, "NOT Calling %s TaskId(%d)", threadtask->name, TASK_ID_OsIdle);
    }
    else {
        pthread_mutex_lock(&threadtask->mutex_lock);
        int thread_is_starting = (threadtask->pthread_status == GNULINUX_PTHREAD_STARTING);
        if (thread_is_starting) {
            threadtask->pthread_status = GNULINUX_PTHREAD_FUNCTION_STARTED;
        }
        pthread_mutex_unlock(&threadtask->mutex_lock);

        if (thread_is_starting) {
            logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_MAIN), LOG_INFO, "Calling %s", threadtask->name);
            TaskWrapper(threadtask); //Will never return
        }
        else {
            logger(LOG_ERR, "Task %s is NOT in state GNULINUX_PTHREAD_STARTING", threadtask->name);
            pthread_exit(NULL);
            return NULL;
        }
    }
#endif
    return NULL;
}


void list_all_events() {

    int i;
    logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_MAIN), LOG_INFO, "Listing all task events");

    for( i=0; i < OS_TASK_CNT; i++) {
        logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_MAIN), LOG_INFO, "IMPROVEMENT %s", ThreadTasks[i].name);
    }
}


#if defined(USE_LINOS_DB) && !defined(_WIN32)
int callback_print_runtime (char *reg_type, int argc, char **argv, char **azColName) {
    int i; 
    int address;
    int value;

    /* Loop through the columns in the DB record. */
    for(i=0; i<argc; i++) {
        //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if ( strcmp (azColName[i],"registeraddress") == 0 ) {
            address = atoi(argv[i]);
        }
        else if ( strcmp (azColName[i],"registervalue") == 0 ) {
            value = atoi(argv[i]);
        }
    }

    if ( address == DB_ECU_RUNTIME_REG_ADDRESS ) {
        logger(LOG_INFO, "Runtime at start-up is %ld s", value);
    }
    return 0;
}
#endif

static void usage() {

    printf("%s\n",progname);
    printf("%s\n", ArcticCore_Version.info);
    printf("USAGE: %s  \n", progname);
#if defined(USE_TCPIP)
    printf("-b bind (listen) to INADDR_ANY instead of the configured ip address\n");
#endif
#if defined(USE_LINOS_DB)
    printf("Database file is here %s\n", db_file_name);
#endif
#ifdef USE_SYSLOG
    printf("-f stay in foreground. Instead of deamonising.\n");
#endif
    printf("-l loops (999 = infinite).\n");
    printf("-m <BITMASK> to set the debug mask. Only three digit hex, e.g. 0xFFF. See logger.h\n");
    printf("-r stop at runtime. For test.\n");
}


int settings(int argc, char **argv ) {
    int index;
    int c;
#ifndef _WIN32
    opterr = 0;
#endif
        // See usage() function above
    while ((c = getopt (argc, argv, "bfhl:m:rt")) != -1) {
        switch (c) {
            case 'b':
#if defined(USE_TCPIP)
                gnulinuxTcpBindAny = TRUE;
#endif
                break;

            case 'l':
#ifndef _WIN32
                loops = atoi(optarg);
#endif
                break;

            case 'm':
#ifndef _WIN32
                //DebugMask = atoi(optarg);
                DebugMask = strtol(optarg, (char **) NULL, 16); // Accept a digit in decimal or hex
                printf("DebugMask = 0x%04X\n", DebugMask);
#endif
                break;

            case 'f':
#ifdef USE_SYSLOG
                logger_set_output(3); /* Syslog + fprintf */
#else
                logger_set_output(1); /* fprintf */
#endif
                daemonise = 0; /* Do NOT deamonise */
                loops = 10;
                break;

            case 'h':
                usage();
                exit(0);
                break;

            case 'r':
                stop_at_runtime = 1;
                break;


            case 't':
                EcuM_Init();
                list_all_events();
                exit(0);
                break;

            case '?':
#ifndef _WIN32
                if ((optopt == 'l')||(optopt == 'm')||(optopt == 'e')) {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                }
                else if (isprint (optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                }
                else {
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                }
#endif
                return 1;
            default:
                abort ();
        }
    }
#ifndef _WIN32
    for (index = optind; index < argc; index++) {
        fprintf(stderr, "Non-option argument %s", argv[index]);
        return 0;
    }
#endif
    return 0;
}


static void close_sigint(int exit_code) {

    logger(LOG_INFO, "Shutting down ... ");
    //logger(LOG_INFO, "Killing threads."); // IMPROVEMENT SimonG
    // pthread_cancel
    disable_gnulinux_pmc_sync_status_reader(); // Read Design note inside this method.

#if defined(USE_LINOS_DB) && !defined(_WIN32)
    db_close();
#endif

    unlink(pid_file_name);

    logger(LOG_INFO, "Done!");
#ifdef USE_SYSLOG
    logger_open_close ( 0, progname );
#endif
    exit(exit_code);
}


void read_event_file(void) {

    FILE *rc;
    char line[80];

    TaskType TaskId;
    EventMaskType Mask;

    logger(LOG_INFO, "SIG HUP caught. Reading events from file %s", bit_event_file_name);

    rc = fopen(bit_event_file_name, "r");
    if ( rc == NULL ) {
        logger(LOG_ERR, "Oops problems opening event file %s", bit_event_file_name);
    } 
    else {
        while(fgets(line, 80, rc) != NULL) {
            logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_MAIN), LOG_INFO, "Line : %s", line);
            TaskId = atoi(strtok (line,"-"));
            Mask = strtol(strtok (NULL, "\0"),NULL,0); // 0xabcdef00
            logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_MAIN), LOG_INFO, "SetEvent( TaskType %d, EventMaskType [0x%.8X] )",
                    TaskId, Mask);
            SetEvent( TaskId, Mask );
        }
        fclose(rc);
    }
}


void signal_handler(int sig) {
#ifndef _WIN32
    switch(sig) {
        case SIGHUP:
            read_event_file();
            break;

        case SIGTERM:
        case SIGINT:
            close_sigint(0);
            break;

        default:
            logger(LOG_ERR, "Unknown signal (%d) %s ignored.", sig, strsignal(sig));
    }
#endif
}

/* Shorten the task name to fit the Linux thread name limits of 15 characters.
 * See the documentation of pthread_setname_np.*/
#define SHORT_TASK_NAME_MAX 15
char buf_shortTaskName[SHORT_TASK_NAME_MAX];

/* "start_thread( int threadId )" handles the creation of threads and the
 * reactivation of tasks that have ALARM_ACTION_ACTIVATETASK configured.
 *
 * main()->init_threads()->start_thread()
 * Call occur at system startup and will start all Autostart-tasks. Mutex
 * is not needed in this case as there is not other task created that want
 * access to "pthread_status".
 *
 * OsStartupTask()->ActivateTask()->start_thread()
 * OsStartupTask is an Autostart-tasks and will activate the
 * the Autosar tasks that are not specified as Autostart-tasks. Mutex
 * is not needed in this case as there is not other task created that want
 * access to "pthread_status".
 *
 * gnulinux_timer()->IncrementCounter()->Os_AlarmCheck()->ActivateTask()->start_thread()
 * If an Autosar task has the ALARM_ACTION_ACTIVATETASK specified this function
 * will be called from gnulinux_timer thread to restart the task. Mutex is needed in this
 * case as the thread being reactivated may read or write to "pthread_status".
 */
int start_thread( int threadId ) {
#ifndef _WIN32
    int err;
    pthread_attr_t attr;

    /*------------------------------------Restart Terminated Thread------------------------------------*/
    /*
     * This will handle the restart of tasks that have ALARM_ACTION_ACTIVATETASK specified.
     */
    pthread_mutex_lock(&ThreadTasks[threadId].mutex_lock);
    int thread_is_terminated = (ThreadTasks[threadId].pthread_status == GNULINUX_PTHREAD_FUNCTION_TERMINATED);
    if (thread_is_terminated) {
    	ThreadTasks[threadId].pthread_status = GNULINUX_PTHREAD_FUNCTION_STARTED;
    }
    pthread_mutex_unlock(&ThreadTasks[threadId].mutex_lock);

    if (thread_is_terminated)
    {
    	err = E_OK;
        logger_mod((LOGGER_MOD_LINOS | LOGGER_SUB_OS_TASK), LOG_DEBUG,
                    "Restarting a terminated task (%s).",
                    ThreadTasks[threadId].name);

        return err;
    }


    /*-----------------------------Thread still running and not Terminated-----------------------------*/
    /* We can not re-create a task that is not empty and passed the above check. */
    pthread_mutex_lock(&ThreadTasks[threadId].mutex_lock);
	int thread_is_empty = (ThreadTasks[threadId].pthread_status == GNULINUX_PTHREAD_EMPTY);
	pthread_mutex_unlock(&ThreadTasks[threadId].mutex_lock);
    if (!thread_is_empty) {
        err = E_NOT_OK;
        /* This is a thread that has not terminated OR never started.
         * We do not log anything here to avoid noise in the logs. */
        return err;
    }

    /*-----------------------------------------Create Thread-----------------------------------------*/
    /* We will only get to this part if the thread was never created.
     * This means that we don't need mutex as there is no other thread
     * that will try to read or write pthread_status.
     */
    ThreadTasks[threadId].pthread_status = GNULINUX_PTHREAD_STARTING;

    /* Initialize thread creation attributes */

    err = pthread_attr_init(&attr);
    if (err != 0) {
        handle_error_en(err, "pthread_attr_init");
    }

// IMPROVEMENT Set prio, affinity etc.
//        if (stack_size > 0) {
//            err = pthread_attr_setstacksize(&attr, stack_size);
//            if (err != 0)
//            handle_error_en(err, "pthread_attr_setstacksize");
//    }
//
//    sched_setaffinity( etc.
//

    err = pthread_create((pthread_t * restrict)&(ThreadTasks[threadId].tid), NULL, &StartTask, &ThreadTasks[threadId]);

    if (err != 0) {
        logger(LOG_ERR, "\nFailed to create thread for task (%s), error :[%s]",
                ThreadTasks[threadId].name, strerror(err));
        pthread_attr_destroy(&attr);
        handle_error_en(err, "pthread_create");
    }

    /* Shorten the task name to fit the Linux thread name limits of 15 characters.
     * See the documentation of pthread_setname_np.*/
    int len_shortTaskName = SHORT_TASK_NAME_MAX;
    if (strlen(ThreadTasks[threadId].name) < SHORT_TASK_NAME_MAX) {
        len_shortTaskName = strlen(ThreadTasks[threadId].name);
    }
    memset(buf_shortTaskName, ' ', SHORT_TASK_NAME_MAX);
    strncpy(buf_shortTaskName, ThreadTasks[threadId].name, len_shortTaskName);

    /* Not POSIX, but needed for debugging. */
    err = pthread_setname_np(ThreadTasks[threadId].tid, (const char *)buf_shortTaskName);
    if (err != 0) {
        logger(LOG_ERR,
                "\nFailed pthread_setname_np for task (%s), error :[%s]",
                ThreadTasks[threadId].name, strerror(err));
    }

    /* Destroy the thread attributes object, since it is no longer needed */
    err = pthread_attr_destroy(&attr);
    if (err != 0) {
        logger(LOG_ERR,
                "\nFailed pthread_attr_destroy for task (%s), error :[%s]",
                ThreadTasks[threadId].name, strerror(err));
    }
    return err;
#else
    return 0;
#endif
}


void init_threads() {

    logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_MAIN), LOG_INFO, "Initializing threads ...");

    int i;

    /* First, mutexes and locks are initialized before any thread is running. */
    for (i=0; i< (OS_TASK_CNT+GNULINUX_TASK_CNT);i++) {
        /* Initialise the Wait/Set event pthread lock */
#ifndef _WIN32
        pthread_mutex_init((pthread_mutex_t *) &ThreadTasks[i].mutex_lock, NULL);
        pthread_cond_init((pthread_cond_t *) &ThreadTasks[i].cond, NULL);
#endif
    }

    /* Second, start autostart threads. */
    for (i=0; i< (OS_TASK_CNT+GNULINUX_TASK_CNT);i++) {

        if ( ThreadTasks[i].autostart == TRUE ) {
            start_thread(i);
        }
    }
}


int demonise(void) {
    // Daemonise

    int i;
    int rc = 0;
#ifndef _WIN32

    // Our process ID and Session ID
    pid_t pid, sid;
    char str[10]; // PID string for /var/run

    logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_MAIN), LOG_INFO, "starting the daemonizing process");

    // Open the PID file
    rc = open(pid_file_name,O_RDWR|O_CREAT|O_EXCL,0640);
    if ( rc < 0 ) {
        logger(LOG_ERR, "Oops problems opening pid file %s errno: %d (%s)",
                pid_file_name, errno, strerror(errno));
        exit(EXIT_FAILURE); /* Do NOT remove the PID file */
    }

    // Fork off the parent process
    pid = fork();
    if (pid < 0) {
        logger(LOG_ERR, "Problems forking the parent process.");
        exit(EXIT_FAILURE); // ERROR
    }
    // If we got a good PID, then
    // we can exit the parent process.
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Change the file mode mask
    umask(0);

    // Create a new SID for the child process
    sid = setsid();
    if (sid < 0) {
        // Log the failure
        logger(LOG_ERR, "Problems creating SID for the child process.");
        close_sigint(EXIT_FAILURE);
    }

    // Change the current working directory
    if ((chdir("/")) < 0) {
        // Log the failure
        logger(LOG_ERR, "Problems changing the current working directory.");
        close_sigint(EXIT_FAILURE);
    }
    else {
        logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_MAIN), LOG_INFO, "Daemonised !!");
    }


    // Close out the standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    i=open("/dev/null",O_RDWR); /* open stdin */
    dup(i); /* stdout */
    dup(i); /* stderr */
    //

    // Save the PID
    // Lock to prevent multiple processes
    if (lockf(rc,F_TLOCK,0)<0) {
        logger(LOG_ERR, "Oops problems locking pid file %s.", pid_file_name);
        close_sigint(EXIT_FAILURE);
    }

    sprintf(str,"%d\n",getpid());
    write(rc,str,strlen(str)); // record pid to lockfile
    logger(LOG_INFO, "PID run file is %s containing the PID %s", pid_file_name, str);
    // close(rc); // Dont close to hold the lock.

#endif
    return rc;
}





int main(int argc, char *argv[]) {
    int rc = 0;
    int running = 1;
#ifndef _WIN32
    struct timespec tp;
#endif

    //progname = argv[0]; // Get this programs name.
    progname =  __PROGNAME__; // Get this programs name. Defined in Makefile

#ifdef USE_SYSLOG
    logger_open_close ( 1, progname );
#endif

    /* Save the PID file here */
    //strcat(pid_file_name, "/var/run/");
    strcat(pid_file_name, "/tmp/");
    strcat(pid_file_name, progname);
    strcat(bit_event_file_name,pid_file_name); /* Put the event file in the same place as the pid file */
#if defined(USE_LINOS_DB)
    strcat(db_file_name,pid_file_name); /* Put the database file in the same place as the pid file */
    strcat(db_file_name, ".sqlite3");
#endif
    strcat(pid_file_name, ".pid");
    strcat(bit_event_file_name,BIT_EVENT_FILE_NAME_SUFFIX);
    logger(LOG_INFO, "Bit event file name is %s",bit_event_file_name);

    if (settings(argc, argv)) {
        usage();
        exit(1);
    }
#ifndef _WIN32
    logger(LOG_INFO,"Main. HZ is %ld CLOCKS_PER_SEC is %ld OsTickFreq is %ld", 
            HZ, CLOCKS_PER_SEC, OsTickFreq);
#endif
    logger(LOG_INFO,"%s", ArcticCore_Version.info);

    //OS_DEBUG(D_ALARM,"SIMONG TEST\n");

#ifndef _WIN32
    /* Catch these signals */
    signal(SIGHUP, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGQUIT, signal_handler);

    // Ignore these signals (deamonise)
    signal(SIGCHLD,SIG_IGN); /* child terminate signal */
    signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
#endif

    if (daemonise) {
       demonise();
    }
    else {
        logger(LOG_INFO, "No daemonising.");
    }

#if defined(USE_LINOS_DB) && !defined(_WIN32)
    if ( db_init (db_file_name) ) {
        logger(LOG_ERR,"Failed to initialize ArcCore CAL database '%s'", db_file_name);
        close_sigint(EXIT_FAILURE);
    }
    else {
        logger(LOG_INFO,"Opened ArcCore CAL database '%s'", db_file_name);
    }
#endif


    /* *************** START ********************** */

#if defined(USE_LINOS_DB) && !defined(_WIN32)
    rc = db_get_register ( DB_ECU_RUNTIME_REG_ADDRESS, 1, callback_print_runtime);

    if ( rc != DB_OK ) {
        logger(LOG_ERR, "ERROR: process_query DB ERROR: %d", rc);
    }
#endif
#ifndef _WIN32
    Linos_GetCurrentTime(&tp);

    logger(LOG_INFO,"Start-up @ clock real time:      %ld.%09ld or %s",
            tp.tv_sec, tp.tv_nsec, ctime(&tp.tv_sec));
#endif
    if (stop_at_runtime) {
        close_sigint(rc);
        return rc;
    }


    EcuM_Init(); /* This calls InitOS() */
    /* Improvement review if we can configure calling Linos_StbM_Init from EcuM module,
     * e.g. from EcuM_Init().
     * See example sequence diagram in 4.2.2/SWS_StbM. */
    Linos_StbM_Init();
    /* In addition to Tasks and Scheduler threads, we start a thread to read
     * PTP synchronization updates from a running pmc_sync_status daemon. */
    init_threads();

    /* THIS is the Main Thread */

    /* All this is just for test */
    int sleep_time_s = 1;
    while (running) {
        if ( loops != 999 ) {
            logger(LOG_INFO, "Loops left %d\n", loops);
            if ( loops > 1 ) {
                loops--;
            } else {
                running=0;
            }
        }
#ifndef _WIN32
        sleep(sleep_time_s);
#endif
#if defined(USE_LINOS_DB) && !defined(_WIN32)
        rc = db_inc_register (DB_ECU_RUNTIME_REG_ADDRESS, sleep_time_s, DB_CREATE_MISSING_WRITE, 1,1); // force_write, writeable
        if ( rc != DB_OK ) {
            logger(LOG_ERR, "DB_ECU_RUNTIME Problems saving runtime to DB.");
        }
#endif

        /* Linos_StbM_GetCurrentTime TEST */
        if ((DebugMask & (LOGGER_MOD_LINOS|LOGGER_SUB_OS_MAIN)) == (LOGGER_MOD_LINOS|LOGGER_SUB_OS_MAIN)) {
            StbM_TimeStampType testTime;
            StbM_UserDataType UserData;
            Std_ReturnType testReturn;
            uint8_t flag_timeout = 0, flag_global_time_base = 0;


            testReturn = Linos_StbM_GetCurrentTime(0, &testTime, &UserData);

            if ( testReturn != E_OK ) {
                logger(LOG_ERR, "os_main : testReturn is %d", testReturn);
            }
            else {
                if ( testTime.timeBaseStatus & 0x01 ) {
                    flag_timeout = 1;
                }

                if ( testTime.timeBaseStatus & 0x08 ) {
                    flag_global_time_base = 1;
                }

                logger(LOG_INFO, "os_main : sys TIME : %d.%d.%09d \tflag_timeout:%d\tflag_global_time_base:%d\n",
                        testTime.secondsHi, testTime.seconds, testTime.nanoseconds, flag_timeout, flag_global_time_base);
            }
        }
    }
    close_sigint(rc);
    return rc;
}
