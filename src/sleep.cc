#include "fork.h"

#include <unistd.h>
#include <errno.h>
#include <string.h>

namespace Fork {

Handle<Value> Sleep (const Arguments& args) {
    HandleScope scope;

    if (args.Length () != 1 && !args[0]->IsNumber ()) {
        return THROW_BAD_ARGS;
    }

    unsigned need = args[0]->Uint32Value ();
    while (need > 0) {
        need = sleep (need);
    }

    return Undefined ();
}

}
