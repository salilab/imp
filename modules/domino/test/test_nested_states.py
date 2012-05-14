import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.domino

class RBDTests(IMP.test.TestCase):
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
            states=IMP.domino.NestedRigidBodyStates([rb_father.get_reference_frame().get_transformation_to()*translation])
            pst.set_particle_states(rb,states)
        #set states to the root
        pst.set_particle_states(rbs[0],IMP.domino.RigidBodyStates([rbs[0].get_reference_frame()]))
        print "sample"
        s=IMP.domino.DominoSampler(mdl, pst)
        cs=s.get_sample()
        print "number of configurations",cs.get_number_of_configurations()
        #TODO - check that the transformations are correct
        cs.load_configuration(0)
        for i,rb in enumerate(rbs):
            trans1=rb.get_reference_frame().get_transformation_to()
            trans2=orig_rfs[i].get_transformation_to()
            diff_trans=trans1/trans2
            print rb.get_name(),diff_trans
            self.assertAlmostEqual(IMP.algebra.get_distance(
                diff_trans.get_translation(),
                i*IMP.algebra.Vector3D(-5.,0,0)),0, delta=.05)
if __name__ == '__main__':
    IMP.test.main()
