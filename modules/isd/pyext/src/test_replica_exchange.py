#!/usr/bin/env python

from FileBasedGrid import FileBasedGrid
from hosts import create_host_list
import atexit
import shared_functions as sf


hostlist = ['localhost']
tmpdir = '/tmp'
src_path = './'
showX11 = False
X11_delay = 1.0
grid_debug = False
grid_verbose = False
window_size = '80x25'

hosts= create_host_list(hostlist, tmpdir)
grid = FileBasedGrid(hosts,src_path,showX11,X11_delay,grid_debug,grid_verbose)
grid.shared_temp_path = True
if showX11:
    grid.window_size = window_size

#register grid.terminate(), otherwise program doesn't quit
atexit.register(grid.terminate)
#fork and execute grid.run() in a child
grid.start()

#if necessary use grid.copy_files(src, filenames) to copy files over.
#files in the local dir are already copied

grid.copy_files(src_path, ['shared_functions.py'])

#use grid.publish(stuff) to send stuff to all nodes. returns a service id.

instance=sf.sfo()
service_id = grid.publish(instance)

proxy = grid.servers.values()[0][0].proxy
retval = proxy.hello().get()
print retval
#make clean exit
exit()

