import RMF

rmf= RMF.create_rmf_file("example.rmf")

colored_factory= RMF.ColoredFactory(rmf)
particle_factory= RMF.ParticleFactory(rmf)
intermediate_particle_factory= RMF.IntermediateParticleFactory(rmf)
rigid_particle_factory = RMF.RigidParticleFactory(rmf)
ball_factory = RMF.BallFactory(rmf)
cylinder_factory = RMF.CylinderFactory(rmf)
segment_factory = RMF.SegmentFactory(rmf)
journal_article_factory= RMF.JournalArticleFactory(rmf)
residue_factory= RMF.ResidueFactory(rmf)
atom_factory= RMF.AtomFactory(rmf)
chain_factory= RMF.ChainFactory(rmf)
domain_factory= RMF.DomainFactory(rmf)
copy_factory= RMF.CopyFactory(rmf)
diffuser_factory= RMF.DiffuserFactory(rmf)
typed_factory= RMF.TypedFactory(rmf)
score_factory= RMF.ScoreFactory(rmf)

# first create a protein fragment
chain= rmf.get_root_node().add_child("chain", RMF.REPRESENTATION)
chain_factory.get(chain).set_chain_id(0)
domain= chain.add_child("n-terminus", RMF.REPRESENTATION)
dd= domain_factory.get(domain)
dd.set_indexes(0,2)

atoms=[]
res_data=[( "MET", 0 , [ ("N", 7 , 14.0067 , 1.85000002384 ,
                          [24.277000427246094, 8.3739995956420898, -9.8540000915527344] ,),
                         ("CA", 6 , 12.0107 , 2.27500009537 ,
                          [24.403999328613281, 9.8590002059936523, -9.939000129699707] ,),
                         ("C", 6 , 12.0107 , 2.0 ,
                          [25.813999176025391, 10.24899959564209, -10.359000205993652] ,),
                         ("O", 8 , 15.9994 , 1.70000004768 ,
                          [26.74799919128418, 9.4689998626708984, -10.196999549865723] ,),
                         ("CB", 6 , 12.0107 , 2.17499995232 ,
                          [24.069999694824219, 10.494999885559082, -8.5959997177124023] ,),
                         ("CG", 6 , 12.0107 , 2.17499995232 ,
                          [24.879999160766602, 9.939000129699707, -7.4419999122619629] ,),
                         ("SD", 16 , 32.065 , 2.0 ,
                          [24.261999130249023, 10.555000305175781, -5.8730001449584961] ,),
                         ("CE", 6 , 12.0107 , 2.05999994278 ,
                          [24.822000503540039, 12.265999794006348, -5.9670000076293945] ,),
                         ],),
          ( "VAL", 1 , [ ("N", 7 , 14.0067 , 1.85000002384 ,
                          [25.964000701904297, 11.453000068664551, -10.902999877929688] ,),
                         ("CA", 6 , 12.0107 , 2.27500009537 ,
                          [27.26300048828125, 11.923999786376953, -11.359000205993652] ,),
                         ("C", 6 , 12.0107 , 2.0 ,
                          [27.392000198364258, 13.428000450134277, -11.114999771118164] ,),
                         ("O", 8 , 15.9994 , 1.70000004768 ,
                          [26.443000793457031, 14.184000015258789, -11.326999664306641] ,),
                         ("CB", 6 , 12.0107 , 2.27500009537 ,
                          [27.454999923706055, 11.630999565124512, -12.878000259399414] ,),
                         ("CG1", 6 , 12.0107 , 2.05999994278 ,
                          [28.756000518798828, 12.208999633789062, -13.381999969482422] ,),
                         ("CG2", 6 , 12.0107 , 2.05999994278 ,
                          [27.431999206542969, 10.130999565124512, -13.140000343322754] ,),
                         ],),
          ( "LEU", 2 , [ ("N", 7 , 14.0067 , 1.85000002384 ,
                          [28.555000305175781, 13.854999542236328, -10.63599967956543] ,),
                         ("CA", 6 , 12.0107 , 2.27500009537 ,
                          [28.797000885009766, 15.269000053405762, -10.390000343322754] ,),
                         ("C", 6 , 12.0107 , 2.0 , [29.492000579833984, 15.902999877929688, -11.585000038146973] ,),
                         ("O", 8 , 15.9994 , 1.70000004768 ,
                          [30.25, 15.239999771118164, -12.305999755859375] ,),
                         ("CB", 6 , 12.0107 , 2.17499995232 ,
                          [29.687999725341797, 15.470000267028809, -9.1520004272460938] ,),
                         ("CG", 6 , 12.0107 , 2.27500009537 ,
                          [29.083999633789062, 15.416000366210938, -7.750999927520752] ,),
                         ("CD1", 6 , 12.0107 , 2.05999994278 ,
                          [28.729999542236328, 13.98799991607666, -7.3899998664855957] ,),
                         ("CD2", 6 , 12.0107 , 2.05999994278 ,
                          [30.084999084472656, 16.007999420166016, -6.7760000228881836] ,),]),
]

for rdata in res_data:
    r=domain.add_child(rdata[0], RMF.REPRESENTATION);
    rd= residue_factory.get(r)
    rd.set_type(rdata[0])
    rd.set_index(rdata[1])
    for adata in rdata[2]:
        a= r.add_child(adata[0], RMF.REPRESENTATION)
        atoms.append(a)
        ad= atom_factory.get(a)
        ad.set_element(adata[1])
        ad.set_mass(adata[2])
        ad.set_radius(adata[3])
        for i in range(0,4):
            adpf= atom_factory.get(a, i)
            adpf.set_coordinates([adata[4][0]+i,
                                  adata[4][1]+i,
                                  adata[4][2]+i])
bond_data=[(1,4),(0,1),(1,2),(2,3),(4,5),(5,6),(6,7),(2,8),(9,12),(8,9),(9,10),(10,11),(12,13),(12,14),(10,15),(16,19),(15,16),(16,17),(17,18),(19,20),(20,21),(20,22)]

for b in bond_data:
    rmf.add_node_pair([atoms[b[0]], atoms[b[1]]], RMF.BOND)
