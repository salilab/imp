import IMP
import IMP.test
import sys
import IMP.em
import IMP.multifit
import IMP.restrainer
import os
import time

class Tests(IMP.test.TestCase):
    """Tests for sampled density maps"""

    def setUp(self):
        """initialize IMP environment create particles"""
        IMP.test.TestCase.setUp(self)
        #init IMP model ( the environment)
        self.mdl = IMP.Model()
        self.sel=IMP.atom.CAlphaPDBSelector()
        self.mhs=IMP.atom.Hierarchies()
        self.mhs.append(IMP.atom.read_pdb(self.get_input_file_name("1z5s_A.pdb"),
                             self.mdl,self.sel))
        self.mhs.append(IMP.atom.read_pdb(self.get_input_file_name("1z5s_C1.pdb"),
                             self.mdl,self.sel))
        self.voxel_size=1.
        for mh in self.mhs:
            IMP.atom.add_radii(mh)
            IMP.multifit.add_surface_index(mh,self.voxel_size)
        self.rbs=IMP.restrainer.set_rigid_bodies(self.mhs)
        #set the restraint
        sr=IMP.restrainer.create_simple_connectivity_on_rigid_bodies(self.rbs,
                                                                 IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits()))

        print "going to evaluate 2"
        self.mdl.evaluate(False)
        self.wev_r=IMP.multifit.create_weighted_excluded_volume_restraint(
           self.rbs[0],self.rbs[1])

        print "going to evaluate 1"
        self.mdl.evaluate(False)
        self.c_r=sr.get_restraint()
        self.mdl.add_restraint(self.c_r)
        print "going to evaluate"
        self.mdl.evaluate(False)
        print "end setup"
    def test_weighted_excluded_volume_restraint(self):

        """Check that weighted excluded volume restraint works"""
        self.assertEqual(self.mdl.get_number_of_restraints(), 2,
                         "the excluded volume restraint was not added")
        #IMP.base.set_log_level(IMP.base.VERBOSE)
        rotations=[[0.960739,0.177613,-0.196201,0.0833023],
                   [0.98373,-0.0268444,-0.115434,-0.135015],
                   [0.995413,0.0545123,-0.0635521,0.0462946],
                   [0.99739,0.0503421,-0.000272958,-0.0517479],
                   [0.953478,0.148336,-0.198021,0.17223],
                   [0.994239,0.0570374,-0.0140131,0.089658],
                   [0.98401,0.148801,-0.00937242,0.0974403],
                   [0.864194,-0.251682,0.325705,-0.28938],
                   [0.952222,-0.155235,-0.253447,-0.0702893],
                   [0.881024,-0.405223,0.154964,-0.188619]]
        translations=[IMP.algebra.Vector3D(7.37269, -32.1143, -68.123),
                      IMP.algebra.Vector3D(-26.968, 30.7037, -18.9437),
                      IMP.algebra.Vector3D(-0.990538, -13.8685,-18.359),
                      IMP.algebra.Vector3D(-4.79513, 3.45006, -9.39788),
                      IMP.algebra.Vector3D(18.2439, -40.6508, -62.0047),
                      IMP.algebra.Vector3D(10.7605, -26.9859, -11.0509),
                      IMP.algebra.Vector3D(13.1573, -34.8041, -26.6549),
                      IMP.algebra.Vector3D(46.1903, 87.4569, 58.9899),
                      IMP.algebra.Vector3D(-32.0694, 24.0887, -33.828),
                      IMP.algebra.Vector3D(20.1398, 111.715, 60.5263)]
        #move chain A and calculate weighted excluded volume
        ps1=IMP.core.get_leaves(self.mhs[0])
        IMP.base.set_log_level(IMP.base.SILENT)#VERBOSE)
        for i in range(10):
            t=IMP.algebra.Transformation3D(
                IMP.algebra.Rotation3D(rotations[i][0],rotations[i][1],rotations[i][2],rotations[i][3]),
                translations[i])
            xyz=IMP.core.XYZ(self.rbs[0].get_particle())
            xyz.set_coordinates(t.get_transformed(xyz.get_coordinates()))
            #check that when the proteins are not connected (self.c_r>0) the excluded volume
            #restraint is bigger than 0
            start = time.clock()
            self.mdl.evaluate(False) #to make sure the coordinates were transformed
            end = time.clock()
            print "Time elapsed for PairRestraint evaluatation = ", end - start, "seconds"
            conn_r=self.c_r.evaluate(False)
            w_exc_vol_r=self.wev_r.evaluate(False)
            self.assertTrue(((conn_r==0.)and(w_exc_vol_r>0.))or((conn_r>0.)and(w_exc_vol_r==0.)),
                         "inconsistency between connectivity and excluded volume restraint")
            #print "connectivity "+str(conn_r) + " | excluded-volume " + str(w_exc_vol_r)
            xyz.set_coordinates(t.get_inverse().get_transformed(xyz.get_coordinates()))
if __name__ == '__main__':
    IMP.test.main()
