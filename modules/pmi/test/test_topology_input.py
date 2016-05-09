import IMP
import IMP.pmi
import IMP.pmi.topology
import IMP.pmi.macros
import os
import IMP.test
import IMP.rmf
import RMF
import math
def children_as_dict(h):
    cdict={}
    for c in h.get_children():
        cdict[c.get_name()]=c
    return cdict

class Tests(IMP.test.TestCase):
    def test_reading(self):
        """Test basic reading"""
        topology_file = self.get_input_file_name("topology_new.txt")
        t = IMP.pmi.topology.TopologyReader(topology_file)
        c = t.get_components()
        self.assertEqual(len(c),8)
        self.assertEqual(c[0].molname,"Prot1")
        self.assertEqual(c[1].molname,"Prot1")
        self.assertEqual(c[1].copyname,"1")
        self.assertEqual(c[5].get_unique_name(),"Prot2.1.1")

    def test_round_trip(self):
        """Test reading and writing"""
        topology_file=self.get_input_file_name("topology_new.txt")
        outfile = self.get_tmp_file_name("ttest.txt")
        t=IMP.pmi.topology.TopologyReader(topology_file)
        t.write_topology_file(outfile)

        tnew = IMP.pmi.topology.TopologyReader(outfile)
        c = tnew.get_components()
        self.assertEqual(len(c),8)
        self.assertEqual(c[0].molname,"Prot1")
        self.assertEqual(c[1].molname,"Prot1")
        self.assertEqual(c[1].copyname,"1")
        self.assertEqual(c[5].get_unique_name(),"Prot2.1.1")

    def test_beads(self):
        mdl = IMP.Model()
        tfile = self.get_input_file_name('topology_beads.txt')
        input_dir = os.path.dirname(tfile)
        t = IMP.pmi.topology.TopologyReader(tfile,
                                            pdb_dir=input_dir,
                                            fasta_dir=input_dir,
                                            gmm_dir=input_dir)
        bs = IMP.pmi.macros.BuildSystem(mdl)
        bs.add_state(t)
        root_hier, dof = bs.execute_macro()
        sel = IMP.atom.Selection(root_hier,residue_indexes=range(1,13),
                                 resolution=IMP.atom.ALL_RESOLUTIONS)
        self.assertEqual(len(sel.get_selected_particles()),2)
        IMP.atom.show_with_representations(root_hier)
        sel = IMP.atom.Selection(root_hier,residue_indexes=range(13,30),
                                 resolution=IMP.atom.ALL_RESOLUTIONS)
        self.assertEqual(len(sel.get_selected_particles()),17+2)

    def test_set_movers(self):
        """Check if rigid bodies etc are set up as requested"""
        try:
            import sklearn
        except ImportError:
            self.skipTest("no sklearn package")
        mdl = IMP.Model()
        tfile = self.get_input_file_name('topology_new.txt')
        input_dir = os.path.dirname(tfile)
        t = IMP.pmi.topology.TopologyReader(tfile,
                                            pdb_dir=input_dir,
                                            fasta_dir=input_dir,
                                            gmm_dir=input_dir)
        self.assertEqual(t.components[0].pdb_file,
                        os.path.join(input_dir,'prot.pdb'))
        rbs = t.get_rigid_bodies()
        srbs = t.get_super_rigid_bodies()
        csrbs = t.get_chains_of_super_rigid_bodies()

        expected_rbs = [['Prot1.1.0','Prot1..0'],
                        ['Prot2..0','Prot2..1','Prot2.1.0','Prot2.1.1'],
                        ['Prot4..0']]
        expected_srbs = [['Prot1.1.0','Prot1..0','Prot2..0','Prot2..1',
                          'Prot2.1.0','Prot2.1.1','Prot4..0','Prot3..0'],
                         ['Prot1.1.0','Prot1..0','Prot3..0']]

        found1 = set(tuple(sorted(i)) for i in rbs)
        found2 = set(tuple(sorted(i)) for i in expected_rbs)
        self.assertEqual(found1,found2)

        found1 = set(tuple(sorted(i)) for i in srbs)
        found2 = set(tuple(sorted(i)) for i in expected_srbs)
        self.assertEqual(found1,found2)

    def test_build_system(self):
        """Test the new BuildSystem macro including beads and ideal helix"""
        try:
            import sklearn
        except ImportError:
            self.skipTest("no sklearn package")
        mdl = IMP.Model()
        tfile = self.get_input_file_name('topology_new.txt')
        input_dir = os.path.dirname(tfile)
        t = IMP.pmi.topology.TopologyReader(tfile,
                                            pdb_dir=input_dir,
                                            fasta_dir=input_dir,
                                            gmm_dir=input_dir)
        bs = IMP.pmi.macros.BuildSystem(mdl)
        bs.add_state(t)
        root_hier, dof = bs.execute_macro()

        # check a few selections
        sel1 = IMP.atom.Selection(root_hier,molecule="Prot1",
                                  resolution=1,copy_index=0).get_selected_particles()
        #                          res1 bead
        self.assertEqual(len(sel1), 7  + 2 )

        sel1 = IMP.atom.Selection(root_hier,molecule="Prot1",
                                  resolution=1,copy_index=1).get_selected_particles()
        #                          res1 bead
        self.assertEqual(len(sel1), 7  + 2 )

        sel1D = IMP.atom.Selection(root_hier,molecule="Prot1",
                                  representation_type=IMP.atom.DENSITIES).get_selected_particles()
        self.assertEqual(len(sel1D),6) #3 each

        sel2 = IMP.atom.Selection(root_hier,molecule="Prot2",
                                  resolution=10,
                                  copy_index=0).get_selected_particles()
        self.assertEqual(len(sel2),2)

        sel2 = IMP.atom.Selection(root_hier,molecule="Prot2",
                                  resolution=10,
                                  copy_index=1).get_selected_particles()
        self.assertEqual(len(sel2),2)

        sel3 = IMP.atom.Selection(root_hier,molecule="Prot3",resolution=5).get_selected_particles()
        self.assertEqual(len(sel3),2)

        sel4_1 = IMP.atom.Selection(root_hier,molecule="Prot4",
                                    resolution=1).get_selected_particles()
        sel4_10 = IMP.atom.Selection(root_hier,molecule="Prot4",
                                     resolution=10).get_selected_particles()
        sel4_D = IMP.atom.Selection(root_hier,molecule="Prot4",
                                    representation_type=IMP.atom.DENSITIES).get_selected_particles()
        self.assertEqual(len(sel4_1),10)
        self.assertEqual(len(sel4_10),1)
        self.assertEqual(len(sel4_D),2)

        # check rigid bodies
        rbs = dof.get_rigid_bodies()
        fbs = dof.get_flexible_beads()
        self.assertEqual(len(rbs),3)
        #                         Prot1x2 Prot3
        self.assertEqual(len(fbs), 4   +  2)

if __name__=="__main__":
    IMP.test.main()
