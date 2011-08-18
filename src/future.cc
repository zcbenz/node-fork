#include "future.h"
#include "async.h"

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define THROW_BAD_PROCESS \
    ThrowException(String::New("Future has already been got"))

namespace Fork {

static Handle<Value> get (const Arguments& args);

Handle<Value> Future (const Arguments& args) {
    HandleScope scope;

    if (args.Length () == 1 && !args[0]->IsFunction ())
        return THROW_BAD_ARGS ;

    Process *process = BeforeAsync (args);
    if (!process)
        return ThrowException (Exception::Error (
                    String::New (strerror (errno))));

    Handle<ObjectTemplate> tpl = ObjectTemplate::New ();
    tpl->SetInternalFieldCount (1);

    Handle<Object> future = tpl->NewInstance ();
    future->SetPointerInInternalField (0, process);
    future->Set (String::New ("pid"), Integer::New (process->pid));
    future->Set (String::New ("get"), FunctionTemplate::New (get)->GetFunction ());

    // We won't wait for child until client calls future.get

    return scope.Close (future);
}

static Handle<Value> get (const Arguments& args) {
    HandleScope scope;

    Process *process = static_cast<Process*> (
            args.Holder ()->GetPointerFromInternalField (0));

    if (!process)
        return THROW_BAD_PROCESS;

    // Wait form child
    waitpid (process->pid, NULL, 0);

    // Must make sure every byte is drained 
    while (drain_read (process->pipe[0], process) > 0)
        ;

    // Translate result to v8::Value
    Handle<Value> result = FromJsonString (
        String::New (&process->buffer[0], process->buffer.size ()));

    // Must clean it
    delete process;
    args.Holder ()->SetPointerInInternalField (0, NULL);

    return scope.Close (result);
}

}
