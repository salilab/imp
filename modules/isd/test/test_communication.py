#!/usr/bin/env python


#general imports
from numpy import *
from random import uniform
import atexit
import os,sys


#imp general
import IMP

#unit testing framework
import IMP.test

#our project
from IMP.isd.FileBasedGrid import FileBasedGrid
from IMP.isd.hosts import create_host_list
import IMP.isd.shared_functions as sf

class Tests(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        #setup host list
        self.hostlist = ['localhost']
        self.tmpdir = '/tmp'
        self.src_path = os.path.dirname(IMP.isd.FileBasedGrid.__file__)
        self.showX11 = False
        self.X11_delay = 1.0
        self.grid_debug = False
        self.grid_verbose = False
        self.window_size = '80x25'
        self.hosts=create_host_list(self.hostlist, self.tmpdir)
        imppy=os.path.abspath(
        os.path.join(os.getenv('IMP_DATA'),'../../tools/imppy.sh'))
        for host in self.hosts:
            host.init_cmd = imppy + ' !'


    def create_grid(self):
        grid = FileBasedGrid(self.hosts,self.src_path,self.showX11,
                self.X11_delay,self.grid_debug,self.grid_verbose)
        grid.shared_temp_path = True
        if self.showX11:
            grid.window_size = self.window_size
        #register grid.terminate(), otherwise program doesn't quit
        #if abruptly terminated
        atexit.register(grid.terminate)
        return grid

    def test_hello(self):
        "test if grid can do simple tasks"
        self.skipTest("grid tests require specific machine setup")
        grid = self.create_grid()
        self.grid = grid
        #fork and execute grid.run() in a child
        grid.start()

        #if necessary use grid.copy_files(src, filenames) to copy files over.
        #files in the local dir are already copied

        grid.copy_files(self.src_path, ['shared_functions.py'])

        #use grid.publish(stuff) to send stuff to all nodes.
        #returns a service id.

        instance=sf.sfo_common()
        service_id = grid.publish(instance)

        proxy = grid.servers.values()[0][0].proxy
        retval = proxy.hello().get()
        self.assertEqual(retval, "hello world")

    def tearDown(self):
        if hasattr(self,'grid'):
            self.grid.terminate()
        IMP.test.TestCase.tearDown(self)

if __name__ == '__main__':
    IMP.test.main()
