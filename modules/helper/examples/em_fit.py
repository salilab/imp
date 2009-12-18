if False:
    import IMP.em
    import IMP.core
    import IMP.atom
    import IMP.display
    import IMP.helper

if False:
    m= IMP.Model()
    p0= IMP.atom.read_pdb(IMP.get_example_path("1NCA_l_u.pdb"), m)
    p1= IMP.atom.read_pdb(IMP.get_example_path("1NCA_r_u.pdb"), m)
    IMP.atom.add_radii(p0)
    IMP.atom.add_radii(p1)

    sp0= IMP.helper.simplify.create_simplified(p0, 10)
    sp1= IMP.helper.simplify.create_simplified(p1, 20)


    map= IMP.em.read_map(IMP.get_example_path("1NCA.mrc"))
    fr= IMP.em.FitRestraint(map, all_leaves)
