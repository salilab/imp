import RMF

rmf= RMF.create_rmf_file("example.rmf")
nframes=10
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

# add a coase grained molecule
cgchain= rmf.get_root_node().add_child("1", RMF.REPRESENTATION)
cg_bond_scores=cgchain.add_child("bond scores", RMF.FEATURE)
for i in range(0,5):
    r= cgchain.add_child(str(i), RMF.REPRESENTATION)
    rd= residue_factory.get(r)
    rd.set_type("LEU")
    rd.set_index(i)
    pd= particle_factory.get(r)
    pd.set_mass(120)
    pd.set_radius(4)
    cd= colored_factory.get(r)
    cd.set_rgb_color([1,0,0])
    if i > 0:
        bs= cg_bond_scores.add_child(str(i), RMF.FEATURE)
        sd= score_factory.get(bs)
        sd.set_representation([last,r])
    for j in range(0,nframes):
        pdpf= particle_factory.get(r, j)
        pdpf.set_coordinates([0, j*i*4, j*2])
        if i >0:
            sdpf= score_factory.get(bs, j)
            sdpf.set_score(j)
    last=r


# create an atomic protein fragment
chain= rmf.get_root_node().add_child("chain", RMF.REPRESENTATION)
chain_factory.get(chain).set_chain_id(0)
domain= chain.add_child("n-terminus", RMF.REPRESENTATION)
dd= domain_factory.get(domain)
dd.set_indexes(0,2)

atoms=[]
res_data=[( "MET", 0 , [ ("N", 7 , 14.00 , 1.85 ,
                          [24.27, 8.37, -9.85] ),
                         ("CA", 6 , 12.01 , 2.27 ,
                          [24.40, 9.85, -9.93]),
                         ("C", 6 , 12.01 , 2.0 ,
                          [25.81, 10.24, -10.35] ),
                         ("O", 8 , 15.99 , 1.70 ,
                          [26.74, 9.46, -10.19]),
                         ("CB", 6 , 12.0107 , 2.17 ,
                          [24.06, 10.49, -8.59] ),
                         ("CG", 6 , 12.01 , 2.17 ,
                          [24.87, 9.93, -7.44] ),
                         ("SD", 16 , 32.06 , 2.0 ,
                          [24.26, 10.55, -5.87] ),
                         ("CE", 6 , 12.01 , 2.05 ,
                          [24.82, 12.26, -5.96] ),
                         ]),
          ( "VAL", 1 , [ ("N", 7 , 14.00 , 1.85 ,
                          [25.96, 11.45, -10.90] ),
                         ("CA", 6 , 12.01 , 2.27 ,
                          [27.26, 11.92, -11.35] ),
                         ("C", 6 , 12.01 , 2.0 ,
                          [27.39, 13.42, -11.11] ),
                         ("O", 8 , 15.99 , 1.70 ,
                          [26.44, 14.18, -11.32] ),
                         ("CB", 6 , 12.01 , 2.27 ,
                          [27.45, 11.63, -12.87] ),
                         ("CG1", 6 , 12.01 , 2.05 ,
                          [28.75, 12.20, -13.38] ),
                         ("CG2", 6 , 12.01 , 2.05 ,
                          [27.43, 10.13, -13.14] ),
                         ]),
          ( "LEU", 2 , [ ("N", 7 , 14.00 , 1.85 ,
                          [28.55, 13.85, -10.63] ),
                         ("CA", 6 , 12.01 , 2.27 ,
                          [28.79, 15.26, -10.39] ),
                         ("C", 6 , 12.01 , 2.0 ,
                          [29.49, 15.90, -11.58] ),
                         ("O", 8 , 15.99 , 1.70 ,
                          [30.25, 15.23, -12.30] ),
                         ("CB", 6 , 12.01 , 2.17 ,
                          [29.68, 15.47, -9.15] ),
                         ("CG", 6 , 12.01 , 2.27 ,
                          [29.08, 15.41, -7.75] ),
                         ("CD1", 6 , 12.01 , 2.05 ,
                          [28.72, 13.98, -7.38] ),
                         ("CD2", 6 , 12.01 , 2.05 ,
                          [30.08, 16.00, -6.77] )]),
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
        for i in range(0,nframes):
            adpf= atom_factory.get(a, i)
            adpf.set_coordinates([adata[4][0]+i,
                                  adata[4][1]+i,
                                  adata[4][2]+i])
bond_data=[(1,4),(0,1),(1,2),(2,3),(4,5),(5,6),(6,7),(2,8),(9,12),(8,9),(9,10),
           (10,11),(12,13),(12,14),(10,15),(16,19),(15,16),(16,17),(17,18),(19,20),
           (20,21),(20,22)]


bond_scores=chain.add_child("bond scores", RMF.FEATURE)

for b in bond_data:
    eps=[atoms[b[0]], atoms[b[1]]]
    rmf.add_node_pair(eps, RMF.BOND)
    bd= bond_scores.add_child(str(b), RMF.FEATURE)
    sd= score_factory.get(bd)
    sd.set_score(0)
    sd.set_representation(eps)

# now add some geometry floating around
#group it for kicks
dg= rmf.get_root_node().add_child("dynamic geometry", RMF.GEOMETRY)
bn= dg.add_child("ball", RMF.GEOMETRY)
cn= dg.add_child("cylinder", RMF.GEOMETRY)
sn= dg.add_child("segment", RMF.GEOMETRY)

cd= colored_factory.get(bn)
cd.set_rgb_color([float(5)/float(nframes),1,0])
for i in range(1, nframes, 2):
    bd= ball_factory.get(bn, i)
    bd.set_radius(i)
    bd.set_coordinates([10+i, i, i])
    #cd= colored_factory.get(bn)
    #cd.set_rgb_color([float(i)/float(nframes),1,0])
    cd= cylinder_factory.get(cn, i)
    cd.set_radius(i)
    cd.set_coordinates([[10+i, 12+i], [i,i], [i,i]])
    sd= segment_factory.get(sn, i)
    sd.set_coordinates([[12+i, 14+i],[i, i], [i,i]])

sg= rmf.get_root_node().add_child("static geometry", RMF.GEOMETRY)
bn= sg.add_child("ball", RMF.GEOMETRY)
sn= sg.add_child("segment", RMF.GEOMETRY)
cn= sg.add_child("cylinder", RMF.GEOMETRY)
bd= ball_factory.get(bn)
bd.set_radius(3)
bd.set_coordinates([-10, 0, 0])
#cd= colored_factory.get(bn)
#cd.set_rgb_color([0,0,1])

sd= segment_factory.get(sn)
sd.set_coordinates([[-18, -20], [0,0], [0,0]])
cd= cylinder_factory.get(cn)
cd.set_radius(2)
cd.set_coordinates([[-12, -14], [0, 0], [0,0]])
print "done"
