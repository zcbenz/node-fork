#include "fork.h"

#include <unistd.h>
#include <errno.h>
#include <string.h>

namespace Fork {

Handle<Value> Fork (const Arguments& args) {
    HandleScope scope;

    pid_t pid = fork ();

    if (pid < 0) {
        return ThrowException (Exception::Error (
                    String::New (strerror (errno))));
    }

    if (pid == 0) {
        /*
         * Child process: We need to tell libev that we are forking because
         * kqueue can't deal with this gracefully.
         *
         * See: http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_fork_code_the_audacity_to_re
         */
        ev_default_fork();
    }

    return scope.Close (Integer::New (pid));
}

}
