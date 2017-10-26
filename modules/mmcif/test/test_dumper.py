from __future__ import print_function
import IMP.test
import IMP.mmcif
import IMP.mmcif.dumper
import IMP.mmcif.format
import io
import sys
if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO

class Tests(IMP.test.TestCase):
    def make_model(self, system, chains=None):
        if chains is None:
            chains = (('foo', 'ACGT', 'A'), ('bar', 'ACGT', 'B'),
                      ('baz', 'ACC', 'C'))
        s = IMP.mmcif.State(system)
        m = s.model
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        for name, seq, cid in chains:
            h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
            mol = IMP.atom.Molecule.setup_particle(h)
            mol.set_name(name)
            top.add_child(mol)

            h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
            chain = IMP.atom.Chain.setup_particle(h, cid)
            chain.set_sequence(seq)
            mol.add_child(chain)
        return top, s

    def test_entry_dumper(self):
        """Test EntryDumper"""
        system = IMP.mmcif.System()
        dumper = IMP.mmcif.dumper._EntryDumper()
        fh = StringIO()
        writer = IMP.mmcif.format._CifWriter(fh)
        dumper.dump(system, writer)

        out = fh.getvalue()
        self.assertEqual(out, "data_imp_model\n_entry.id imp_model\n")

    def test_chem_comp_dumper(self):
        """Test ChemCompDumper"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system)
        state.add_hierarchy(h)
        dumper = IMP.mmcif.dumper._ChemCompDumper()
        fh = StringIO()
        writer = IMP.mmcif.format._CifWriter(fh)
        dumper.dump(system, writer)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_chem_comp.id
_chem_comp.type
ALA 'L-peptide linking'
CYS 'L-peptide linking'
GLY 'L-peptide linking'
THR 'L-peptide linking'
#
""")

    def test_entity_dumper(self):
        """Test EntityDumper"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system)
        state.add_hierarchy(h)
        dumper = IMP.mmcif.dumper._EntityDumper()
        fh = StringIO()
        writer = IMP.mmcif.format._CifWriter(fh)
        dumper.dump(system, writer)

        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_entity.id
_entity.type
_entity.src_method
_entity.pdbx_description
_entity.formula_weight
_entity.pdbx_number_of_molecules
_entity.details
1 polymer man foo ? 1 ?
2 polymer man baz ? 1 ?
#
""")

    def test_entity_poly_dumper(self):
        """Test EntityPolyDumper"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system)
        state.add_hierarchy(h)
        dumper = IMP.mmcif.dumper._EntityPolyDumper()
        fh = StringIO()
        writer = IMP.mmcif.format._CifWriter(fh)
        dumper.dump(system, writer)

        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_entity_poly.entity_id
_entity_poly.type
_entity_poly.nstd_linkage
_entity_poly.nstd_monomer
_entity_poly.pdbx_strand_id
_entity_poly.pdbx_seq_one_letter_code
_entity_poly.pdbx_seq_one_letter_code_can
1 polypeptide(L) no no A ACGT ACGT
2 polypeptide(L) no no C ACC ACC
#
""")

    def test_entity_poly_seq_dumper(self):
        """Test EntityPolySeqDumper"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system)
        state.add_hierarchy(h)
        dumper = IMP.mmcif.dumper._EntityPolySeqDumper()
        fh = StringIO()
        writer = IMP.mmcif.format._CifWriter(fh)
        dumper.dump(system, writer)

        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_entity_poly_seq.entity_id
_entity_poly_seq.num
_entity_poly_seq.mon_id
_entity_poly_seq.hetero
1 1 ALA .
1 2 CYS .
1 3 GLY .
1 4 THR .
2 1 ALA .
2 2 CYS .
2 3 CYS .
#
""")

    def test_struct_asym_dumper(self):
        """Test StructAsymDumper"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system)
        state.add_hierarchy(h)
        dumper = IMP.mmcif.dumper._StructAsymDumper()
        fh = StringIO()
        writer = IMP.mmcif.format._CifWriter(fh)
        dumper.dump(system, writer)

        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_struct_asym.id
_struct_asym.entity_id
_struct_asym.details
A 1 foo
B 1 bar
C 2 baz
#
""")

    def test_assembly_dumper_get_subassembly(self):
        """Test AssemblyDumper.get_subassembly()"""
        d = IMP.mmcif.dumper._AssemblyDumper()
        complete = IMP.mmcif.data._Assembly(['a', 'b', 'c'])
        d.add(complete)
        x1 = d.get_subassembly({'a':None, 'b':None})
        x2 = d.get_subassembly({'a':None, 'b':None, 'c':None})
        d.finalize() # assign IDs to all assemblies
        self.assertEqual(complete.id, 1)
        self.assertEqual(x1.id, 2)
        self.assertEqual(x1, ['a', 'b'])
        self.assertEqual(x2.id, 1)

    def test_assembly_all_modeled(self):
        """Test AssemblyDumper, all components modeled"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system, (("foo", "AAA", 'A'),
                                            ("bar", "AAA", 'B'),
                                            ("baz", "AA", 'C')))
        state.add_hierarchy(h)
        foo, bar, baz = state._all_modeled_components
        d = IMP.mmcif.dumper._AssemblyDumper()
        fh = StringIO()
        writer = IMP.mmcif.format._CifWriter(fh)

        d.add(IMP.mmcif.data._Assembly((foo, bar)))
        d.add(IMP.mmcif.data._Assembly((bar, baz)))

        d.finalize()
        d.dump(system, writer)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_struct_assembly.ordinal_id
_ihm_struct_assembly.assembly_id
_ihm_struct_assembly.parent_assembly_id
_ihm_struct_assembly.entity_description
_ihm_struct_assembly.entity_id
_ihm_struct_assembly.asym_id
_ihm_struct_assembly.seq_id_begin
_ihm_struct_assembly.seq_id_end
1 1 1 foo 1 A 1 3
2 1 1 foo 1 B 1 3
3 2 2 foo 1 B 1 3
4 2 2 baz 2 C 1 2
#
""")

    def test_assembly_subset_modeled(self):
        """Test AssemblyDumper, subset of components modeled"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system, (("foo", "AAA", 'A'),))
        state.add_hierarchy(h)
        system.add_non_modeled_chain(name="bar", sequence="AA")
        d = system.assembly_dump
        fh = StringIO()
        writer = IMP.mmcif.format._CifWriter(fh)

        d.finalize() # assign IDs
        d.dump(system, writer)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_struct_assembly.ordinal_id
_ihm_struct_assembly.assembly_id
_ihm_struct_assembly.parent_assembly_id
_ihm_struct_assembly.entity_description
_ihm_struct_assembly.entity_id
_ihm_struct_assembly.asym_id
_ihm_struct_assembly.seq_id_begin
_ihm_struct_assembly.seq_id_end
1 1 1 foo 1 A 1 3
2 1 1 bar 2 . 1 2
3 2 2 foo 1 A 1 3
#
""")

    def test_model_representation_dumper(self):
        """Test ModelRepresentationDumper"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system, (("foo", "AAAA", 'A'),))
        chain = IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)[0]
        m = state.model
        res1 = IMP.atom.Residue.setup_particle(IMP.Particle(m),
                                               IMP.atom.ALA, 1)
        chain.add_child(res1)
        res2 = IMP.atom.Residue.setup_particle(IMP.Particle(m),
                                               IMP.atom.ALA, 2)
        chain.add_child(res2)
        # First matching object (res1 and res2) will be used
        frag1 = IMP.atom.Fragment.setup_particle(IMP.Particle(m),[1,2])
        chain.add_child(frag1)
        frag2 = IMP.atom.Fragment.setup_particle(IMP.Particle(m),[3,4])
        chain.add_child(frag2)
        state.add_hierarchy(h)
        d = IMP.mmcif.dumper._ModelRepresentationDumper()
        fh = StringIO()
        writer = IMP.mmcif.format._CifWriter(fh)

        d.dump(system, writer)
        out = fh.getvalue()
        self.assertEqual(out, "#
loop_
_ihm_model_representation.ordinal_id
_ihm_model_representation.representation_id
_ihm_model_representation.segment_id
_ihm_model_representation.entity_id
_ihm_model_representation.entity_description
_ihm_model_representation.entity_asym_id
_ihm_model_representation.seq_id_begin
_ihm_model_representation.seq_id_end
_ihm_model_representation.model_object_primitive
_ihm_model_representation.starting_model_id
_ihm_model_representation.model_mode
_ihm_model_representation.model_granularity
_ihm_model_representation.model_object_count
1 1 1 1 foo A 1 2 sphere . flexible by-residue 2
2 1 2 1 foo A 3 4 sphere . flexible by-feature 1
#
")


if __name__ == '__main__':
    IMP.test.main()
