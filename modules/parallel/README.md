This module allows for \imp tasks to be distributed to multiple processors
or machines. It employs a master-slave model; the main (master) \imp process
sends the tasks out to one or more slaves. Tasks cannot communicate with each
other, but return results to the master. The master can then start new tasks,
possibly using results returned from completed tasks. The system is fault
tolerant; if a slave fails, any tasks running on that slave are automatically
moved to another slave.

To use the module, first create a Manager object. Add one or
more slaves to the Manager using its
\link Manager::add_slave() add_slave()\endlink method (example slaves are
LocalSlave, which simply starts another \imp process on the
same machine as the master, and SGEQsubSlaveArray, which starts
an array of multiple slaves on a Sun GridEngine cluster). Next, call
Manager::get_context() method, which creates and returns a new Context object.
Add tasks to the Context with the Context::add_task() method (each task is
simply a Python function or other callable object). Finally, call
Context::get_results_unordered() to
send the tasks out to the slaves (a slave only runs a single task at a time;
if there are more tasks than slaves later tasks will be queued until a slave
is done with an earlier task). This method returns the results from each task
as it completes.

Setup in \imp is often expensive, and thus the Manager::get_context() method
allows you to specify a Python function or other callable object to do any
setup for the tasks. This function will be run on the slave before any tasks
from that context are started (the return values from this function are
passed to the task functions). If multiple tasks from the same context are
run on the same slave, the setup function is only called once.

<b>Troubleshooting</b>

Several common problems with this module are described below, together with
solutions.

 - <b>Master process fails with <tt>/bin/sh: qsub: command not found</tt>,
   but <tt>qsub</tt> works fine from a terminal.</b>\n
   SGEQsubSlaveArray uses the <tt>qsub</tt> command to submit the SGE job that
   starts the slaves. Thus, <tt>qsub</tt> must be in your system PATH. This may
   not be the case if you are using a shell script such as <tt>imppy.sh</tt>
   to start \imp. To fix this, modify the shell script to add the directory
   containing <tt>qsub</tt> to the PATH, or remove the setting of PATH entirely.

 - <b>The master process 'hangs' and does not do anything when
   Context::get_results_unordered() is called.</b>\n
   Usually this is because no slaves have successfully started up. Check the
   slave output files to determine what the problem is.

 - <b>%Slave output files contain only a Python traceback ending in
   <tt>ImportError: No module named IMP.parallel.slave_handler</tt>.</b>\n
   The slaves simply run 'python' and expect to be able to load in the \imp
   Python modules. If you need to run a modified version of Python, or usually
   prefix your Python command with a shell script such as <tt>imppy.sh</tt>,
   you need to tell the slaves to do that too. Specify the full command line
   needed to start a suitable Python interpreter as the 'python' argument when
   you create the Manager object.

 - <b>%Slave output files contain only a Python traceback ending in
   <tt>socket.error: (110, 'Connection timed out')</tt>.</b>\n
   The slaves need to connect to the machine running the master process
   over the network. This connection can fail (or time out) if that machine
   is firewalled. It can also fail if the master machine is multi-homed (a
   common setup for the headnode of a compute cluster). For a multi-homed
   master machine, use the 'host' argument when you create the Manager object
   to tell the slaves the name of the machine as visible to them (typically
   this is the name of the machine's internal network interface).

 - <b>%Slave output files contain only a Python traceback ending in
   <tt>socket.error: (111, 'Connection refused')</tt>.</b>\n
   If the master encounters an error and exits, it will no longer be around
   to accept connections from slaves, so they will get this error when they
   try to start up. Check the master log file for errors. Alternatively, the
   master may have simply finished all of its work and exited normally before
   the slave started (either the master had little work to do, or the slave
   took a very long time to start up). This is normal.

_Author(s)_: Ben Webb

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - See main IMP papers list.
