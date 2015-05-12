#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#define RUNNINGDIR "."
#define LOGFILE "logs.txt"

/* Global variables */
int lockfile;

/* Headers */
void logIt(char *msg);
void closeDaemon();
void restartDaemon();
void daemonSignalsHandler(int sig);
void daemonize();

void logIt(char *msg){
    FILE *logfile;
    time_t ltime; /* calendar time */
    char timedate[26];
    ltime=time(NULL); /* get current cal time */
    strftime(timedate, 26, "%Y-%m-%d %H:%M:%S", localtime(&ltime));
    if ((logfile = fopen(LOGFILE, "a")) == NULL)
        exit(EXIT_FAILURE);
    fprintf(logfile, "[%s] %s\n", timedate, msg);
    fclose(logfile);
}

void closeDaemon(){
    logIt("[OK] Exiting...");
    close(lockfile);
    exit(EXIT_SUCCESS);
}

void restartDaemon(){

}

void daemonSignalsHandler(int sig){
    switch(sig) {
        case SIGHUP:
            /* We restart the daemon */
            logIt("[OK] Restarting...");
            restartDaemon();
            break;
        case SIGTERM:
            /* We stop the daemon */
            closeDaemon();
            exit(EXIT_SUCCESS);
            break;
        case SIGINT:
            /* Ctrl c : we do nothing */
            break;
        case SIGCHLD:
            /* Child terminates */
            break;
    }
}

void daemonize(){
    /* Our process ID and Session ID */
    pid_t pid, sid;
    /* lock file */
    char pidstr[10];
    /* Signals */
    sigset_t sig_proc;
    struct sigaction action;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        printf("[ERROR] Fork failed.\n");
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then we can exit the parent process. */
    if (pid > 0) {
        printf("[OK] Launching...\n");
        exit(EXIT_SUCCESS);
    }

    /* Change the file mode mask */
    umask(027);

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        /* Log the failure */
        printf("[ERROR] setsid() failed.\n");
        exit(EXIT_FAILURE);
    }

    /* Change the current working directory */
    if ((chdir(RUNNINGDIR)) < 0){
        printf("[ERROR] Cannot chdir to the running directory.\n");
        exit(EXIT_FAILURE);
    }

    /* Create pid file & lock it */
    lockfile=open("pid.lock",O_RDWR|O_CREAT,0640);
    if (lockfile < 0){
        printf("[ERROR] Cannot open logfile. Exiting...\n");
        logIt("[ERROR] Cannot open logfile. Exiting...");
        exit(EXIT_FAILURE);
    }
    if (lockf(lockfile,F_TLOCK,0 ) < 0){
        printf("[ERROR] The daemon seems to be launched already. Exiting...\n");
        logIt("[ERROR] The daemon seems to be launched already. Exiting...");
        exit(EXIT_FAILURE);
    }
    sprintf(pidstr, "%d\n", getpid());
    write(lockfile, pidstr, strlen(pidstr));

    /* Handle the signals */
    sigemptyset(&sig_proc);
    action.sa_mask=sig_proc;
    action.sa_flags=0;
    action.sa_handler = daemonSignalsHandler;
    sigaction(SIGINT, &action,0);
    sigaction(SIGTERM, &action,0);
    sigaction(SIGHUP, &action,0);
    sigaction(SIGCHLD, &action,0);

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    /* Daemon-specific initialization goes here */
    logIt("[OK] Initialized.");
}

int main(int argc, char *args[]){
    /* Checking arguments */
    if (argc != 2 || (strcmp("start", args[1]) && strcmp("stop", args[1])
                      && strcmp("restart", args[1]))){
        printf("[Help] ./daemon <start|stop|restart>");
        exit(EXIT_FAILURE);
    }

    /* Stopping the daemon */
    if (!strcmp("stop", args[1])){
        
    }

    /* Restarting the daemon */
    if (!strcmp("restart", args[1])){

    }

    /* Starting the daemon */
    if (!strcmp("start", args[1])){
        /* Daemonizing */
        daemonize();

        /* The Big Loop */
        while (1) {
            sleep(10);
            exit(EXIT_SUCCESS);
        }
    }

    /* Closing the daemon */
    closeDaemon();
    exit(EXIT_SUCCESS);
}
