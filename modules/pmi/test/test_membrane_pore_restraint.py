import IMP
import IMP.atom
import IMP.pmi
import IMP.pmi.restraints.proteomics
import IMP.pmi.nonmantained
import IMP.pmi.representation
import IMP.pmi.io
import IMP.rmf
import RMF


m = IMP.Model()

r=IMP.pmi.representation.Representation(m)

smr=IMP.pmi.nonmantained.SetupMembranePoreRestraint(
        r,
        selection_tuples_outside=None,
        selection_tuples_membrane=None,
        selection_tuples_inside=None,
        center=(0.0,0.0,0.0),
        z_tickness=100,
        radius=300,
        membrane_tickness=40.0,
        resolution=1,
        label="None")

smr.create_representation()
rh = RMF.create_rmf_file("test.rmf3")
IMP.rmf.add_hierarchies(rh, [r.prot])
IMP.rmf.save_frame(rh)
del rh
