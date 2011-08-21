#include "async.h"
#include "process.h"

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

namespace Fork {

using std::vector;

static void child_watcher (EV_P_ ev_child *w, int revents);

Handle<Value> Async (const Arguments& args) {
    HandleScope scope;

    if ((args.Length () != 1 && args.Length () != 2)  || // 1 or 2 Arguments
        (args.Length () == 1 && !args[0]->IsFunction ()) || // (call)
        (args.Length () == 2 && !args[0]->IsFunction () && !args[1]->IsFunction ()) // (call, back)
       )
        return THROW_BAD_ARGS ;

    Process *process = Process::New (args);
    if (!process)
        return ThrowException (String::New (strerror (errno)));

    // Monitor child
    ev_child_init (&process->cw, child_watcher, process->Pid (), 0);
    ev_child_start (EV_DEFAULT_ &process->cw);

    return scope.Close (Integer::New (process->Pid ()));
}

static void child_watcher (EV_P_ ev_child *w, int revents) {
    HandleScope scope;

    // Stop child watcher
    ev_child_stop (EV_A_ w);

    Process *process = static_cast<Process*> (w->data);

    // Get result
    process->End ();
}

}
