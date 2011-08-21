#include "main.h"
#include <ev.h>
#include <vector>

namespace Fork {

Handle<Value> Async (const Arguments& args);

// An Process instance
struct Process {
    Process (pid_t pid, int *pipefd, Handle<Function> back, Handle<Object> holder);

    ~Process (); 

    Persistent<Function> back;
    Persistent<Object> holder;

    int pid;
    int pipe[2];

    ev_child cw;
    ev_io eread;

    std::vector<char> buffer;
};

Process* BeforeAsync (const Arguments& args);
ssize_t drain_read (int fd, Process *process);
Handle<String> ToJsonString (Handle<Value> value);
Handle<Value> FromJsonString (const char *str, size_t length);

}
