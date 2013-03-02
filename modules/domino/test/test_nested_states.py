import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.domino

class Tests(IMP.test.TestCase):
    """Tests for nested RigidBody function"""
    def test_nested2(self):
        """Test nested with transformations"""
        mdl=IMP.Model()
        mhs=[]
        rbs=[]
        orig_rfs=[]
        for i in range(3):
            mhs.append(IMP.atom.read_pdb(self.get_input_file_name("small_protein.pdb"),mdl))
            mhs[-1].set_name("prot"+str(i))
            rb=IMP.atom.create_rigid_body(mhs[-1])
            print rb.get_name()
            rbs.append(rb)
            orig_rfs.append(rb.get_reference_frame())
        #set the nesting
        for i in range(2):
            rbs[i].add_member(rbs[i+1])
        #set domino states
        translation=IMP.algebra.Transformation3D(IMP.algebra.Rotation3D(1.,0.,0.,0.),IMP.algebra.Vector3D(-5.,0.,0.))
        root_rf=rbs[0].get_reference_frame()
        root_to_global=root_rf.get_transformation_from()
        global_to_root=root_rf.get_transformation_to()
        pst=IMP.domino.ParticleStatesTable()
        for i,rb in enumerate(rbs[1:]):
            rb_father=rbs[i]
            states=IMP.domino.NestedRigidBodyStates([IMP.algebra.get_transformation_from_first_to_second(rb.get_reference_frame(),rb_father.get_reference_frame())*translation])
            pst.set_particle_states(rb,states)
        #set states to the root
        pst.set_particle_states(rbs[0],IMP.domino.RigidBodyStates([rbs[0].get_reference_frame()]))
        print "sample"
        s=IMP.domino.DominoSampler(mdl, pst)
        cs=s.get_sample()
        print "number of configurations",cs.get_number_of_configurations()
        #TODO - check that the transformations are correct
        cs.load_configuration(0)
        for i,rb in enumerate(rbs[1:]):
            rmsd = IMP.atom.get_rmsd(IMP.core.XYZs(IMP.core.get_leaves(mhs[0])),
                              IMP.core.XYZs(IMP.core.get_leaves(mhs[i+1])))
            self.assertAlmostEqual(rmsd,5*(i+1),delta=.05)

if __name__ == '__main__':
    IMP.test.main()
