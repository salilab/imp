import IMP.rmf
import IMP.benchmark
import datetime

# don't bother with command line arguments, to keep in simple
file_name=IMP.benchmark.get_data_path("rnapii.rmf")
frame=0
verbose=True

# open the file, and don't clear the contents
rh= IMP.rmf.open_rmf_file(file_name);
nhs=[]
stack=[rh]
while (len(stack)>0):
    cur= stack[-1]
    stack=stack[0:-1]
    nhs.append(cur)
    stack=stack+nhs.get_children()
k= rh.get_float_key(RMF.Physics, "x")

start=datetime.datetime.now()
all= RMF.get_values(nhs, k, 0)
elapsed= datetime.datetime.now()-start

IMP.benchmark.report("rmf batch", elapsed.total_seconds(), 0)


start=datetime.datetime.now()
all=[]
for n in nhs:
    all.append(n.get_value(k, 0))
elapsed= datetime.datetime.now()-start

IMP.benchmark.report("rmf loop", elapsed.total_seconds(), 0)
