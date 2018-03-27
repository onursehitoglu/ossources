# dining-mp.c#

Multiprocess dining philosophers. Not deadlock safe, uses SysV (XSI) semaphores

# dining-mp-noholdandwait.c #

Multiprocess dining philosophers. Deadlock safe, uses SysV (XSI) semaphore sets
to hold both or none of the chopsticks.


# dining-mt.c #

Multithread dining philosophers. Not deadlock safe.


# dining-mt-nocirc.c #

Multithread dining philosophers. Deadlock safe, the picking order  depends on
philosophers id (even or odd) which breaks the circular wait. (Knuth's solution)

# dining-mt-nopreempt.c #
Multithread dining philosophers. Deadlock safe. Second chopstick either tried
or timed-out, breaking preemption. If fails philosopher releases first chopstick
and tries again.

