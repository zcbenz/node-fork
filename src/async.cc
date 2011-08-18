#include "async.h"

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

namespace Fork {

using std::vector;

static void child_watcher (EV_P_ ev_child *w, int revents);
static void pipe_watcher (EV_P_ ev_io *w, int revents);

Process* BeforeAsync (const Arguments& args) {
    HandleScope scope;

    // Pipe for communicating
    int pipefd[2];
    if (pipe (pipefd) == -1) {
        return NULL;
    }

    pid_t pid = fork ();
    if (pid < 0) {
        return NULL;
    }

    Handle<Function> call = Handle<Function>::Cast (args[0]);

    if (pid == 0) {
        ev_default_fork();
        close (pipefd[0]);

        // Call the function in the child
        Handle<Value> result = call->Call (Object::New (), 0, NULL);

        // Send result to parent
        Handle<String> json = ToJsonString (result);
        String::AsciiValue str (json);
        ssize_t need = str.length ();
        while (need > 0) {
            ssize_t has  = str.length () - need;
            ssize_t done = write (pipefd[1], *str + (has), need);
            if (done < 0) {
                perror ("write");
                exit (1);
            } else if (done == 0) {
                break;
            } else {
                need -= done;
            }
        }

        close (pipefd[1]);
        exit (0);
    } else {
        close (pipefd[1]);

        // The callback function after child is ended
        Handle<Function> back = args.Length () == 1 ?
            FunctionTemplate::New ()->GetFunction () :
            Handle<Function>::Cast (args[1]);

        return new Process (pid, pipefd, back);
    }
}

Handle<Value> Async (const Arguments& args) {
    HandleScope scope;

    if ((args.Length () != 1 && args.Length () != 2)  || // 1 or 2 Arguments
        (args.Length () == 1 && !args[0]->IsFunction ()) || // (call)
        (args.Length () == 2 && !args[0]->IsFunction () && !args[1]->IsFunction ()) // (call, back)
       )
        return THROW_BAD_ARGS ;

    Process *process = BeforeAsync (args);
    if (!process)
        return ThrowException (String::New (strerror (errno)));

    // Monitor child
    ev_child_init (&process->cw, child_watcher, process->pid, 0);
    ev_child_start (EV_DEFAULT_ &process->cw);

    return scope.Close (Integer::New (process->pid));
}

Handle<String> ToJsonString (Handle<Value> value) {
    HandleScope scope;

    Handle<Context> context = Context::New ();
    Context::Scope context_scope (context);
    context->Global()->Set (String::New ("s"), value);
    Handle<Script> script = Script::Compile (
            String::New ("JSON.stringify(s);"));
    Handle<String> result = Handle<String>::Cast (script->Run());

    return scope.Close (result);
}

Handle<Value> FromJsonString (Handle<String> value) {
    HandleScope scope;

    Handle<Context> context = Context::New ();
    Context::Scope context_scope (context);
    context->Global()->Set (String::New ("s"), value);
    Handle<Script> script = Script::Compile (
            String::New ("JSON.parse(s);"));

    return scope.Close (script->Run ());
}

ssize_t drain_read (int fd, Process *process) {
    const size_t SIZE = 256;
    char buffer[SIZE];
    ssize_t done = read (fd, buffer, 256);

    if (done > 0) {
        process->buffer.insert (process->buffer.end (),
                                static_cast<char*> (buffer),
                                static_cast<char*> (buffer) + done);
    }

    return done;
}

static void pipe_watcher (EV_P_ ev_io *w, int revents) {
    HandleScope scope;

    Process *process = static_cast<Process*> (w->data);

    // Read from pipe
    ssize_t done = drain_read (w->fd, process);

    if (done < 0) {
        ev_io_stop (EV_A_ w);
        perror ("read");
    } else if (done == 0) {
        ev_io_stop (EV_A_ w);
    }
}

static void child_watcher (EV_P_ ev_child *w, int revents) {
    HandleScope scope;

    Process *process = static_cast<Process*> (w->data);

    // Must make sure every byte is drained 
    while (drain_read (process->pipe[0], process) > 0)
        ;

    // Translate result to v8::Value
    Handle<Value> result = FromJsonString (
        String::New (&process->buffer[0], process->buffer.size ()));

    Handle<Value> args[1] = { result };
    process->back->Call (Object::New (), 1, args);

    // Must clean it
    ev_child_stop (EV_A_ w);
    delete process;
}

// An Process instance
Process::Process (pid_t pid, int *pipefd, Handle<Function> back)
    : back (Persistent<Function>::New (back)),
      pid (pid)
{
    cw.data    = this;
    eread.data = this;

    pipe[0] = pipefd[0];
    pipe[1] = pipefd[1];

    ev_io_init (&eread, pipe_watcher, pipe[0], EV_READ);
    ev_io_start (EV_DEFAULT_ &eread);
}

Process::~Process () {
    close (pipe[0]);
    back.Dispose ();
    ev_io_stop (EV_DEFAULT_ &eread);
}

}
