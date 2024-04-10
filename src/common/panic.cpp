/*
 * Copyright (c) CERN 2013-2015
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "panic.h"
#include <cstring>
#include <execinfo.h>
#include <semaphore.h>
#include <signal.h>
#include <iostream>
#include <boost/thread.hpp>
#include <sys/prctl.h>

/*
 * This file contains the logic to handle signals, logging them and
 * killing the process.
 * Is it this complicated because the signal handler itself should do as little
 * as possible, and must be reentrant. Otherwise, deadlocks may occur.
 * Therefore, we limit the handle to set two flags, and let the logging and killing
 * happen in a separate thread, outside the signal handling logic.
 */

namespace fts3 {
namespace common {
namespace panic {


static sem_t semaphore;
static sig_atomic_t raised_signal = 0;
static void (*_arg_shutdown_callback)(int, void*);
static void *_arg_udata;

void *stack_backtrace[STACK_BACKTRACE_SIZE] = {0};
int stack_backtrace_size = 0;


// Safely convert signal number into string
void signal_number_to_string(int signum, char* buffer, size_t size) {
    int len = 0;

    // Convert signal number to string manually
    if (signum == 0) {
        buffer[len++] = '0';
    } else {
        int num = signum;
        while (num != 0 && len < (int)size - 1) {
            buffer[len++] = '0' + num % 10;
            num /= 10;
        }
    }

    // Null-terminate the string
    buffer[len] = '\0';

    // Reverse the string
    for (int i = 0, j = (int)len - 1; i < j; ++i, --j) {
        char temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }
}


void get_backtrace(int signum)
{
    stack_backtrace_size = backtrace(stack_backtrace, STACK_BACKTRACE_SIZE);

    char buffer[128];
    // Safely convert signal number to a string to be printed to stderr
    signal_number_to_string(signum, buffer, sizeof(buffer));

    // print out all the frames to stderr
    write(STDERR_FILENO, "Caught signal: ", strlen("Caught signal: "));
    write(STDERR_FILENO, buffer, strlen(buffer));
    write(STDERR_FILENO, "\nStack trace: \n", strlen("\nStack trace: \n"));
    backtrace_symbols_fd(stack_backtrace, stack_backtrace_size, STDERR_FILENO);
}


// Reset handler and re-raise, so the system handles it
// If ulimit -c is unlimited, a coredump will be generated for
// some of them (as SIGSEGV)
static void delegate_to_default(int signum)
{
    // Change working directory to a writeable directory!
    if (chdir("/tmp") < 0) {
        fprintf(stderr, "Failed to change working directory to /tmp (%d)", errno);
    }
    // setxid clears this flag, so need to reset to get the dump
    prctl(PR_SET_DUMPABLE, 1);
    // re-issue and let the system do the work
    signal(signum, SIG_DFL);
    raise(signum);
}


// Minimalistic logic inside a signal!
static void signal_handler(int signum)
{
    if (signum != raised_signal) {
        if (signum == SIGABRT ||
            signum == SIGSEGV ||
            signum ==  SIGILL ||
            signum ==  SIGFPE ||
            signum == SIGBUS ||
            signum ==  SIGTRAP ||
            signum ==  SIGSYS) {
                get_backtrace(signum);
        }
    }
    raised_signal = signum;
    // From man sem_post
    // sem_post() is async-signal-safe: it may be safely called within a signal handler.
    sem_post(&semaphore);

    switch (signum) {
        // Termination signals, do nothing, the installed callback should handle it
        case SIGINT: case SIGUSR1: case SIGTERM:
            break;
        // Let the system handle the rest
        default:
            sleep(30);
            delegate_to_default(signum);
            break;
    }
}


// Thread that logs, waits and kills
static void signal_watchdog(void)
{
    int r = 0;
    do
        {
            r = sem_wait(&semaphore);
        }
    while (r < 0);   // Semaphore may return spuriously with errno = EINTR
    _arg_shutdown_callback(raised_signal, _arg_udata);
}


// Set up the callbacks, and launch the watchdog thread
static void set_handlers(void)
{
    static const int CATCH_SIGNALS[] =
    {
        SIGABRT, SIGSEGV, SIGILL, SIGFPE,
        SIGBUS, SIGTRAP, SIGSYS,
        SIGQUIT, SIGINT, SIGUSR1, SIGTERM
    };
    static const size_t N_CATCH_SIGNALS = sizeof(CATCH_SIGNALS) / sizeof(int);
    static struct sigaction actions[N_CATCH_SIGNALS];

    sem_init(&semaphore, 0, 0);

    static sigset_t proc_mask;
    sigemptyset(&proc_mask);

    memset(actions, 0, sizeof(actions));
    for (size_t i = 0; i < N_CATCH_SIGNALS; ++i)
        {
            actions[i].sa_handler = &signal_handler;
            sigemptyset(&actions[i].sa_mask);
            actions[i].sa_flags = SA_RESTART;
            sigaction(CATCH_SIGNALS[i], &actions[i], NULL);
            sigaddset(&proc_mask, CATCH_SIGNALS[i]);
        }

    // Unblock signals (daemon may have blocked some of them)
    sigprocmask(SIG_UNBLOCK, &proc_mask, NULL);

    boost::thread watchdog(signal_watchdog);
}


// Wrap set_handlers, so it is called only once
void setup_signal_handlers(void (*shutdown_callback)(int, void*), void* udata)
{
    // First thing, wait for a signal to be caught
    static boost::once_flag set_handlers_flag = BOOST_ONCE_INIT;
    _arg_shutdown_callback = shutdown_callback;
    _arg_udata = udata;
    boost::call_once(&set_handlers, set_handlers_flag);
}


std::string stack_dump(void *array[], int stack_size)
{
    std::string stackTrace;

    char **symbols = backtrace_symbols(array, stack_size);
    for (int i = 0; i < stack_size; ++i) {
        if (symbols && symbols[i]) {
            stackTrace += std::string(symbols[i]) + '\n';
        }
    }
    if (symbols) {
        free(symbols);
    }

    return stackTrace;
}

} // end namespace panic
} // end namespace common
} // end namespace fts3
