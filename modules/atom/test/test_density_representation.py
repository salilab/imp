import IMP
import IMP.atom
import IMP.core
import IMP.algebra
import IMP.test

def bead2gaussian(center,radius,mdl,p=None):
    trans = IMP.algebra.Transformation3D(
        IMP.algebra.get_identity_rotation_3d(),center)
    shape = IMP.algebra.Gaussian3D(
        IMP.algebra.ReferenceFrame3D(trans),[radius]*3)
    if p is None:
        return IMP.core.Gaussian.setup_particle(IMP.Particle(mdl),shape)
    else:
        return IMP.core.Gaussian.setup_particle(p,shape)

class DensityRepresentationTest(IMP.test.TestCase):

    def test_simple_density(self):
        """Test representing a particle with a Gaussian"""
        # create single bead
        mdl = IMP.Model()
        center = IMP.algebra.Vector3D(1,1,1)
        rad = 1
        p1 = IMP.Particle(mdl)
        xyz1 = IMP.core.XYZ.setup_particle(p1, center)

        # create representation
        rep = IMP.atom.Representation.setup_particle(p1,0)

        # check no density exists yet
        ht = rep.get_representation(1,IMP.atom.DENSITIES)
        self.assertTrue(not rep.get_representation(1,IMP.atom.DENSITIES))

        # create Gaussian representation and check again
        g2 = bead2gaussian(center,rad,mdl)
        rep.add_representation(g2.get_particle(),IMP.atom.DENSITIES,1)
        self.assertEqual(rep.get_representation(1,IMP.atom.DENSITIES),
                         g2.get_particle())

        # should get correct representation type even with incorrect resolution
        self.assertEqual(rep.get_representation(0,IMP.atom.DENSITIES),
                         g2.get_particle())


    def test_self_density(self):
        """Test representing a particle with itself as the Gaussian"""
        # create single bead
        mdl = IMP.Model()
        center = IMP.algebra.Vector3D(1,1,1)
        rad = 1
        p1 = IMP.Particle(mdl)
        xyz1 = IMP.core.XYZ.setup_particle(p1, center)

        # create Gaussian representation
        g1 = bead2gaussian(center,rad,mdl,p1)
        rep = IMP.atom.Representation.setup_particle(p1,0)
        rep.add_representation(p1,IMP.atom.DENSITIES,1)

        # check get_representation
        self.assertEqual(rep.get_representation(1,IMP.atom.DENSITIES),
                         g1.get_particle())

    def test_multi_density(self):
        """Test representing a group of residues with group of Gaussians"""
        # read in system
        mdl = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('1z5s_C.pdb'),mdl)
        ch = mh.get_children()[0]
        idxs = [IMP.atom.Residue(h).get_index() \
                for h in IMP.atom.get_by_type(ch,IMP.atom.RESIDUE_TYPE)]
        rep = IMP.atom.Representation.setup_particle(ch,0)

        # average along backbone to get Gaussians (don't do this in real life)
        sh = IMP.atom.create_simplified_along_backbone(ch,10)
        density_frag = IMP.atom.Fragment.setup_particle(IMP.Particle(mdl),idxs)
        for frag in sh.get_children():
            gp = frag.get_particle()
            xyzr = IMP.core.XYZR(gp)
            g = bead2gaussian(xyzr.get_coordinates(),xyzr.get_radius(),mdl)
            density_frag.add_child(g)

        rep.add_representation(density_frag.get_particle(),IMP.atom.DENSITIES,10)

        # check get_representation
        self.assertEqual(rep.get_representation(10,IMP.atom.DENSITIES),
                         density_frag.get_particle())

        ## check selection - should work even without resolution
        sel = IMP.atom.Selection(mh,residue_index=idxs[5],
                                 representation_type = IMP.atom.DENSITIES)
        self.assertEqual(sel.get_selected_particles(),
                         [h.get_particle() for h in density_frag.get_children()])


if __name__ == '__main__':
    IMP.test.main()
