import os
density_map="data/em_maps/groel-11.5A.mrc"
spacing=2.7
origin=[-135,-135,-135]
num_segments=14
density_threshold=0.7
centers_output="output/groel_segments_center.pdb"
segments_output= "output/groel_subunit"
input=[density_map,num_segments,density_threshold,centers_output,"--seg",segments_output,"--apix",spacing,"--x",origin[0],"--y",origin[1],"--z",origin[2]]
exe="$IMP/modules/multifit/bin/density2anchors"
command="$IMP/tools/imppy.sh "+exe+" "+str(input).replace("[","").replace("]","").replace(",", " ").replace("'","")
print "======running command:"
print command
os.system(command)
