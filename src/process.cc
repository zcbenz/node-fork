#include "process.h"

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

namespace Fork {

using std::vector;

static ssize_t drain_read (int fd, vector<char> *to);
static void pipe_watcher (EV_P_ ev_io *w, int revents);

static Handle<Value> FromJsonString (const char *str, size_t length);
static Handle<String> ToJsonString (Handle<Value> value);

Process* Process::New (const Arguments& args) {
    HandleScope scope;

    // Pipe for communicating
    int pipefd[2];
    if (::pipe (pipefd) == -1) {
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

        // An object describing the result
        Handle<Object> ret = Object::New ();

        // Call the function in the child
        TryCatch try_catch;
        Handle<Value> result = call->Call (
                Context::GetCurrent()->Global(), 0, NULL);

        if (result.IsEmpty ()) {
            ret->Set (String::New ("data"), Undefined ());
        } else {
            ret->Set (String::New ("data"), result);
        }

        // Send result to parent
        Handle<String> json = ToJsonString (ret);
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

        return new Process (pid, pipefd[0], back);
    }
}

// An Process instance
Process::Process (pid_t pid, int fd, Handle<Function> back)
    : back (Persistent<Function>::New (back)),
      pid_ (pid),
      fd_ (fd)
{
    cw.data    = this;
    eread_.data = this;

    ev_io_init (&eread_, pipe_watcher, fd_, EV_READ);
    ev_io_start (EV_DEFAULT_ &eread_);
}

Process::~Process () {
    close (fd_);
    back.Dispose ();
    ev_io_stop (EV_DEFAULT_ &eread_);
}

Handle<Object> Process::End () {
    HandleScope scope;

    // Must make sure every byte is drained 
    Drain ();

    // Translate result to v8::Value
    Handle<Object> result = Handle<Object>::Cast (
        FromJsonString (&buffer_[0], buffer_.size ()));
    
    // Send to the `back` callback
    Handle<Value> args[1] = { result->Get (String::New ("data")) };
    back->Call (Context::GetCurrent()->Global(), 1, args);

    delete this;

    return scope.Close (result);
}

void Process::Drain () {
    while (drain_read (fd_, &buffer_) > 0)
        ;
}

static ssize_t drain_read (int fd, vector<char> *to) {
    const size_t SIZE = 256;
    char buffer[SIZE];
    ssize_t done = read (fd, buffer, 256);

    if (done > 0) {
        to->insert (to->end (),
                    static_cast<char*> (buffer),
                    static_cast<char*> (buffer) + done);
    }

    return done;
}

static void pipe_watcher (EV_P_ ev_io *w, int revents) {
    HandleScope scope;

    Process *process = static_cast<Process*> (w->data);

    // Read from pipe
    ssize_t done = drain_read (w->fd, &process->buffer_);

    if (done < 0) {
        ev_io_stop (EV_A_ w);
        perror ("read");
    } else if (done == 0) {
        ev_io_stop (EV_A_ w);
    }
}

static Handle<String> ToJsonString (Handle<Value> value) {
    HandleScope scope;

    Handle<Context> context = Context::New ();
    Context::Scope context_scope (context);
    context->Global()->Set (String::New ("s"), value);

    TryCatch try_catch;
    Handle<Script> script = Script::Compile (
            String::New ("JSON.stringify(s);"));
    // Compilation error
    if (script.IsEmpty ()) {
        return String::New ("undefined");
    }

    Handle<String> result = Handle<String>::Cast (script->Run());
    // Runtime error
    if (result.IsEmpty ()) {
        return String::New ("undefined");
    }

    return scope.Close (result);
}

static Handle<Value> FromJsonString (const char *str, size_t length) {
    HandleScope scope;

    // Can not parse `undefined`
    if (!strncmp (str, "undefined", length)) {
        return scope.Close (Undefined ());
    }

    // `JSON.parse` now
    Handle<Context> context = Context::New ();
    Context::Scope context_scope (context);
    context->Global()->Set (String::New ("s"), String::New (str, length));
    Handle<Script> script = Script::Compile (
            String::New ("JSON.parse(s);"));

    return scope.Close (script->Run ());
}

}
