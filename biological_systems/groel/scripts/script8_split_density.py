import IMP.em
density_fn="data/em_maps/groel-11.5A.mrc"
map_origin_x=-135
map_origin_y=-135
map_origin_z=-135
mrw=IMP.em.MRCReaderWriter()
dmap=IMP.em.read_map(density_fn,mrw)
dmap.set_origin(IMP.algebra.Vector3D(map_origin_x,map_origin_y,map_origin_z))
dmap.show()
#split on Z
dmap_top=IMP.em.get_segment(dmap,0,99,0,99,0,50)
dmap_bottom=IMP.em.get_segment(dmap,0,99,0,99,51,99)
dmap_top.set_origin(IMP.algebra.Vector3D(-135,-135,-135))
dmap_bottom.set_origin(IMP.algebra.Vector3D(-135,-135,dmap.get_spacing()))
IMP.em.write_map(dmap_top,"output/groel-11.5A.top.mrc",mrw)
IMP.em.write_map(dmap_bottom,"output/groel-11.5A.bottom.mrc",mrw)
