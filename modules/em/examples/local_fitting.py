import IMP.em
import IMP.core
import IMP.atom
import random,math

IMP.set_log_level(IMP.TERSE)#SILENT)
m= IMP.Model()
#1. setup the input protein
##1.1 select a selector.
sel=IMP.atom.NonWaterPDBSelector()
##1.2 read the protein
mh=IMP.atom.read_pdb(IMP.em.get_example_path("input.pdb"),m,sel)
mh_ref=IMP.atom.read_pdb(IMP.em.get_example_path("input.pdb"),m,sel)
##1.3 add radius info to each atom, otherwise the resampling would fail.
IMP.atom.add_radii(mh)
IMP.atom.add_radii(mh_ref)
ps= IMP.Particles(IMP.core.get_leaves(mh))
ps_ref= IMP.Particles(IMP.core.get_leaves(mh_ref))
#2. read the density map of the protein
resolution=8.
voxel_size=1.5
dmap=IMP.em.read_map(IMP.em.get_example_path("input.mrc"),IMP.em.MRCReaderWriter())
dmap.get_header_writable().set_resolution(resolution)
#3. The protein is now fitted correctly in the density. We can validate
#that by making sure that the cross-correlation score is close to 1.

##3.1 generate a sampled density map to the same resolution and spacing as the target density map. Note that the function we are going to use (cross_correlation_coefficient) expect to get the same map dimensions as the target density map.
sampled_input_density = IMP.em.SampledDensityMap(dmap.get_header())
sampled_input_density.set_particles(ps)
sampled_input_density.resample()
sampled_input_density.calcRMS()
IMP.em.write_map(sampled_input_density,"vv0.mrc",IMP.em.MRCReaderWriter())
#3.2 calculate the cross-correlation score, which should be close to 1
best_score=IMP.em.CoarseCC.cross_correlation_coefficient(
    dmap,sampled_input_density,sampled_input_density.get_header().dmin)
print "The CC score of the native transformation is:",best_score

#4. To denostrate local fitting we locally rotate and translate the protein and show how we can go back to the correct placement.

##4.1 define a local transformatione
translation = IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
axis = IMP.algebra.get_random_vector_on(IMP.algebra.get_unit_sphere_3d())
rand_angle = random.uniform(-30./180*math.pi,30./180*math.pi)
r= IMP.algebra.get_rotation_in_radians_about_axis(axis, rand_angle);
local_trans=IMP.algebra.Transformation3D(r,translation)
##4.2 rotate the protein
# prot_xyz=IMP.core.XYZs(IMP.core.get_leaves(mh))
# for xyz in prot_xyz:
#     xyz.set_coordinates(local_trans.get_transformed(xyz.get_coordinates()))
##4.2 set the protein as a rigid body
prot_rb = IMP.atom.rigid_body_setup_hierarchy(mh)
##4.3 apply the trasnformation to the protein
IMP.core.transform(prot_rb,local_trans)
m.evaluate(None)#to make sure the transformation was applied
##4.4 print the new correlation score, should be lower than before
print len(IMP.core.get_leaves(mh))
IMP.atom.write_pdb(mh,"input2.pdb")
print "wrote input2"
sampled_input_density.resample()
sampled_input_density.calcRMS()
IMP.em.write_map(sampled_input_density,"vv.mrc",IMP.em.MRCReaderWriter())
score1=IMP.em.CoarseCC.cross_correlation_coefficient(
    dmap,sampled_input_density,sampled_input_density.get_header().dmin)
rmsd=IMP.atom.get_rmsd(IMP.core.XYZs(ps),IMP.core.XYZs(ps_ref))
print "The start score is:",score1, "with rmsd of:",rmsd
##5. apply local fitting
## 5.1 run local fitting
print "preforming local refinement, may run for 3-4 minutes"
fitting_sols = IMP.em.FittingSolutions() #fitting solutions will go here.
## translate the molecule to the center of the density
IMP.core.transform(prot_rb,IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),dmap.get_centroid()-IMP.core.centroid(IMP.core.XYZs(ps))))
m.evaluate(None)#to make sure the transformation was applied
sampled_input_density.resample()
sampled_input_density.calcRMS()
rmsd=IMP.atom.get_rmsd(IMP.core.XYZs(ps),IMP.core.XYZs(ps_ref))
score2=IMP.em.CoarseCC.cross_correlation_coefficient(
    dmap,sampled_input_density,sampled_input_density.get_header().dmin)
print "The score after centering is:",score2, "with rmsd of:",rmsd
# IMP.em.local_rigid_fitting_grid_search(
#    ps,IMP.core.XYZR.get_default_radius_key(),
#    IMP.atom.Mass.get_mass_key(),
#    dmap,fitting_sols)

num_sol=3
IMP.em.local_rigid_fitting(
   prot_rb,IMP.core.XYZR.get_default_radius_key(),
   IMP.atom.Mass.get_mass_key(),
   dmap,fitting_sols,None,num_sol,10,20)


## 5.2 report best result
### 5.2.1 transform the protein to the preferred transformation
for i in range(num_sol):
    #IMP.core.transform(prot_rb,fitting_sols.get_transformation(i))
    prot_rb.set_transformation(fitting_sols.get_transformation(i))
    m.evaluate(None)#to make sure the transformation was applied
## 5.2.2 calc rmsd to native configuration
    rmsd=IMP.atom.get_rmsd(IMP.core.XYZs(ps),IMP.core.XYZs(IMP.core.get_leaves(mh_ref)))
    print "Fit with index:",i," with cc: ",1.-fitting_sols.get_score(i), " and rmsd to native of:",rmsd
    IMP.atom.write_pdb(mh,"sol_"+str(i)+".pdb")
    IMP.core.transform(prot_rb,fitting_sols.get_transformation(i).get_inverse())
