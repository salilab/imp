writer= IMP.display.VRMLWriter()
log= IMP.display.LogOptimizerState(writer, "my_opt.%03d.vrml")

# display each XYZR particle in the SingletonContainer ps as a sphere
ge= IMP.display.XYZRGeometryExtractor()
log.add_geometry(ge, ps)

# display each bond in the SingletonContainer bps as an edge
be= IMP.display.BondGeometryExtractor()
log.add_gemetry(be, bps)
opt.add_optimizer_state(log)
