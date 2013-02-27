#!/usr/bin/env python
import IMP.rmf
import RMF
import IMP.benchmark
import datetime

# don't bother with command line arguments, to keep in simple
file_name=IMP.benchmark.get_data_path("rnapii.rmf")
frame=0
verbose=True

# open the file, and don't clear the contents
rh= RMF.open_rmf_file(file_name);
nhs=[]
stack=[rh]
while (len(stack)>0):
    cur= stack[-1]
    stack=stack[0:-1]
    nhs.append(cur)
    stack=stack+cur.get_children()
k= rh.get_float_key(RMF.Physics, "cartesian x")

start=datetime.datetime.now()
all= RMF.get_values(nhs, k, 0)
tot=0.0
for a in all:
    tot=tot+a
elapsed= datetime.datetime.now()-start

IMP.benchmark.report("rmf access", "batch", elapsed.total_seconds(), len(all))


start=datetime.datetime.now()
all=[]
for n in nhs:
    if n.get_has_value(k, 0):
        all.append(n.get_value(k, 0))
elapsed= datetime.datetime.now()-start

IMP.benchmark.report("rmf acess", "loop", elapsed.total_seconds(), 0)
