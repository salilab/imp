#!/usr/bin/python
import sys
import re
import StringIO
import os
import glob

def handle_namespace(infile, outfile):
    #print "filtering ", sys.argv[1]
    txt= infile.read()
    lines = txt.splitlines()
    beg= re.compile("IMP[ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789]*_BEGIN_NAMESPACE")
    end= re.compile("IMP[ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789]*_END_NAMESPACE")
    export= re.compile("IMP[ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_]*EXPORT")
    merge=re.compile("\\\\\\\\$")
    for l in lines:
        #if l=="IMP_BEGIN_NAMESPACE":
        #    print  >> outfile, "#1"
        #    outfile.write("namespace IMP {\n")
        #elif l=="IMP_END_NAMESPACE":
        #    print  >> outfile, "#2"
        #    outfile.write("}\n")
        if beg.match(l):
            #print "match 0"
            #print  >> outfile, "#3"
            mname= l[3:l.find("_")]
            outfile.write("namespace IMP {\n")
            if mname!="":
                outfile.write("namespace "+ mname.lower()+ "{\n")
        elif end.match(l):
            #print "match 1"
            #print  >> outfile, "#4"
            outfile.write("}\n}\n")
        elif merge.search(l):
            #print>>sys.stderr, "merging ", l
            outfile.write(l)
        elif export.search(l):
            #print "match 2"
            #print  >> outfile, "#5"
            mo= export.search(l)
            #print "found", mo.start, mo.end
            outfile.write(l[0:mo.start()])
            outfile.write(l[mo.end():])
            outfile.write("\n")
        elif l.startswith("#include"):
            #print  >> outfile, "#6"
            pass
        else:
            #print  >> outfile, "#7"
            #print "HITHERE"
            outfile.write(l)
            outfile.write("\n")

def cleanup(infile):
    txt= infile.read()
    lines = txt.splitlines()
    for l in lines:
        if len(l)>0 and l[0]=="#":
            pass
        else:
            print l.replace("##", "").replace("#", "")

def main():
    txt= open(sys.argv[1])
    if sys.argv[1].endswith("macros.h"):
        print txt
    else:
        pns= StringIO.StringIO()
        defs= glob.glob('doc/doxygen/generated/*.def')
        macros=[]
        for d in defs:
            macros.append("-imacros")
            macros.append(d)
        handle_namespace(txt, pns)
        open("/tmp/pns.h", "w").write(pns.getvalue())
        command="cpp -C "+ " ".join(macros) +" /tmp/pns.h > /tmp/cppout"
        #print >> sys.stderr, command
        ret=os.system(command)
        if ret != 0:
            lns= pns.getvalue().split("\n")
            for i in range(0, len(lns)):
                print >>sys.stderr, i, lns[i]
        else:
            print "ok"
        cleanup(open("/tmp/cppout", "r"))

if __name__ == '__main__':
    main()
