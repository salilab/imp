##
## The Inferential Structure Determination (ISD) software library
##
## Authors: Michael Habeck and Wolfgang Rieping
##
##          Copyright (C) Michael Habeck and Wolfgang Rieping
##
##          All rights reserved.
##
## NO WARRANTY. This library is provided 'as is' without warranty of any
## kind, expressed or implied, including, but not limited to the implied
## warranties of merchantability and fitness for a particular purpose or
## a warranty of non-infringement.
##
## Distribution of substantively modified versions of this module is
## prohibited without the explicit permission of the copyright holders.
##

import os

def abspath(x):

    return os.path.abspath(os.path.expanduser(x))

def remote_mkdirs(host, folder, debug):

    exists = False

    command = "ssh %s '%s -c \"import os, sys; print %s, os.path.exists(os.path.abspath(os.path.expanduser(sys.argv[1]))) \" %s'"

    tag = '%s' % id(command)
    out = os.popen(command % (host.name, host.python, tag, folder))
    lines = out.readlines()

    #print lines

    for line in lines: ## find the line with output of python command
        if line.split()[0] == tag:
            exists = eval(line.split()[-1])
    out.close()

    if not exists:

        if debug:
            print 'Creating temporary directory %s on the host %s... ' % (folder, host.name)

        command = "ssh %s '%s -c \"import os, sys; os.makedirs(sys.argv[1])\" %s\' > /dev/null"
        os.system(command % (host.name, host.python, folder))

class Host(object):

    def __str__(self):

        s = '%s(name=%s, nice=%d, python=%s)'

        return s % (self.__class__.__name__, self.name, self.niceness, self.python)

    __repr__ = __str__

    def __init__(self, name, temp_path, n_cpu=1, niceness=0, python='python', init_cmd=''):

        self.name = name
        self.set_temp_path(temp_path)
        self.set_niceness(niceness)
        self.python = python
        self.isd_shared_lib_path = None
        self.n_cpu = n_cpu
        self.use_n_cpu = self.n_cpu
        self.init_cmd = init_cmd

    def set_temp_path(self, temp_path):

        self.temp_path = abspath(temp_path)

    def set_niceness(self, n):

        self.niceness = n

def create_host_list(host_names, temp_path, niceness=0, python='python', init_cmd=''):

    host_list = [Host(name, temp_path, niceness=niceness, python=python, init_cmd=init_cmd) for name in host_names]

    return host_list
