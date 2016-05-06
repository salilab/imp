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

class Tests(IMP.test.TestCase):

    def test_named_representation(self):
        """Test representation when you manually set resolutions"""
        mdl = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('1z5s_C.pdb'),mdl)
        mh.set_name("res0")


        res1 = IMP.atom.create_simplified_along_backbone(mh,1)
        res1.set_name('res1')
        res10 = IMP.atom.create_simplified_along_backbone(mh,10)
        res10.set_name('res10')
        root = IMP.atom.Hierarchy.setup_particle(IMP.Particle(mdl))
        root.add_child(mh)
        rep = IMP.atom.Representation.setup_particle(root,0)

        # should get res0 when it's the only resolution
        self.assertEqual(rep.get_representation(0),root)
        sel0 = IMP.atom.Selection(root,resolution=0,residue_index=432)
        self.assertEqual(set(sel0.get_selected_particles()),
                         set(mh.get_children()[0].get_children()[0].get_children()))

        # ... and when it's not
        rep.add_representation(res1,IMP.atom.BALLS,1)
        rep.add_representation(res10,IMP.atom.BALLS,10)
        self.assertEqual(rep.get_representation(0),root)
        sel0 = IMP.atom.Selection(root,resolution=0,residue_index=432)
        self.assertEqual(set(sel0.get_selected_particles()),
                         set(mh.get_children()[0].get_children()[0].get_children()))

        # checking other reps
        self.assertEqual(rep.get_representation(1),res1)
        self.assertEqual(rep.get_representation(10),res10)

        # should get nothing when requesting a type that isn't there
        #print(rep.get_representation(0,IMP.atom.DENSITIES))
        self.assertTrue(not rep.get_representation(0,IMP.atom.DENSITIES))
        self.assertTrue(not rep.get_representation(1,IMP.atom.DENSITIES))
        self.assertTrue(not rep.get_representation(10,IMP.atom.DENSITIES))

        # now test selection
        sel1 = IMP.atom.Selection(root,resolution=1,residue_index=432)
        self.assertEqual(IMP.atom.Fragment(sel1.get_selected_particles()[0]).get_residue_indexes(),
                         [432])
        sel10 = IMP.atom.Selection(root,resolution=10,residue_index=432)
        self.assertEqual(IMP.atom.Fragment(sel10.get_selected_particles()[0]).get_residue_indexes(),
                         list(range(432,442)))

    def test_rep_1and10(self):
        """Test representation when you manually set resolutions to 1 and 10"""
        mdl = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('1z5s_C.pdb'),mdl)
        mh.set_name("res0")

        res1 = IMP.atom.create_simplified_along_backbone(mh,1)
        res1.set_name('res1')
        res10 = IMP.atom.create_simplified_along_backbone(mh,10)
        res10.set_name('res10')

        root = IMP.atom.Hierarchy.setup_particle(IMP.Particle(mdl))
        rep = IMP.atom.Representation.setup_particle(root,1)
        root.add_child(res1)
        rep.add_representation(res10,IMP.atom.BALLS,10)

        # should get res1
        sel1 = IMP.atom.Selection(root,resolution=1)
        self.assertEqual(len(sel1.get_selected_particles()),
                         len(IMP.atom.get_by_type(mh,IMP.atom.RESIDUE_TYPE)))

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

    def test_no_density(self):
        """Test you get empty set if you request a non-existing representation type"""
        mdl = IMP.Model()
        root = IMP.atom.Hierarchy.setup_particle(IMP.Particle(mdl))

        # this subtree - no representation
        res = IMP.atom.Residue.setup_particle(IMP.Particle(mdl),IMP.atom.ResidueType("ALA"),1)
        p1 = IMP.Particle(mdl)
        x1 = IMP.core.XYZR.setup_particle(p1)
        a1 = IMP.atom.Atom.setup_particle(p1,IMP.atom.AtomType("CA"))
        res.add_child(a1)
        root.add_child(res)

        # this subtree - representation but no density
        segp = IMP.Particle(mdl)
        segf = IMP.atom.Fragment.setup_particle(segp,[2])
        segr = IMP.atom.Representation.setup_particle(segp,0)

        this_resolution = IMP.atom.Fragment.setup_particle(IMP.Particle(mdl),[2])
        p2 = IMP.Particle(mdl)
        x2 = IMP.core.XYZR.setup_particle(p2)
        a2 = IMP.atom.Atom.setup_particle(p2,IMP.atom.AtomType("CA"))
        res2 = IMP.atom.Residue.setup_particle(IMP.Particle(mdl),IMP.atom.ResidueType("ALA"),2)
        res2.add_child(a2)
        this_resolution.add_child(res2)

        segr.add_child(this_resolution)
        root.add_child(segr)

        # should get empty set if no densities
        sel0 = IMP.atom.Selection(root,representation_type=IMP.atom.DENSITIES).get_selected_particles()

        self.assertEqual(len(sel0),0)

    def test_self_density(self):
        """Test representing particles with themselves the Gaussians"""
        # read in system, store at atomic resolution
        mdl = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('1z5s_C.pdb'),mdl)
        ch = mh.get_children()[0]
        rep = IMP.atom.Representation.setup_particle(ch,0)
        idxs = [IMP.atom.Residue(h).get_index() \
                for h in IMP.atom.get_by_type(ch,IMP.atom.RESIDUE_TYPE)]

        # also create densities
        for p in IMP.core.get_leaves(mh):
            center = IMP.core.XYZ(p).get_coordinates()
            rad = 1
            g1 = bead2gaussian(center,rad,mdl,p)
        rep.add_representation(ch,IMP.atom.DENSITIES,0)

        # check selection
        selA = IMP.atom.Selection(mh,residue_index=idxs[0],resolution=0)
        selD = IMP.atom.Selection(mh,residue_index=idxs[0],
                                  representation_type=IMP.atom.DENSITIES)
        self.assertEqual(selA.get_selected_particles()[0],
                         selD.get_selected_particles()[0])

    def test_multi_density(self):
        """Test representing a group of residues with group of Densities"""
        # read in system
        mdl = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('1z5s_C.pdb'),mdl)
        ch = mh.get_children()[0]
        idxs = [IMP.atom.Residue(h).get_index() \
                for h in IMP.atom.get_by_type(ch,IMP.atom.RESIDUE_TYPE)]
        rep = IMP.atom.Representation.setup_particle(ch,0)

        # average along backbone to get Gaussians (don't do this in real life)
        tmp20 = IMP.atom.create_simplified_along_backbone(ch,20)
        density_frag = IMP.atom.Fragment.setup_particle(IMP.Particle(mdl),idxs)
        for frag in tmp20.get_children():
            gp = frag.get_particle()
            xyzr = IMP.core.XYZR(gp)
            g = bead2gaussian(xyzr.get_coordinates(),xyzr.get_radius(),mdl)
            density_frag.add_child(g)

        rep.add_representation(density_frag.get_particle(),IMP.atom.DENSITIES,20)

        # also add some beads
        res10 = IMP.atom.create_simplified_along_backbone(ch,10)
        rep.add_representation(res10,IMP.atom.BALLS,10)
        res20 = IMP.atom.create_simplified_along_backbone(ch,20)
        rep.add_representation(res20,IMP.atom.BALLS,20)

        # check get_representation
        self.assertEqual(rep.get_representation(20,IMP.atom.DENSITIES),
                         density_frag.get_particle())

        ## check that you get ALL gaussians when requesting one
        sel = IMP.atom.Selection(mh,residue_index=idxs[5],
                                 representation_type = IMP.atom.DENSITIES)
        self.assertEqual(sel.get_selected_particles(),
                         [h.get_particle() for h in density_frag.get_children()])

        ## check that you get ONLY the correct bead
        sel10 = IMP.atom.Selection(mh,residue_index=idxs[0],resolution=10)
        self.assertEqual(sel10.get_selected_particles()[0],
                         res10.get_children()[0])
        sel20 = IMP.atom.Selection(mh,residue_index=idxs[0],resolution=20)
        self.assertEqual(sel20.get_selected_particles()[0],
                         res20.get_children()[0])

    def test_show(self):
        """Test new show_with_representations function"""
        # read in system
        mdl = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('1z5s_C.pdb'),mdl)
        ch = mh.get_children()[0]
        idxs = [IMP.atom.Residue(h).get_index() \
                for h in IMP.atom.get_by_type(ch,IMP.atom.RESIDUE_TYPE)]
        rep = IMP.atom.Representation.setup_particle(ch,0)

        # average along backbone to get Gaussians (don't do this in real life)
        tmp20 = IMP.atom.create_simplified_along_backbone(ch,20)
        density_frag = IMP.atom.Fragment.setup_particle(IMP.Particle(mdl),idxs)
        for frag in tmp20.get_children():
            gp = frag.get_particle()
            xyzr = IMP.core.XYZR(gp)
            g = bead2gaussian(xyzr.get_coordinates(),xyzr.get_radius(),mdl)
            density_frag.add_child(g)
        density_frag.set_name("Density:20")
        rep.add_representation(density_frag.get_particle(),IMP.atom.DENSITIES,20)

        # also add some beads
        res10 = IMP.atom.create_simplified_along_backbone(ch,10)
        res10.set_name("Res:10")
        rep.add_representation(res10,IMP.atom.BALLS,10)

        res20 = IMP.atom.create_simplified_along_backbone(ch,20)
        res20.set_name("Res:20")
        rep.add_representation(res20,IMP.atom.BALLS,20)
        IMP.atom.show_with_representations(rep)

    def test_clone(self):
        """Test you can clone representations"""
        # read in system
        mdl = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('1z5s_C.pdb'),mdl)
        ch = mh.get_children()[0]
        idxs = [IMP.atom.Residue(h).get_index() \
                for h in IMP.atom.get_by_type(ch,IMP.atom.RESIDUE_TYPE)]
        rep = IMP.atom.Representation.setup_particle(ch,0)

        # average along backbone to get Gaussians (don't do this in real life)
        tmp20 = IMP.atom.create_simplified_along_backbone(ch,20)
        density_frag = IMP.atom.Fragment.setup_particle(IMP.Particle(mdl),idxs)
        for frag in tmp20.get_children():
            gp = frag.get_particle()
            xyzr = IMP.core.XYZR(gp)
            g = bead2gaussian(xyzr.get_coordinates(),xyzr.get_radius(),mdl)
            density_frag.add_child(g)
        density_frag.set_name("Density:20")
        rep.add_representation(density_frag.get_particle(),IMP.atom.DENSITIES,20)

        # also add some beads
        res10 = IMP.atom.create_simplified_along_backbone(ch,10)
        res10.set_name("Res:10")
        rep.add_representation(res10,IMP.atom.BALLS,10)

        res20 = IMP.atom.create_simplified_along_backbone(ch,20)
        res20.set_name("Res:20")
        rep.add_representation(res20,IMP.atom.BALLS,20)

        # clone it!
        clone = IMP.atom.create_clone(rep)

        # now test clone has the same representations and resolutions
        def compare_leaves(h0,h1):
            lv0 = IMP.core.get_leaves(h0)
            lv1 = IMP.core.get_leaves(h0)
            self.assertEqual(len(lv0),len(lv1))
            c0 = set(IMP.core.XYZ(p) for p in lv0)
            c1 = set(IMP.core.XYZ(p) for p in lv1)
            self.assertEqual(c0,c1)

        for rtype in (IMP.atom.BALLS,IMP.atom.DENSITIES):
            h0 = rep.get_representations(rtype)
            res0 = rep.get_resolutions(rtype)
            h1 = rep.get_representations(rtype)
            res1 = rep.get_resolutions(rtype)
            res0,h0 = zip(*sorted(zip(res0,h0)))
            res1,h1 = zip(*sorted(zip(res1,h1)))
            self.assertEqual(res0,res1)
            for nr in range(len(res0)):
                compare_leaves(h0[nr],h1[nr])

if __name__ == '__main__':
    IMP.test.main()
