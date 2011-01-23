from modeller import *

env = environ()
aln = alignment(env)
templates_file="build_profile.prf"
templates=[]
#here add fully coverting and !1oel
for line in open(templates_file):
    if line[0]=="#":
        continue
    s=line.split()
    if int(s[0])==1:
        continue
    if int(s[4])-int(s[3])>480:
        templates.append([s[1][:4],s[1][-1]])
    else:
        print "Not including:",s[0],s[1]
for (pdb, chain) in templates:
    print pdb
    m = model(env, file="data/templates/"+pdb, model_segment=('FIRST:'+chain, 'LAST:'+chain))
    aln.append_model(m, atom_files=pdb, align_codes=pdb+chain)
aln.malign()
aln.malign3d()
aln.compare_structures()
aln.id_table(matrix_file='family.mat')
env.dendrogram(matrix_file='family.mat', cluster_cut=-1.0)
