#include "async.h"

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <map>
using std::map;

#include <ev.h>

namespace Fork {

static void child_watcher (EV_P_ ev_child *w, int revents);

// An Process instance
struct Process {
    Process (pid_t pid, int *pipefd) {
        ev_child_init (&cw, child_watcher, pid, 0);
        ev_child_start (EV_DEFAULT_ &cw);

        pipe[0] = pipefd[0];
        pipe[1] = pipefd[1];
    }

    int pipe[2];

    ev_child cw;
};

// Store one-to-one processes
map<int, Process*> childs;

Handle<Value> Async (const Arguments& args) {
    HandleScope scope;

    if ((args.Length () != 1 && args.Length () != 2)  || // 1 or 2 Arguments
        (args.Length () == 1 && !args[0]->IsFunction ()) || // (call)
        (args.Length () == 2 && !args[0]->IsFunction () && !args[1]->IsFunction ()) // (call, back)
       )
        return THROW_BAD_ARGS ;

    pid_t pid = fork ();
    if (pid < 0) {
        return ThrowException (Exception::Error (
                    String::New (strerror (errno))));
    }

    Handle<Function> call = Handle<Function>::Cast (args[0]);

    // Pipe for communicating
    int pipefd[2];
    if (pipe (pipefd) == -1) {
        return ThrowException (Exception::Error (
                    String::New (strerror (errno))));
    }

    if (pid == 0) {
        ev_default_fork();

        // Call the function in the child
        Handle<Value> result = call->Call (Object::New (), 0, NULL);

        // Send result to parent

        exit (0);
    } else {
        // Start listen in parent
        Process *process = new Process (pid, pipefd);
        childs[pid] = process;
    }

    return Undefined ();
}

static void child_watcher (EV_P_ ev_child *w, int revents) {
    // Must clean it
    Process *process = childs[w->rpid];
    childs.erase (w->rpid);
    delete process;

    ev_child_stop (EV_A_ w);
    fprintf (stderr, "process %d exited with status %x\n", w->rpid, w->rstatus);
}

}
