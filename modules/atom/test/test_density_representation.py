import IMP
import IMP.atom
import IMP.core
import IMP.algebra
import IMP.test

def bead2gaussian(center,radius,mdl):
    trans = IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),center)
    shape = IMP.algebra.Gaussian3D(IMP.algebra.ReferenceFrame3D(trans),[radius]*3)
    return IMP.core.Gaussian.setup_particle(IMP.Particle(mdl),shape)


class DensityRepresentationTest(IMP.test.TestCase):

    def test_simple_density(self):
        # create single bead
        mdl = IMP.Model()
        center = IMP.algebra.Vector3D(1,1,1)
        rad = 1
        p1 = IMP.Particle(mdl)
        xyz1 = IMP.core.XYZ.setup_particle(p1, center)

        # create Gaussian representation
        g2 = bead2gaussian(center,rad,mdl)
        rep = IMP.atom.Representation.setup_particle(p1,0)
        rep.add_representation(g2.get_particle(),IMP.atom.DENSITIES,1)

        # check get_representation
        self.assertEqual(rep.get_representation(1,IMP.atom.DENSITIES),g2.get_particle())

    def test_multi_density(self):
        # read in system
        mdl = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('1z5s_C.pdb'),mdl)
        ch = mh.get_children()[0]
        rep = IMP.atom.Representation.setup_particle(ch,0)
        idxs = [IMP.atom.Residue(h).get_index() for h in IMP.atom.get_by_type(ch,IMP.atom.RESIDUE_TYPE)]

        # create single Gaussian to represent the density
        centroid = IMP.algebra.get_centroid([IMP.core.XYZ(p).get_coordinates() for p in IMP.core.get_leaves(mh)])
        g = bead2gaussian(centroid,100,mdl)
        gp = g.get_particle()
        f = IMP.atom.Fragment.setup_particle(gp,idxs)
        rep.add_representation(gp,IMP.atom.DENSITIES,100)

        # check get_representation
        self.assertEqual(rep.get_representation(1,IMP.atom.DENSITIES),g.get_particle())

        # check selection
        sel = IMP.atom.Selection(mh,residue_index=idxs[5],resolution=100) # representation_type = IMP.atom.DENSITIES)
        self.assertEqual(sel.get_selected_particles(),[gp])

if __name__ == '__main__':
    IMP.test.main()
