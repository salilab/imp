from __future__ import print_function, division
import IMP
import IMP.test
import IMP.core
import IMP.pmi
import IMP.pmi.representation
import IMP.pmi.samplers
import IMP.pmi.macros
import IMP.pmi.restraints
import IMP.pmi.restraints.stereochemistry
import IMP.container
import IMP.algebra
import IMP.rmf
import RMF
import shutil

# initialize Replica Exchange class
try:
    import IMP.mpi
    rem = IMP.mpi.ReplicaExchange()
except ImportError:
    rem = IMP.pmi.samplers._SerialReplicaExchange()

class Tests(IMP.test.TestCase):
    def test_xyz_particles(self):
        m=IMP.Model()
        ps=[]
        hs=[]
        for i in range(10):
            p=IMP.Particle(m)
            h=IMP.atom.Hierarchy.setup_particle(p)
            d=IMP.core.XYZR.setup_particle(p)
            IMP.atom.Mass.setup_particle(p,1.0)
            d.set_coordinates(IMP.algebra.get_random_vector_in(
                                    IMP.algebra.Sphere3D((0,0,0),10)))
            d.set_radius(1)
            ps.append(p)
            hs.append(h)

        srbm = IMP.pmi.TransformMover(m, 1, 0.5)

        #origin point
        origin=IMP.Particle(m)
        d=IMP.core.XYZR.setup_particle(origin)
        d.set_coordinates((0,0,0))
        d.set_radius(1)

        rs = IMP.RestraintSet(m)
        ts1 = IMP.core.Harmonic(10,10)
        dr=IMP.core.DistanceRestraint(m, ts1,
                                       ps[0],origin)


        for p in ps:
            srbm.add_xyz_particle(IMP.core.XYZ(p))
        smv = IMP.core.SerialMover([srbm])
        mc = IMP.core.MonteCarlo(m)
        mc.set_scoring_function([dr])
        mc.set_return_best(False)
        mc.set_kt(1.0)
        mc.add_mover(smv)

        rh = RMF.create_rmf_file("test_transform_mover_xyz.rmf3")
        IMP.rmf.add_hierarchies(rh, hs)

        for n in range(10):
            mc.optimize(1)
            IMP.rmf.save_frame(rh)

        del rh
        #os.unlink("test_transform_mover_xyz.rmf3")


    def test_rigid_body_particles(self):
        m=IMP.Model()
        ps=[]
        hs=[]
        for i in range(10):
            p=IMP.Particle(m)
            h=IMP.atom.Hierarchy.setup_particle(p)
            d=IMP.core.XYZR.setup_particle(p)
            IMP.atom.Mass.setup_particle(p,1.0)
            d.set_coordinates(IMP.algebra.get_random_vector_in(
                                    IMP.algebra.Sphere3D((0,0,0),10)))
            d.set_radius(1)
            ps.append(p)
            hs.append(h)

        p=IMP.Particle(m)
        rb=IMP.core.RigidBody.setup_particle(p,ps)

        srbm = IMP.pmi.TransformMover(m, 1, 0.5)

        #origin point
        origin=IMP.Particle(m)
        d=IMP.core.XYZR.setup_particle(origin)
        d.set_coordinates((0,0,0))
        d.set_radius(1)

        rs = IMP.RestraintSet(m)
        ts1 = IMP.core.Harmonic(10,10)
        dr=IMP.core.DistanceRestraint(m, ts1,
                                       ps[0],origin)


        #for p in ps:
        #    srbm.add_xyz_particle(p)
        srbm.add_rigid_body_particle(IMP.core.RigidBody(rb))
        smv = IMP.core.SerialMover([srbm])
        mc = IMP.core.MonteCarlo(m)
        mc.set_scoring_function([dr])
        mc.set_return_best(False)
        mc.set_kt(1.0)
        mc.add_mover(smv)

        rh = RMF.create_rmf_file("test_transform_mover_rigid_body.rmf3")
        IMP.rmf.add_hierarchies(rh, hs)

        for n in range(10):
            mc.optimize(1)
            IMP.rmf.save_frame(rh)
        del rh
        #os.unlink("test_transform_mover_rigid_body.rmf3")


    def test_xyz_particles_rotamer(self):
        m=IMP.Model()
        ps=[]
        hs=[]
        for i in range(4):
            p=IMP.Particle(m)
            h=IMP.atom.Hierarchy.setup_particle(p)
            d=IMP.core.XYZR.setup_particle(p)
            IMP.atom.Mass.setup_particle(p,1.0)
            d.set_coordinates((float(i)*2.0,0,0))
            d.set_radius(1.0)
            ps.append(p)
            hs.append(h)

        for i in range(5,10):
            p=IMP.Particle(m)
            h=IMP.atom.Hierarchy.setup_particle(p)
            d=IMP.core.XYZR.setup_particle(p)
            IMP.atom.Mass.setup_particle(p,1.0)
            d.set_coordinates((float(i)*2.0,float(i-5)*2.0,0))
            d.set_radius(1.0)
            ps.append(p)
            hs.append(h)

        srbm = IMP.pmi.TransformMover(m, ps[3],ps[4], 0.0, 0.2)

        #origin point
        origin=IMP.Particle(m)
        d=IMP.core.XYZR.setup_particle(origin)
        d.set_coordinates((0,0,0))
        d.set_radius(1.0)

        rs = IMP.RestraintSet(m)
        ts1 = IMP.core.Harmonic(10,10)
        dr=IMP.core.DistanceRestraint(m, ts1,
                                       ps[0],origin)


        for p in ps[4:]:
            srbm.add_xyz_particle(IMP.core.XYZ(p))
        smv = IMP.core.SerialMover([srbm])
        mc = IMP.core.MonteCarlo(m)
        mc.set_scoring_function([dr])
        mc.set_return_best(False)
        mc.set_kt(1.0)
        mc.add_mover(smv)

        rh = RMF.create_rmf_file("test_transform_mover_xyz_rotamer.rmf3")
        IMP.rmf.add_hierarchies(rh, hs)

        for n in range(10):
            mc.optimize(10)
            IMP.rmf.save_frame(rh)

        del rh

    def test_pmi_representation_sampling_macro1_helix(self):

        rbmaxtrans = 3.00
        fbmaxtrans = 3.00
        rbmaxrot=0.1
        srbmaxtrans = 100.0
        srbmaxrot=0.2
        outputobjects = []
        sampleobjects = []

        # setting up topology
        m=IMP.Model()
        s = IMP.pmi.topology.System(m)
        st = s.create_state()

        fastafile = self.get_input_file_name("1WCM.fasta.txt")
        sequences = IMP.pmi.topology.Sequences(fastafile)

        mol = st.create_molecule("A", sequence=sequences['1WCM:A'])
        mol.add_representation(mol[:101] | mol[151:251], resolutions=[20])
        mol.add_representation(mol[101:151], ideal_helix=True, resolutions=[50])

        root_hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(m)

        dof.create_flexible_beads(mol[:101] | mol[151:251],
                                 max_trans=fbmaxtrans)
        dof.create_rigid_body(mol[101:151], max_trans=rbmaxtrans,
                              max_rot=rbmaxrot)
        dof.create_super_rigid_body(mol, max_trans=srbmaxtrans,
                                    max_rot=srbmaxrot)

        cr = IMP.pmi.restraints.stereochemistry.ConnectivityRestraint(mol)
        cr.add_to_model()
        outputobjects.append(cr)

        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
                                    included_objects=mol, resolution=10)
        ev.add_to_model()
        outputobjects.append(ev)


        mc2=IMP.pmi.macros.ReplicaExchange0(m,
                                    root_hier=root_hier,
                                    monte_carlo_sample_objects=dof.get_movers(),
                                    output_objects=outputobjects,
                                    monte_carlo_temperature=1.0,
                                    replica_exchange_minimum_temperature=1.0,
                                    replica_exchange_maximum_temperature=2.5,
                                    number_of_best_scoring_models=0,
                                    monte_carlo_steps=1,
                                    number_of_frames=10,
                                    write_initial_rmf=True,
                                    initial_rmf_name_suffix="initial",
                                    stat_file_name_suffix="stat",
                                    best_pdb_name_suffix="model",
                                    do_clean_first=True,
                                    do_create_directories=True,
                                    global_output_directory="test_transform_mover_output_2",
                                    rmf_dir="rmfs/",
                                    best_pdb_dir="pdbs/",
                                    replica_stat_file_suffix="stat_replica",
                                    replica_exchange_object=rem)
        mc2.execute_macro()
        #shutil.rmtree("test_transform_mover_output_2")


if __name__ == '__main__':
    IMP.test.main()
