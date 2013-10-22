import IMP.em
import IMP.core
import IMP.multifit

resolution=15.
dmap=IMP.em.read_map(IMP.multifit.get_example_path('3sfd/3sfd_15.mrc'),IMP.em.MRCReaderWriter())
dmap.get_header_writable().set_resolution(resolution)

m= IMP.kernel.Model()

sel=IMP.atom.CAlphaPDBSelector()
mh=IMP.atom.read_pdb(IMP.multifit.get_example_path('3sfd/3sfd.A.pdb'),m,sel)
IMP.atom.add_radii(mh)
ps= IMP.core.get_leaves(mh)
IMP.atom.create_rigid_body(mh)

refiner = IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits())


fitting_sols=IMP.em.local_rigid_fitting(
   mh.get_particle(),refiner,
   IMP.atom.Mass.get_mass_key(),
   dmap,[],20,10,100)
