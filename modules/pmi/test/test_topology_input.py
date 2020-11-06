import IMP
import IMP.pmi
import IMP.pmi.topology
import IMP.pmi.macros
import os
import warnings
import IMP.test
import IMP.rmf
import IMP.pmi.plotting
import IMP.pmi.plotting.topology

def children_as_dict(h):
    cdict={}
    for c in h.get_children():
        cdict[c.get_name()]=c
    return cdict

class Tests(IMP.test.TestCase):
    def test_old(self):
        """Test reading of old-style topology file"""
        topology_file = self.get_input_file_name("topology.txt")
        with IMP.allow_deprecated():
            t = IMP.pmi.topology.TopologyReader(topology_file)
        c = t.get_components()
        self.assertEqual(len(c), 3)
        self.assertEqual(c[0].molname, "Prot1")
        self.assertEqual(os.path.abspath(c[0].fasta_file),
                         self.get_input_file_name("seqs.fasta"))
        self.assertEqual(c[1].molname, "Prot2")
        self.assertEqual(c[1].get_unique_name(), "Prot2..0")
        self.assertEqual(c[2].get_unique_name(), "Prot2..1")

    def test_reading(self):
        """Test basic reading"""
        topology_file = self.get_input_file_name("topology_new.txt")
        t = IMP.pmi.topology.TopologyReader(topology_file)
        self.assertEqual(list(t.molecules.keys()),
                         ['Prot1', 'Prot2', 'Prot3', 'Prot4', 'Prot5',
                          'RNA1', 'DNA1'])
        c = t.get_components()
        self.assertEqual(len(c),11)
        self.assertEqual(c[0].molname,"Prot1")
        self.assertEqual(c[1].molname,"Prot1")
        self.assertEqual(c[1].copyname,"1")
        self.assertEqual(c[2].get_unique_name(),"Prot2..0")
        self.assertEqual(c[3].get_unique_name(),"Prot2..1")
        self.assertEqual(c[5].get_unique_name(),"Prot2.1.1")

    def test_round_trip(self):
        """Test reading and writing"""
        topology_file = self.get_input_file_name("topology_new.txt")
        t = IMP.pmi.topology.TopologyReader(topology_file)
        components_original = t.get_components()

        outfile = self.get_tmp_file_name("ttest.txt")
        t.write_topology_file(outfile)
        t_new = IMP.pmi.topology.TopologyReader(outfile)
        components_written = t_new.get_components()

        for original, written in zip(components_original, components_written):
            for key in original.__dict__:
                self.assertEqual(original.__dict__[key], written.__dict__[key])

    def test_beads(self):
        try:
            import sklearn
        except ImportError:
            self.skipTest("no sklearn package")
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

        sel = IMP.atom.Selection(root_hier,molecule="Prot2",
                                 residue_indexes=range(1,13),
                                 resolution=IMP.atom.ALL_RESOLUTIONS)
        self.assertEqual(len(sel.get_selected_particles()),2)
        IMP.atom.show_with_representations(root_hier)
        sel = IMP.atom.Selection(root_hier,molecule="Prot2",
                                 residue_indexes=range(13,30),
                                 resolution=IMP.atom.ALL_RESOLUTIONS)
        self.assertEqual(len(sel.get_selected_particles()),17+2)

        # check rigid and flexible parts
        rbs = dof.get_rigid_bodies()
        fbs = dof.get_flexible_beads()
        self.assertEqual(len(rbs),2)
        self.assertEqual(len(fbs),7)

    def test_flexible(self):
        """Check that movers are created for flexible regions"""
        mdl = IMP.Model()
        tfile = self.get_input_file_name('topology_flexible.txt')
        input_dir = os.path.dirname(tfile)
        t = IMP.pmi.topology.TopologyReader(tfile,
                                            pdb_dir=input_dir,
                                            fasta_dir=input_dir,
                                            gmm_dir=input_dir)
        bs = IMP.pmi.macros.BuildSystem(mdl)
        bs.add_state(t)
        with warnings.catch_warnings(record=True) as cw:
            warnings.simplefilter("always")
            root_hier, dof = bs.execute_macro()
        # Both domains (one a PDB, one beads) should be flexible
        # There should be 12 flexible beads:
        # Residues 1, 2, 5, 6 (taken from the PDB file)
        # Beads 3, 4, 7, 8, 9, 10
        # Resolution=10 fragments 1-2, 5-6
        self.assertEqual(len(dof.get_movers()), 12)
        self.assertEqual(len(dof.get_rigid_bodies()), 0)
        self.assertEqual(len(dof.get_flexible_beads()), 12)
        # One warning should be emitted, for the PDB domain
        w, = cw
        self.assertIn("Making Prot1..0 flexible. This will likely distort",
                      str(w.message))
        self.assertIs(w.category, IMP.pmi.StructureWarning)

    def test_draw_molecular_composition(self):
        try:
            import matplotlib
        except ImportError:
            self.skipTest("no matplotlib package")
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
        IMP.pmi.plotting.topology.draw_component_composition(dof)


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
        comps = t.get_components()
        self.assertEqual(comps[0].pdb_file, os.path.join(input_dir,'prot.pdb'))
        rbs = t.get_rigid_bodies()
        srbs = t.get_super_rigid_bodies()
        csrbs = t.get_chains_of_super_rigid_bodies()

        expected_rbs = [['Prot1.1.0','Prot1..0'],
                        ['Prot2..0','Prot2..1','Prot2.1.0','Prot2.1.1'],
                        ['Prot4..0', 'Prot5..0'],
                        ['DNA1..0'], ['RNA1..0']]
        expected_srbs = [['Prot1.1.0','Prot1..0','Prot2..0','Prot2..1',
                          'Prot2.1.0','Prot2.1.1','Prot4..0','Prot3..0', 'Prot5..0'],
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

        sel5 = IMP.atom.Selection(root_hier, molecule='Prot5',
                                  resolution=1).get_selected_particles()

        rna = IMP.atom.Selection(root_hier, molecule='RNA1',
                                  resolution=1).get_selected_particles()
        self.assertTrue(all(IMP.atom.Residue.get_is_setup(r) for r in rna))
        self.assertEqual([IMP.atom.Residue(r).get_residue_type() for r in rna],
                         [IMP.atom.ADE, IMP.atom.CYT, IMP.atom.GUA,
                          IMP.atom.URA])

        dna = IMP.atom.Selection(root_hier, molecule='DNA1',
                                  resolution=1).get_selected_particles()
        self.assertTrue(all(IMP.atom.Residue.get_is_setup(r) for r in dna))
        self.assertEqual([IMP.atom.Residue(r).get_residue_type() for r in dna],
                         [IMP.atom.DADE, IMP.atom.DCYT, IMP.atom.DGUA,
                          IMP.atom.DTHY])

        color=IMP.display.Colored(sel5[0]).get_color()
        self.assertAlmostEqual(color.get_red(), 0.1, delta=1e-6)
        self.assertAlmostEqual(color.get_green(), 0.2, delta=1e-6)
        self.assertAlmostEqual(color.get_blue(), 0.3, delta=1e-6)
        self.assertEqual(len(sel4_1),10)
        self.assertEqual(len(sel4_10),1)
        self.assertEqual(len(sel4_D),2)

        # check rigid bodies
        rbs = dof.get_rigid_bodies()
        fbs = dof.get_flexible_beads()
        self.assertEqual(len(rbs),5)
        #                         Prot1x2 Prot3
        self.assertEqual(len(fbs), 4   +  2)

    def test_build_system_mmcif(self):
        """Test BuildSystem macro with mmCIF input files"""
        try:
            import sklearn
        except ImportError:
            self.skipTest("no sklearn package")
        mdl = IMP.Model()
        tfile = self.get_input_file_name('topology_mmcif.txt')
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
                                  resolution=1,
                                  copy_index=0).get_selected_particles()
        self.assertEqual(len(sel1), 7  + 2 )



if __name__=="__main__":
    IMP.test.main()
