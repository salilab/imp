import RMF

name = RMF._get_temporary_file_path("domain.rmf")
fh = RMF.create_rmf_file(name)

df = RMF.DomainFactory(fh)

dd = df.get(fh.get_root_node().add_child("domain", RMF.REPRESENTATION))
input = [0, 10]
dd.set_residue_indexes(*input)
output = dd.get_residue_indexes()
assert(input[0] == output[0])
assert(input[1] == output[1])
