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
        for i in range(3):
            mhs.append(IMP.atom.read_pdb(self.get_input_file_name("small_protein.pdb"),mdl))
            mhs[-1].set_name("prot"+str(i))
            rb=IMP.atom.create_rigid_body(mhs[-1])
            print rb.get_name()
            rbs.append(rb)
        #set the nesting
        for i in range(2):
            rbs[i].add_member(rbs[i+1])
        #set domino states
        pst=IMP.domino.ParticleStatesTable()
        for rb in rbs:
            states=IMP.domino.NestedRigidBodyStates([IMP.algebra.Transformation3D(IMP.algebra.Rotation3D(1.,0.,0.,0.),IMP.algebra.Vector3D(0.,0.,0.))])
            pst.set_particle_states(rb,states)
        print "sample"
        s=IMP.domino.DominoSampler(mdl, pst)
        cs=s.get_sample()
        print "number of configurations",cs.get_number_of_configurations()
        #TODO - check that the transformations are correct

if __name__ == '__main__':
    IMP.test.main()
