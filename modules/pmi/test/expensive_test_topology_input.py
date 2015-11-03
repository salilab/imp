import IMP
import IMP.pmi
import IMP.pmi.topology
import IMP.pmi.macros
import os
import IMP.test
import IMP.rmf
import RMF

def children_as_dict(h):
    cdict={}
    for c in h.get_children():
        cdict[c.get_name()]=c
    return cdict

class TopologyReaderTests(IMP.test.TestCase):

    def test_reading(self):
        '''Test basic reading'''
        topology_file=self.get_input_file_name("topology.txt")
        t=IMP.pmi.topology.TopologyReader(topology_file)
        self.assertEqual(len(t.component_list),3)
        self.assertEqual(t.component_list[0].domain_name,"Rpb1_1")
        self.assertEqual(t.component_list[1].name,"Rpb1")
        self.assertEqual(t.component_list[2].domain_name,"Rpb4")
        self.assertEqual(t.component_list[2].name,"Rpb4")

    def test_change_dir(self):
        '''Test changing default pdb directory'''
        newdir="../../"
        topology_file=self.get_input_file_name("topology.txt")
        t=IMP.pmi.topology.TopologyReader(topology_file)
        t.set_dir("pdb_dir", newdir)
        self.assertEqual(t.defaults["pdb_dir"], newdir)
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
        '''Test reading and writing'''
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
        '''Test building with macro BuildModel using a topology file'''
        mdl = IMP.Model()

        topology_file=self.get_input_file_name("topology.txt")
        t=IMP.pmi.topology.TopologyReader(topology_file)
        self.assertEqual(t.defaults['gmm_dir'],'./')

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
        self.assertEqual(len(r4dict["Densities"].get_children()[0].get_children()),3)

    def test_build_create_gmms(self):
        '''Test building with macro using sklearn to create stuff'''
        try:
            import sklearn
        except ImportError:
            self.skipTest("no sklearn package")
        mdl = IMP.Model()

        topology_file=self.get_input_file_name("topology.txt")
        t=IMP.pmi.topology.TopologyReader(topology_file)
        t.set_dir("gmm_dir","../")

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
        self.assertEqual(len(r4dict["Densities"].get_children()[0].get_children()),3)
        for output in ['../Rpb4.mrc', '../Rpb4.txt']:
            os.unlink(self.get_input_file_name(output))

    def test_build_with_movers(self):
        '''Check if rigid bodies etc are set up as requested'''
        pass

    def test_beads_only(self):
        '''Test setting up BEADS-only'''
        mdl = IMP.Model()
        topology_file = self.get_input_file_name("topology_beads.txt")
        t = IMP.pmi.topology.TopologyReader(topology_file)
        bm = IMP.pmi.macros.BuildModel(mdl,
                                       component_topologies=t.component_list)
        rep = bm.get_representation()
        p1 = IMP.pmi.tools.select(rep,name='detgnt')
        p2 = IMP.pmi.tools.select(rep,name='pom152')
        self.assertTrue(IMP.core.Gaussian.get_is_setup(p1[0]))
        self.assertTrue(IMP.core.Gaussian.get_is_setup(p2[0]))

if __name__=="__main__":
    IMP.test.main()
