#include "main.h"
#include <vector>

namespace Fork {

// An Process instance
class Process {
    public:
        // Fork a new process
        static Process* New (const Arguments& args);

        // Delete process and get result
        Handle<Object> End ();

        // Make sure every byte in `fd` is read to buffer
        void Drain ();

        // Returns pid of the child
        int Pid () const { return pid_; }

        ev_child cw;
        std::vector<char> buffer_;

    private:
        Process (pid_t pid, int fd, Handle<Function> back);
        ~Process (); 

        Persistent<Function> back;

        int pid_;
        int fd_;

        ev_io eread_;
};

}
