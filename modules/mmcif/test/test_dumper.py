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
    def make_model(self, m, writer):
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        for name, seq, cid in (('foo', 'ACGT', 'A'), ('bar', 'ACGT', 'B'),
                               ('baz', 'ACC', 'C')):
            h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
            mol = IMP.atom.Molecule.setup_particle(h)
            mol.set_name(name)
            top.add_child(mol)

            h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
            chain = IMP.atom.Chain.setup_particle(h, cid)
            chain.set_sequence(seq)
            mol.add_child(chain)
        return top

    def test_entry_dumper(self):
        """Test EntryDumper"""
        m = IMP.Model()
        writer = IMP.mmcif.Writer()
        dumper = IMP.mmcif.dumper._EntryDumper()
        fh = StringIO()
        cifw = IMP.mmcif.format._CifWriter(fh)
        dumper.dump(writer, cifw)

        out = fh.getvalue()
        self.assertEqual(out, "data_imp_model\n_entry.id imp_model\n")

    def test_entity_dumper(self):
        """Test EntityDumper"""
        m = IMP.Model()
        writer = IMP.mmcif.Writer()
        h = self.make_model(m, writer)
        writer.add_hierarchy(h)
        dumper = IMP.mmcif.dumper._EntityDumper()
        fh = StringIO()
        cifw = IMP.mmcif.format._CifWriter(fh)
        dumper.dump(writer, cifw)

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
        m = IMP.Model()
        writer = IMP.mmcif.Writer()
        h = self.make_model(m, writer)
        writer.add_hierarchy(h)
        dumper = IMP.mmcif.dumper._EntityPolyDumper()
        fh = StringIO()
        cifw = IMP.mmcif.format._CifWriter(fh)
        dumper.dump(writer, cifw)

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
        m = IMP.Model()
        writer = IMP.mmcif.Writer()
        h = self.make_model(m, writer)
        writer.add_hierarchy(h)
        dumper = IMP.mmcif.dumper._EntityPolySeqDumper()
        fh = StringIO()
        cifw = IMP.mmcif.format._CifWriter(fh)
        dumper.dump(writer, cifw)

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

if __name__ == '__main__':
    IMP.test.main()
