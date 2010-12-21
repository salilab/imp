#!/usr/bin/env python

from FileBasedGrid import FileBasedGrid
from hosts import create_host_list

hostlist = ['localhost']
tmpdir = '/tmp'
src_path = './'
showX11 = False
X11_delay = 1.0
grid_debug = True
grid_verbose = True
window_size = '80x25'

hosts= create_host_list(hostlist, tmpdir)
grid = FileBasedGrid(hosts,src_path,showX11,X11_delay,grid_debug,grid_verbose)
grid.shared_temp_path = True
if showX11:
    grid.window_size = window_size
#fork and execute grid.run() in a child
grid.start()

#if necessary use grid.copy_files(src, filenames) to copy files over.
#files in the local dir are already copied

#use grid.publish(stuff) to send stuff to all nodes. returns a service id.
class dummy(object):
    pass

service_id = grid.publish(dummy())


