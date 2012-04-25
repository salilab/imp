import RMF
import IMP.base

# find the name for a temporary file to use to for writing the hdf5 file
tfn=IMP.base.create_temporary_file_name("aliases", ".rmf")
print "File is", tfn

# open the temporary file, clearing any existing contents
fh = RMF.create_rmf_file(tfn)

rh= fh.get_root_node()
# add a bunch of "molecules" with colors
mrh= rh.add_child("molecules", RMF.REPRESENTATION)

pf= RMF.ParticleFactory(fh)
cf= RMF.ColoredFactory(fh)

red=[]
green=[]
blue=[]
for i in range(0,10):
    curph= mrh.add_child("mol"+str(i), RMF.REPRESENTATION)
    for j in range(0,10):
        curh= curph.add_child("atom"+str(j), RMF.REPRESENTATION)
        p= pf.get(curh)
        p.set_coordinates([3*i,3*j,0])
        p.set_mass(1)
        p.set_radius(1)
        c= cf.get(curh)
        if j%3==0:
            c.set_rgb_color([1,0,0])
            red.append(curh)
        elif j%3==1:
            c.set_rgb_color([0,1,0])
            green.append(curh)
        else:
            c.set_rgb_color([0,0,1])
            blue.append(curh)

# add a second organization scheme by color
crh= rh.add_child("colors", RMF.REPRESENTATION)

redh= crh.add_child("red", RMF.REPRESENTATION)
greenh= crh.add_child("green", RMF.REPRESENTATION)
blueh= crh.add_child("blue", RMF.REPRESENTATION)

for r in red:
    RMF.add_child_alias(redh, r)
for g in green:
    RMF.add_child_alias(greenh, g)
for b in blue:
    RMF.add_child_alias(blueh, b)
