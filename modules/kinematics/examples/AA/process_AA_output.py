import numpy
import re
from matplotlib import pyplot as plt
from matplotlib.mlab import griddata
import math

#filein2 = open("AA_m1_pio180_2.dat", "r")


example = "RRT Step |  [3.08782,3.13196] [3.03564,3.12392] [2.98346,3.11588] [2.93128,3.10784] [2.87911,3.09979] [2.82693,3.09175] [2.77475,3.08371] [2.72257,3.07567] [2.67039,3.06763] [2.61821,3.05959] [2.56603,3.05155] [2.51385,3.04351] [2.46168,3.03547] [2.4095,3.02742] [2.35732,3.01938] [2.30514,3.01134] [2.25296,3.0033] [2.20078,2.99526] [2.1486,2.98722] [2.09642,2.97918] [2.04424,2.97114] [1.99207,2.9631] [1.93989,2.95505] [1.88771,2.94701] [1.83553,2.93897] [1.78335,2.93093] [1.73117,2.92289] [1.67899,2.91485] [1.62681,2.90681] [1.57464,2.89877] [1.52246,2.89073] [1.47028,2.88269] [1.4181,2.87464] [1.36592,2.8666] [1.31374,2.85856] [1.26156,2.85052] [1.20938,2.84248] [1.1572,2.83444] [1.10503,2.8264] [1.05285,2.81836] [1.00067,2.81032] [0.948489,2.80227] [0.89631,2.79423] [0.861525,2.78887] | q_near | [3.14,3.14] | q_rand | [-1.26041,2.46187]"

pi = -3.14159256
def get_dofs_as_list(dofs):

    dof_list = []

    fields = dofs.split("] [")


    for f in fields:
        f = f.translate(None, '[]').split(",")
        if len(f) == 2:
            dof_list.append((float(f[0].strip()), float(f[1].strip())))

    return dof_list


def get_line_info(line):
    fields = line.split("|")

    if len(fields) < 4:
        return None, None, []

    dofs = get_dofs_as_list(fields[1])
    q_near = fields[3].strip().translate(None, '[]').split(",")
    q_rand = fields[5].strip().translate(None, '[]').split(",")

    q_near_tup = (float(q_near[0].strip()), float(q_near[1].strip()))
    q_rand_tup = (float(q_rand[0].strip()), float(q_rand[1].strip()))

    return q_near_tup, q_rand_tup, dofs

filein1 = open("AA_m0.4_pio180.dat", "r")
pmffile = open("pmf.dat", "r")

pmfx = []
pmfy = []
pmfz = []

i=0
for line in pmffile:

        fields = line.split()
        pmfx.append(float(fields[2].strip()))
        pmfy.append(float(fields[3].strip()))
        pmfz.append(math.log(float(fields[1].strip())))
        #print fields


all_dofs = []

for l in filein1.readlines():

    if l.startswith("RRT Step"):
        q_near_tup, q_rand_tup, dofs = get_line_info(l)
        all_dofs += dofs
print(len(all_dofs))
plt.tripcolor(pmfx,pmfy,pmfz)
#for i in range(len(pmfx)):
#    if i%20==0:
#        print pmfx[i], pmfy[i], pmfz[i]

plt.scatter(*zip(*all_dofs))
xi = numpy.linspace(-pi, pi)
yi = numpy.linspace(-pi, pi)
#Z = griddata(pmfx,pmfy,pmfz, xi, yi, interp='linear')
#plt.contourf(pmfx,pmfy,Z)

plt.show()





