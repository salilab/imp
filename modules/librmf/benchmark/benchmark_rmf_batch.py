#!/usr/bin/python
import IMP.rmf
import RMF
import IMP.benchmark
import time

# don't bother with command line arguments, to keep in simple
file_name=IMP.benchmark.get_data_path("rnapii.rmf")
frame=0
verbose=True

# open the file, and don't clear the contents
rh= RMF.open_rmf_file(file_name);
nhs=[]
stack=[rh.get_root_node()]
while (len(stack)>0):
    cur= stack[-1]
    stack=stack[0:-1]
    nhs.append(cur)
    stack=stack+cur.get_children()
phys = rh.get_category("physics")
k= rh.get_float_key(phys, "cartesian x", False)

start=time.time()
all= RMF.get_values(nhs, k, 0)
tot=0.0
for a in all:
    tot=tot+a
elapsed= time.time()-start

IMP.benchmark.report("rmf access", "batch", elapsed, len(all))


start=time.time()
all=[]
for n in nhs:
    if n.get_has_value(k, 0):
        all.append(n.get_value(k, 0))
elapsed= time.time()-start

IMP.benchmark.report("rmf acess", "loop", elapsed, 0)
