import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.display


class Tests(IMP.test.TestCase):

    def _residual_cond(self, x):
        if x != 0:
            return 1
        else:
            return 0

    def test_simplify_by_residue(self):
        """Test protein simplification by residues"""
        IMP.base.set_log_level(IMP.base.SILENT)  # VERBOSE)
        m = IMP.kernel.Model()
        print 'reading'
        mh = IMP.atom.read_pdb(self.get_input_file_name('input.pdb'), m)
        print "getting chains"
        chains = IMP.atom.get_by_type(mh, IMP.atom.CHAIN_TYPE)
        num_residues = len(
            IMP.atom.get_by_type(chains[0],
                                 IMP.atom.RESIDUE_TYPE))
        IMP.atom.add_radii(mh)
        for res_segment in [5, 10, 20, 30, num_residues]:
            print "simplifying"
            mh_simp = IMP.atom.create_simplified_along_backbone(
                IMP.atom.Chain(chains[0].get_particle()),
                res_segment)
            IMP.atom.show_molecular_hierarchy(mh_simp)
            #w= IMP.display.PymolWriter(self.get_tmp_file_name("simplified1."+str(res_segment)+".pym"))
            # IMP.atom.show_hierarchy(mh_simp)
            # for p in IMP.atom.get_leaves(mh_simp):
            #    d= IMP.core.XYZR(p.get_particle())
            #    w.add_geometry(IMP.display.SphereGeometry(d.get_sphere()))
            o = self._residual_cond(num_residues % res_segment)
            print "getting leaves"
            lvs = IMP.core.get_leaves(mh_simp)
            self.assertEqual(num_residues / res_segment + o, len(lvs))

    def test_simplify_by_segments(self):
        """Test protein simplification by segments"""
        IMP.base.set_log_level(IMP.base.SILENT)  # VERBOSE)
        m = IMP.kernel.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('input.pdb'), m)
        chains = IMP.atom.get_by_type(mh, IMP.atom.CHAIN_TYPE)
        IMP.atom.add_radii(mh)
        # define the segments
        segs = []
        num_res = len(IMP.atom.get_by_type(chains[0], IMP.atom.RESIDUE_TYPE))
        start = 0
        step = 30
        while start <= num_res:
            segs.append((start, min(start + step, num_res + 1)))
            start = start + step
            print segs[-1]
        print start
        mh_simp = IMP.atom.create_simplified_along_backbone(
            IMP.atom.Chain(chains[0].get_particle()), segs)
        IMP.atom.show_molecular_hierarchy(mh_simp)
        print segs
        IMP.atom.show_molecular_hierarchy(mh_simp)
        # for l in IMP.core.get_leaves(mh_simp):
        # print "leaf"
        #    IMP.atom.show_molecular_hierarchy(l)
        #    f= IMP.atom.Fragment(l)
        #    print f.get_residue_indexes()
        self.assertEqual(len(segs),
                         len(IMP.core.get_leaves(mh_simp)))


    def test_simplify_by_volume(self):
        """Test protein simplification by volume"""
        IMP.base.set_log_level(IMP.base.SILENT)  # VERBOSE)
        m = IMP.kernel.Model()
        print 'reading'
        mh = IMP.atom.read_pdb(self.get_input_file_name('input.pdb'), m)
        print "getting chains"
        chains = IMP.atom.get_by_type(mh, IMP.atom.CHAIN_TYPE)
        IMP.atom.add_radii(mh)
        for resolution_A in [0.1, 0.3, 0.5, 0.66, 1]:
            print "simplifying", resolution_A
            mh_simp = IMP.atom.create_simplified_assembly_from_volume(
                IMP.atom.Chain(chains[0].get_particle()),
                resolution_A)
            IMP.atom.show_molecular_hierarchy(mh_simp)
            #w= IMP.display.PymolWriter(self.get_tmp_file_name("simplified1."+str(resolution_A)+".pym"))
            # IMP.atom.show_hierarchy(mh_simp)
            # for p in IMP.atom.get_leaves(mh_simp):
            #    d= IMP.core.XYZR(p.get_particle())
            #    w.add_geometry(IMP.display.SphereGeometry(d.get_sphere()))
#            o = self._residual_cond(num_residues % res_segment)
#            print "getting leaves"
#            lvs = IMP.core.get_leaves(mh_simp)
#            self.assertEqual(num_residues / res_segment + o, len(lvs))


if __name__ == '__main__':
    IMP.test.main()
