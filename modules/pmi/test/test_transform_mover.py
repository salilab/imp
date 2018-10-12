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
        #os.unlink("test_transform_mover_xyz_rotamer.rmf3")

    """
    def test_pmi_representation_sampling_macro1(self):
        import IMP
        import IMP.pmi
        import IMP.pmi.representation
        import IMP.pmi.restraints
        import IMP.pmi.restraints.stereochemistry
        import IMP.pmi.macros

        rbmaxtrans = 5.00
        fbmaxtrans = 3.00
        rbmaxrot=0.05
        outputobjects = []
        sampleobjects = []

        # setting up topology
        m=IMP.Model()
        simo = IMP.pmi.representation.Representation(m,upperharmonic=True,disorderedlength=True)

        pdbfile = self.get_input_file_name("1WCM.pdb")
        fastafile = self.get_input_file_name("1WCM.fasta.txt")
        components = ["Rpb1","Rpb2","Rpb3","Rpb4","Rpb5"]
        chains = "ABCD"
        colors = [0.,0.1,0.5,1.0]
        beadsize = 20
        fastids = IMP.pmi.tools.get_ids_from_fasta_file(fastafile)

        domains=[]
        for n in [2,3]:
            domains+=[(components[n],  components[n],   colors[n],  fastafile,  "1WCM:"+chains[n],   pdbfile,   chains[n],    (1,-1,0),  None, 20,       n,      [0,n],     None,   None,  None,   None)]
        bm=IMP.pmi.macros.BuildModel1(simo)
        bm.build_model(domains,sequence_connectivity_scale=1.0)

        simo.set_rigid_bodies_max_rot(rbmaxrot)
        simo.set_floppy_bodies_max_trans(fbmaxtrans)
        simo.set_rigid_bodies_max_trans(rbmaxtrans)

        outputobjects.append(simo)
        sampleobjects.append(simo)

        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(simo,resolution=10)
        ev.add_to_model()
        outputobjects.append(ev)


        mc2=IMP.pmi.macros.ReplicaExchange0(m,
                                    simo,
                                    monte_carlo_sample_objects=sampleobjects,
                                    output_objects=outputobjects,
                                    monte_carlo_temperature=1.0,
                                    replica_exchange_minimum_temperature=1.0,
                                    replica_exchange_maximum_temperature=2.5,
                                    number_of_best_scoring_models=0,
                                    monte_carlo_steps=10,
                                    number_of_frames=1000,
                                    write_initial_rmf=True,
                                    initial_rmf_name_suffix="initial",
                                    stat_file_name_suffix="stat",
                                    best_pdb_name_suffix="model",
                                    do_clean_first=True,
                                    do_create_directories=True,
                                    global_output_directory="test_transform_mover_output_1",
                                    rmf_dir="rmfs/",
                                    best_pdb_dir="pdbs/",
                                    replica_stat_file_suffix="stat_replica",
                                    replica_exchange_object=rem)
        mc2.execute_macro()
        #shutil.rmtree("test_transform_mover_output_1")
    """
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
        with IMP.allow_deprecated():
            simo = IMP.pmi.representation.Representation(m,upperharmonic=True,disorderedlength=True)

        pdbfile = self.get_input_file_name("1WCM.pdb")
        fastafile = self.get_input_file_name("1WCM.fasta.txt")
        components = ["Rpb1","Rpb2","Rpb3","Rpb4","Rpb5"]
        chains = "ABCD"
        colors = [0.,0.1,0.5,1.0]
        beadsize = 20
        fastids = IMP.pmi.tools.get_ids_from_fasta_file(fastafile)



        domains=[("A",  "A.1",    0.10, fastafile,  "1WCM:A",   "BEADS",         None,    (1,100,0),       None,       20,         None,      [0],     None,   None,  None,   None),
                 ("A",  "A.2",    0.10, fastafile,  "1WCM:A",   "IDEAL_HELIX",   None,    (101,150,0),     None,       20,         0,      [0],     None,   None,  None,   None),
                 ("A",  "A.3",    0.10, fastafile,  "1WCM:A",   "BEADS",         None,    (151,250,0),     None,       20,         None,      [0],     None,   None,  None,   None)]


        with IMP.allow_deprecated():
            bm=IMP.pmi.macros.BuildModel1(simo)
        bm.build_model(domains,sequence_connectivity_scale=1.0)

        simo.set_rigid_bodies_max_rot(rbmaxrot)
        simo.set_floppy_bodies_max_trans(fbmaxtrans)
        simo.set_rigid_bodies_max_trans(rbmaxtrans)
        simo.set_super_rigid_bodies_max_rot(srbmaxrot)
        simo.set_super_rigid_bodies_max_trans(srbmaxtrans)


        outputobjects.append(simo)
        sampleobjects.append(simo)

        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(simo,resolution=10)
        ev.add_to_model()
        outputobjects.append(ev)


        mc2=IMP.pmi.macros.ReplicaExchange0(m,
                                    simo,
                                    monte_carlo_sample_objects=sampleobjects,
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



    def test_pmi_representation_sampling_mainchain_mover(self):
        rbmaxtrans = 3.00
        fbmaxtrans = 3.00
        rbmaxrot=0.1
        srbmaxtrans = 0.01
        srbmaxrot=0.1
        outputobjects = []
        sampleobjects = []

        # setting up topology
        m=IMP.Model()
        with IMP.allow_deprecated():
            simo = IMP.pmi.representation.Representation(m,upperharmonic=True,disorderedlength=True)

        pdbfile = self.get_input_file_name("1WCM.pdb")
        fastafile = self.get_input_file_name("1WCM.fasta.txt")
        components = ["Rpb1","Rpb2","Rpb3","Rpb4","Rpb5"]
        chains = "ABCD"
        colors = [0.,0.1,0.5,1.0]
        beadsize = 20
        fastids = IMP.pmi.tools.get_ids_from_fasta_file(fastafile)



        domains=[("A",  "A.1",    0.10, fastafile,  "1WCM:A",   "BEADS",         None,    (1,50,0),       None,       1,         None,      None,     None,   None,  None,   None)]

        with IMP.allow_deprecated():
            bm=IMP.pmi.macros.BuildModel1(simo)
        bm.build_model(domains,sequence_connectivity_scale=1.0)
        bm.set_main_chain_mover("A.1")

        simo.set_rigid_bodies_max_rot(rbmaxrot)
        simo.set_floppy_bodies_max_trans(fbmaxtrans)
        simo.set_rigid_bodies_max_trans(rbmaxtrans)
        simo.set_super_rigid_bodies_max_rot(srbmaxrot)
        simo.set_super_rigid_bodies_max_trans(srbmaxtrans)
        simo.shuffle_configuration(max_translation=100)

        outputobjects.append(simo)
        sampleobjects.append(simo)

        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(simo,resolution=10)
        ev.add_to_model()
        outputobjects.append(ev)


        mc2=IMP.pmi.macros.ReplicaExchange0(m,
                                    simo,
                                    monte_carlo_sample_objects=sampleobjects,
                                    output_objects=outputobjects,
                                    monte_carlo_temperature=1.0,
                                    replica_exchange_minimum_temperature=1.0,
                                    replica_exchange_maximum_temperature=2.5,
                                    number_of_best_scoring_models=0,
                                    monte_carlo_steps=1,
                                    number_of_frames=2,
                                    write_initial_rmf=True,
                                    initial_rmf_name_suffix="initial",
                                    stat_file_name_suffix="stat",
                                    best_pdb_name_suffix="model",
                                    do_clean_first=True,
                                    do_create_directories=True,
                                    global_output_directory="test_transform_mover_output_3",
                                    rmf_dir="rmfs/",
                                    best_pdb_dir="pdbs/",
                                    replica_stat_file_suffix="stat_replica",
                                    replica_exchange_object=rem)
        mc2.execute_macro()
        #shutil.rmtree("test_transform_mover_output_3")








if __name__ == '__main__':
    IMP.test.main()
