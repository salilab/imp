#!/usr/bin/env python

import glob
import os.path
import sys

def update(module_path):
    examples = glob.glob(os.path.join(module_path, "examples", "*.readme"))
    for e in examples:
        base= os.path.splitext(e)[0]
        print base
        name= (glob.glob(base+".cpp")+ glob.glob(base+".py"))[0]
        contents= open(name, "r").read()
        out= open(name, "w")
        out.write("## \example "+name+"\n")
        for l in open(e, "r").read().split("\n"):
            out.write("## "+l+"\n")
        out.write("\n")
        out.write(contents)
        os.unlink(e)
def main():
    for m in sys.argv[1:]:
        update(m)

if __name__ == '__main__':
    main()
