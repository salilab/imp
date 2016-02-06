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

class TopologyReaderTests(IMP.test.TestCase):
    def test_reading(self):
        """Test basic reading"""
        topology_file=self.get_input_file_name("topology.txt")
        t=IMP.pmi.topology.TopologyReader(topology_file)
        self.assertEqual(len(t.component_list),3)
        self.assertEqual(t.component_list[0].domain_name,"Rpb1_1")
        self.assertEqual(t.component_list[1].name,"Rpb1")
        self.assertEqual(t.component_list[2].domain_name,"Rpb4")
        self.assertEqual(t.component_list[2].name,"Rpb4")

    def test_change_dir(self):
        """Test changing default pdb directory"""
        newdir="../../"
        topology_file = self.get_input_file_name("topology.txt")
        inputdir=os.path.dirname(topology_file)
        t = IMP.pmi.topology.TopologyReader(topology_file)
        t.set_dir("pdb_dir", newdir)
        self.assertEqual(os.path.abspath(t.pdb_dir),
                         os.path.abspath(os.path.join(inputdir,newdir)))
        self.assertEqual(t.component_list[0].pdb_file,
                         os.path.dirname(topology_file) \
                         + '/../../1WCM_map_fitted.pdb')

    def test_get_components(self):
        topology_file=self.get_input_file_name("topology.txt")
        t=IMP.pmi.topology.TopologyReader(topology_file)
        components=t.get_component_topologies([1,0,2])
        self.assertEqual(components[0].domain_name, "Rpb1_2")
        self.assertEqual(components[1].domain_name, "Rpb1_1")

    def test_round_trip(self):
        """Test reading and writing"""
        topology_file=self.get_input_file_name("topology.txt")
        outfile = self.get_tmp_file_name("ttest.txt")
        t=IMP.pmi.topology.TopologyReader(topology_file)
        t.write_topology_file(outfile)

        t=IMP.pmi.topology.TopologyReader(outfile)
        self.assertEqual(len(t.component_list),3)
        self.assertEqual(t.component_list[0].domain_name,"Rpb1_1")
        self.assertEqual(t.component_list[1].name,"Rpb1")
        self.assertEqual(t.component_list[2].domain_name,"Rpb4")
        self.assertEqual(t.component_list[2].name,"Rpb4")

    def test_build_read_gmms(self):
        """Test building with macro BuildModel using a topology file"""
        mdl = IMP.Model()

        topology_file=self.get_input_file_name("topology.txt")
        inputdir=os.path.dirname(topology_file)
        t=IMP.pmi.topology.TopologyReader(topology_file)
        self.assertEqual(os.path.abspath(t.gmm_dir),
                         os.path.abspath(inputdir))

        with IMP.allow_deprecated():
            bm = IMP.pmi.macros.BuildModel(mdl,
                                           component_topologies=t.component_list,
                                           force_create_gmm_files=False)
        rep = bm.get_representation()

        o = IMP.pmi.output.Output()
        rmf_fn = self.get_tmp_file_name("buildmodeltest.rmf")
        o.init_rmf(rmf_fn, [rep.prot])
        o.write_rmf(rmf_fn)
        o.close_rmf(rmf_fn)
        f = RMF.open_rmf_file_read_only(rmf_fn)
        r = IMP.rmf.create_hierarchies(f, mdl)[0]
        IMP.rmf.load_frame(f, RMF.FrameID(0))
        self.assertEqual(len(r.get_children()),2)
        cdict=children_as_dict(r)
        self.assertEqual(set([c.get_name() for c in cdict["Rpb1"].get_children()]),
                         set(["Beads" , "Rpb1_Res:1" , "Rpb1_Res:10"]))
        self.assertEqual(set([c.get_name() for c in cdict["Rpb4"].get_children()]),
                         set(["Beads","Rpb4_Res:1", "Rpb4_Res:10","Densities"]))
        r1dict=children_as_dict(cdict["Rpb1"])
        self.assertEqual(len(r1dict["Rpb1_Res:1"].get_children()),6)
        r4dict=children_as_dict(cdict["Rpb4"])
        self.assertEqual(len(r4dict["Densities"].get_children()[0].get_children()),5)

    def test_build_create_gmms(self):
        """Test building with macro using sklearn to create stuff"""
        try:
            import sklearn
        except ImportError:
            self.skipTest("no sklearn package")
        mdl = IMP.Model()

        topology_file=self.get_input_file_name("topology.txt")
        t=IMP.pmi.topology.TopologyReader(topology_file)
        t.set_dir("gmm_dir","../")

        with IMP.allow_deprecated():
            bm = IMP.pmi.macros.BuildModel(mdl,
                                           component_topologies=t.component_list,
                                           force_create_gmm_files=True)
        rep = bm.get_representation()

        o = IMP.pmi.output.Output()
        rmf_fn = self.get_tmp_file_name("buildmodeltest.rmf")
        o.init_rmf(rmf_fn, [rep.prot])
        o.write_rmf(rmf_fn)
        o.close_rmf(rmf_fn)
        f = RMF.open_rmf_file_read_only(rmf_fn)
        r = IMP.rmf.create_hierarchies(f, mdl)[0]
        IMP.rmf.load_frame(f, RMF.FrameID(0))
        self.assertEqual(len(r.get_children()),2)
        cdict=children_as_dict(r)
        self.assertEqual(set([c.get_name() for c in cdict["Rpb1"].get_children()]),
                         set(["Beads" , "Rpb1_Res:1" , "Rpb1_Res:10"]))
        self.assertEqual(set([c.get_name() for c in cdict["Rpb4"].get_children()]),
                         set(["Beads", "Rpb4_Res:0","Rpb4_Res:1", "Rpb4_Res:10","Densities"]))
        r1dict=children_as_dict(cdict["Rpb1"])
        self.assertEqual(len(r1dict["Rpb1_Res:1"].get_children()),6)
        r4dict=children_as_dict(cdict["Rpb4"])
        self.assertEqual(len(r4dict["Densities"].get_children()[0].get_children()),4)
        for output in ['../Rpb4.mrc', '../Rpb4.txt']:
            os.unlink(self.get_input_file_name(output))

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
        self.assertEqual(t.component_list[0].pdb_file,
                        os.path.join(input_dir,'1WCM_map_fitted.pdb'))
        rbs = t.get_rigid_bodies()
        srbs = t.get_super_rigid_bodies()
        csrbs = t.get_chains_of_super_rigid_bodies()
        self.assertEqual(rbs,[['Rpb1_1','Rpb1_2'],['Rpb4']])
        self.assertEqual(srbs,[['Rpb1_1','Rpb1_2'],
                               ['Rpb1_1','Rpb1_2','Rpb3','Rpb4'],
                               ['Rpb3','Rpb4']])
        self.assertEqual(csrbs,[['Rpb1_1','Rpb1_2'],['Rpb3']])

    def test_beads_only(self):
        """Test setting up BEADS-only"""
        mdl = IMP.Model()
        topology_file = self.get_input_file_name("topology_beads.txt")
        t = IMP.pmi.topology.TopologyReader(topology_file)
        with IMP.allow_deprecated():
            bm = IMP.pmi.macros.BuildModel(mdl,
                                           component_topologies=t.component_list)
        rep = bm.get_representation()
        p1 = IMP.pmi.tools.select(rep,name='detgnt')
        p2 = IMP.pmi.tools.select(rep,name='pom152')
        self.assertTrue(IMP.core.Gaussian.get_is_setup(p1[0]))
        self.assertTrue(IMP.core.Gaussian.get_is_setup(p2[0]))

    def test_build_system(self):
        """Test the new BuildSystem macro"""
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
        sel1 = IMP.atom.Selection(root_hier,molecule="Rpb1",resolution=1).get_selected_particles()
        r1m = [1,8,10,10,10] #missing region lengths
        self.assertEqual(len(sel1),(1274-sum(r1m)+sum([int(math.ceil(float(r)/5)) for r in r1m])))

        sel3 = IMP.atom.Selection(root_hier,molecule="Rpb3",resolution=10).get_selected_particles()
        self.assertEqual(len(sel3),int(math.ceil(318.0/10)))

        sel4 = IMP.atom.Selection(root_hier,molecule="Rpb4",
                                  representation_type=IMP.atom.DENSITIES).get_selected_particles()
        self.assertEqual(len(sel4),5)

        # check rigid bodies
        rbs = dof.get_rigid_bodies()
        fbs = dof.get_flexible_beads()
        self.assertEqual(len(rbs),2)

if __name__=="__main__":
    IMP.test.main()
