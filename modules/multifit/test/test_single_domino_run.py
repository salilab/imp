import IMP
import IMP.test
import sys
import IMP.em
import IMP.multifit
import IMP.helper
import os
import time

class SingleDominoRunTests(IMP.test.TestCase):
    """Tests for a domino run on a single mapping"""
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.TERSE)
        self.data_path = os.path.dirname(self.get_input_file_name("assembly.input"))+"/"
        self.settings = IMP.multifit.read_settings(self.get_input_file_name("assembly.input"),self.data_path)
        self.data = IMP.multifit.DataContainer(self.settings)
    def _test_run(self):
        """Check that the correct answer is returned"""
        anchor2comp = IMP.ParticlePairs()
        for i in range(4):
            anchor2comp.append(IMP.ParticlePair(self.data.get_density_anchor_point(i),
                                                self.data.get_component(i).get_particle()))
        single_run=IMP.multifit.SingleDominoRun(self.data)
        num_sols=10
        single_run.set_ev_restraint_weight( 0.2715)
        single_run.set_em_restraint_weight(24.9571)
        aa = single_run.run(anchor2comp,900.,num_sols)
        print "=========="
        print self.data.get_model().evaluate(None)
        print "=========="
        mhs = IMP.atom.Hierarchies()
        mhs_xyz=IMP.core.XYZs()
        for i in range(4):
            mhs.append(self.data.get_component(i))
            mhs_xyz = mhs_xyz + IMP.core.XYZs(IMP.core.get_leaves(mhs[-1]))
        print "=========="
        mhs_ref = IMP.atom.Hierarchies()
        mhs_ref_xyz=IMP.core.XYZs()
        for i in range(4):
            mhs_ref.append(self.data.get_component_reference(i))
            mhs_ref_xyz = mhs_ref_xyz + IMP.core.XYZs(IMP.core.get_leaves(mhs_ref[-1]))

        for i in range(aa.get_number_of_solutions()):
            opt_comb = aa.get_optimum_configuration(i)
            print opt_comb
            aa.move_to_opt_comb(i)
            print "rmsd:",IMP.atom.get_rmsd(mhs_xyz,mhs_ref_xyz)," score:",opt_comb.get_total_score()
            #check that the components did not move ( because the transformations are identity)
        #for i in range(5):
        #    aa.load_configuration(i)
    def _test_run_from_file(self):
        """Check that the correct answer is returned when reading restraint data from file"""
        anchor2comp1 = IMP.ParticlePairs()
        anchor2comp2 = IMP.ParticlePairs()
        for i in range(4):
            anchor2comp1.append(IMP.ParticlePair(self.data.get_density_anchor_point(i),
                                                self.data.get_component(i).get_particle()))
            anchor2comp2.append(IMP.ParticlePair(self.data.get_density_anchor_point(i),
                                                self.data.get_component(3-i).get_particle()))

        single_run=IMP.multifit.SingleDominoRun(self.data)
        single_run.set_restraint_score_directory_path(self.data_path+"scoring/")
        num_sols=10
        aa = single_run.run(anchor2comp1,900.,num_sols,True)
        print "=========="
        print self.data.get_model().evaluate(None)
        print "=========="
        mhs = IMP.atom.Hierarchies()
        mhs_xyz=IMP.core.XYZs()
        for i in range(4):
            mhs.append(self.data.get_component(i))
            mhs_xyz = mhs_xyz + IMP.core.XYZs(IMP.core.get_leaves(mhs[-1]))
        print "=========="
        mhs_ref = IMP.atom.Hierarchies()
        mhs_ref_xyz=IMP.core.XYZs()
        for i in range(4):
            mhs_ref.append(self.data.get_component_reference(i))
            mhs_ref_xyz = mhs_ref_xyz + IMP.core.XYZs(IMP.core.get_leaves(mhs_ref[-1]))

        for i in range(aa.get_number_of_solutions()):
            opt_comb = aa.get_optimum_configuration(i)
            print opt_comb
            aa.move_to_opt_comb(i)
            print "rmsd:",IMP.atom.get_rmsd(mhs_xyz,mhs_ref_xyz)," score:",opt_comb.get_total_score()

        #second
        print "start second"
        aa = single_run.run(anchor2comp2,900.,num_sols,True)
        print "=========="
        print self.data.get_model().evaluate(None)
        print "=========="
        mhs = IMP.atom.Hierarchies()
        mhs_xyz=IMP.core.XYZs()
        for i in range(4):
            mhs.append(self.data.get_component(i))
            mhs_xyz = mhs_xyz + IMP.core.XYZs(IMP.core.get_leaves(mhs[-1]))
        print "=========="
        mhs_ref = IMP.atom.Hierarchies()
        mhs_ref_xyz=IMP.core.XYZs()
        for i in range(4):
            mhs_ref.append(self.data.get_component_reference(i))
            mhs_ref_xyz = mhs_ref_xyz + IMP.core.XYZs(IMP.core.get_leaves(mhs_ref[-1]))

        for i in range(aa.get_number_of_solutions()):
            opt_comb = aa.get_optimum_configuration(i)
            print opt_comb
            aa.move_to_opt_comb(i)
            print "rmsd:",IMP.atom.get_rmsd(mhs_xyz,mhs_ref_xyz)," score:",opt_comb.get_total_score()
            #check that the components did not move ( because the transformations are identity)
        #for i in range(5):
        #    aa.set_configuration(i)
if __name__ == '__main__':
    IMP.test.main()
