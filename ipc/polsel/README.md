# MULTIPLEXED IO EXAMPLE


## PROBLEM

Read multiple file descriptors in a single loop. If read
sequentially, blocking I/O will wait on a not ready
file descriptor, even other file descriptors are ready.

```C
for (i = 0; i < N; i++) {
	n = read(fd[i], buf, bufsize);
	... process buffer
}
```

## TEST CASE

A program `rythm.c` write messages on standard output
periodically. A typical command line

`./rythm 1000 3000 10 hello`

Waits for 1000 ms and then writes `hello` message.
It will periodically repeats the message
10 times with 3000 ms interval.


Main programs create N pipes and call fork/exec N times to
create N instances of `rythm` program with different
parameters. `stdout` of `rythm` processes go to
pipe write end. The main program needs to read `N`
pipes and output them on terminal.


## SOLUTIONS

### `blocking.c`

It is not a solution. It illustrates the blocking
I/O. It reads the pipe buffers in the loop order.
Pipe data arrives at arbitrary intervals but
program outputs them later when others unblock
and they get the turn eventually.

### `nonblock.c`

`fcntl(fd,SET_FD, O_NONBLOCK)` system call
makes a file descriptor non-blocking. `read()`
system call will return immediately either
with data or no data. For the later, it
returns -1 and sets the `errno` as `EAGAIN`.

This way, we can skip to next file descriptor 
without blocking. However, resulting loop
will continuously call `read()` making
a busy wait. It uses the CPU and makes
too many `read()` calls for data which
is not available. You can observe the
CPU usage with programs like `top` and/or
by calling it as `time ./nonblock`.


### `select.c`

`select()` calls take file descriptor sets
as arguments. It blocks untile at least one 
of the descriptors are ready to read (or write
in another set). It returns the ready set in
the same parameter. Then we can check if a file
descriptor is ready and read it. `read()` is
a blocking read but we make sure that there
is data thanks to `select()`.


### `poll.c`

`poll()` is similar to `select()`. The
paramters are different. It gets
an array of `pollfd` structures. Instead
of set based updates and checks, you
can check on array values.








