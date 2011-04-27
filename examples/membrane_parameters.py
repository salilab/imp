import math

# various parameters for restraint are specified here
kappa_=1000.0
max_score_=0.01

# packing restraint
packing_nsig_=3
packing_ncl_=13

# scoring function
score_name_="dope_score.lib"

# other restraints
d0_inter_=8.0
diameter_=35.0
z_range_=[-3.5,3.5]
tilt_range_=[0,math.radians(45)]


def setup_protein():
# define TMH sequences
    seq0=("M","L","I","H","N","W","I","L","T","F","S","I","F","R","E","H","P","S","T","V","F","Q","I","F","T","K","C","I","L","V","S","S","S","F","L","L","F","Y","T","L","L","P","H","G","L","L","E","D","L","M","R","R","V","G","D","S","L","V","D","L","I","V","I","C","E","D","S","Q","G","Q","H","L","S","S","F","C","L","F","V","A","T","L","Q","S","P","F","S","A","G","V","S","G","L","C","K","A","I","L","L","P","S","K","Q","I","H","V","M","I","Q","S","V","D","L","S","I","G","I","T","N","S","L","T","N","E","Q","L","C","G","F","G","F","F","L","N","V","K","T","N","L","H","C","S","R","I","P","L","I","T","N","L","F","L","S","A","R","H","M","S","L","D","L","E","N","S","V","G","S","Y","H","P","R","M","I","W","S","V","T","W","Q","W","S","N","Q","V","P","A","F","G","E","T","S","L","G","F","G","M","F","Q","E","K","G","Q","R","H","Q","N","Y","E","F","P","C","R","C","I","G","T","C","G","R","G","S","V","Q","C","A","G","L","I","S","L","P","I","A","I","E","F","T","Y","Q","L","T","S","S","P","T","C","I","V","R","P","W","R","F","P","N","I","F","P","L","I","A","C","I","L","L","L","S","M","N","S","T","L","S","L","F","S","F","S","G","G","R","S","G","Y","V","L","M","L","S","S","K","Y","Q","D","S","F","T","S","K","T","R","N","K","R","E","N","S","I","F","F","L","G","L","N","T","F","T","D","F","R","H","T","I","N","G","P","I","S","P","L","M","R","S","L","T","R","S","T","V","E")

# TM regions
    TMH= [[24,48], [75,94], [220,238]]#, [254,276]]
# define the topology
    topo=[1.0,-1.0,1.0]#,-1.0]
# name of the TMH
    names=["TM0","TM1","TM2"]#,"TM3"]
# interacting pairs
    TM_inter=[["TM0","TM1"]]

# storing sequences of TMH
    seq=[]
    for h in TMH:
        tmp=[]
        for j in range(h[0],h[1]+1):
            tmp.append(seq0[j-1])
        seq.append(tmp)

    return seq,names,TMH,topo,TM_inter
