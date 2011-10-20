#!/usr/bin/env python


if __name__ == '__main__':

    #retrieve number of replicas and replicanums list
    replicanums = []
    stepno = []
    fl=open('replicanums.txt')
    tokens=(fl.readline()).split()
    nreps = len(tokens)-1
    replicanums.append([int(i)-1 for i in tokens[1:]])
    stepno.append(int(tokens[0]))
    for line in fl:
        tokens=line.split()
        replicanums.append([int(i)-1 for i in tokens[1:]])
        stepno.append(int(tokens[0]))

    #demux p??_stats.txt files
    infiles = [open('r%02d_stats.txt' % i) for i in xrange(1,nreps+1)]
    outfiles = [open('p%02d_stats.txt' % i, 'w') for i in xrange(1,nreps+1)]
    for inf,outf in zip(infiles,outfiles):
        outf.write(inf.readline())
    lines = [fl.readline() for fl in infiles]
    for (no,rn) in zip(stepno,replicanums):
        while (lines[0] != '' and int(lines[0].split()[0]) == no):
            for i in xrange(nreps):
                outfiles[i].write(lines[rn[i]])
            lines = [fl.readline() for fl in infiles]
