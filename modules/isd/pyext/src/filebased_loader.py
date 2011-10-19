##
## The Inferential Structure Determination (ISD) software library
##
## Authors: Michael Habeck and Wolfgang Rieping,
##
##          Copyright (C) Michael Habeck and Wolfgang Rieping
##
##          All rights reserved.
##
## NO WARRANTY. This library is provided 'as is' without warranty of any
## kind, express or implied, including, but not limited to the implied
## warranties of merchantability and fitness for a particular purpose or
## a warranty of non-infringement.
##
## Distribution of substantively modified versions of this module is
## prohibited without the explicit permission of the copyright holders.
##

if __name__ == '__main__':

    import os, sys, time, cPickle

    import IMP.isd.FileBasedGrid as FileBasedGrid
    import IMP.isd.logfile as logfile

    import shared_functions as sf

    log_output = True

    f = open(sys.argv[1])
    init_data = cPickle.load(f)
    f.close()

    temp_path = init_data['temp_path']

    os.environ['ISD_ROOT'] = temp_path

    if not temp_path in sys.path:
        sys.path.insert(0, temp_path)

    os.nice(init_data['niceness'])

    kill_on_error = not init_data['display']
    signal_file = '%s/kill' % temp_path

    parent_tid = init_data['parent_tid']
    tid = init_data['tid']
    debug = init_data['debug']

    print 'Tid=%d, nice=%d, debug=%s, temp=%s' % (tid, init_data['niceness'],
                                                  str(debug), temp_path)

    if log_output:
        filename = temp_path + '/%d.log' % tid
        log = logfile.logfile(filename)
        if debug:
            os.dup2(sys.stdout.fileno(),log.fileno())
        else:
            sys.stdout = log

    handler = FileBasedGrid.FileBasedRemoteObjectHandler(
        kill_on_error, signal_file, temp_path, parent_tid, tid,
        debug, init_data['nfs_care'])

    handler.start()
