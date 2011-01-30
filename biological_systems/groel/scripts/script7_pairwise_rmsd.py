import IMP.atom
import numpy

model_fn=[]
for i in range(1,11):
    model_fn.append("P0A6F5.B9999%(a)04d.pdb"%{'a':i})
mhs_xyz=[]
ca_sel=IMP.atom.CAlphaPDBSelector()
mdl=IMP.Model()
for fn in model_fn:
    mhs_xyz.append(IMP.core.XYZs(IMP.core.get_leaves(IMP.atom.read_pdb(fn,mdl,ca_sel))))
num_mols=len(mhs_xyz)
all_rmsd=[]
for i in range(num_mols):
    all_rmsd.append(numpy.zeros(num_mols))
for i in range(num_mols):
    for j in range(i+1,num_mols):
        rmsd=IMP.atom.get_rmsd(mhs_xyz[i],mhs_xyz[j])
        all_rmsd[i][j]=rmsd
        all_rmsd[j][i]=rmsd
#print header line
for i in range(num_mols+1):
    print '%(a)05d'%{'a':i},
print ""
#print rmsds
for i in range(num_mols):
    print '%(a)05d'%{'a':i+1},
    for j in range(num_mols):
        if i<j:
            k1=i; k2=j
        else:
            k1=j; k2=i;
        print '%(a).3f'%{'a':all_rmsd[k1][k2]},
    print ""
print ""
print ""
print "max rmsd:", max(numpy.reshape(all_rmsd,num_mols*num_mols))
for i in range(num_mols):
    all_rmsd[i][i]=1000
print "min rmsd:", min(numpy.reshape(all_rmsd,num_mols*num_mols))
