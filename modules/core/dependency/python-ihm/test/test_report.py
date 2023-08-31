import unittest
import utils
import os
import sys
if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm
import ihm.report
import ihm.reference
import ihm.location
import ihm.representation
import ihm.protocol
import ihm.analysis
import ihm.restraint
import ihm.geometry
import ihm.model


class Tests(unittest.TestCase):
    def test_report(self):
        """Test System.report()"""
        sio = StringIO()
        s = ihm.System(title='test system')
        s.report(sio)

    def test_entities(self):
        """Test report_entities"""
        sio = StringIO()
        s = ihm.System(title='test system')
        e = ihm.Entity("ACG")
        a = ihm.AsymUnit(e, "my asym")
        a.id = 'A'
        s.asym_units.append(a)
        s.entities.append(e)
        r = ihm.report.Reporter(s, sio)
        # Should warn about missing references
        self.assertWarns(ihm.report.MissingDataWarning, r.report_entities)
        uniprot = ihm.reference.UniProtSequence(
            db_code='testcode', accession='testacc', sequence='CCCG')
        e.references.append(uniprot)
        r.report_entities()

    def test_asyms(self):
        """Test report_asyms"""
        sio = StringIO()
        s = ihm.System(title='test system')
        e = ihm.Entity("ACG")
        s.entities.append(e)
        a = ihm.AsymUnit(e, "my asym")
        s.asym_units.append(a)
        r = ihm.report.Reporter(s, sio)
        r.report_asyms()

    def test_representations(self):
        """Test report_representations"""
        sio = StringIO()
        s = ihm.System(title='test system')
        e = ihm.Entity("ACGT")
        s.entities.append(e)
        a = ihm.AsymUnit(e, "my asym")
        s.asym_units.append(a)
        s1 = ihm.representation.ResidueSegment(
            a(1, 2), starting_model=None,
            rigid=False, primitive='sphere')
        s2 = ihm.representation.FeatureSegment(
            a(3, 4), starting_model=None,
            rigid=True, primitive='other', count=3)
        r1 = ihm.representation.Representation((s1, s2), name='foo')
        s.orphan_representations.append(r1)
        r = ihm.report.Reporter(s, sio)
        # Test report of representation without ID
        r.report_representations()
        # Test report of representation with ID
        r1._id = 42
        r.report_representations()

    def test_citations(self):
        """Test report_citations"""
        sio = StringIO()
        s = ihm.System(title='test system')
        c = ihm.Citation(pmid="foo", title="bar", journal="j", volume=1,
                         page_range=(10, 20), year=2023,
                         authors=["foo", "bar"], doi="test")
        s.citations.append(c)
        r = ihm.report.Reporter(s, sio)
        r.report_citations()

    def test_software(self):
        """Test report_software"""
        sio = StringIO()
        s = ihm.System(title='test system')
        soft = ihm.Software(name='foo', version='1.0',
                            classification='1', description='2', location='3')
        s.software.append(soft)
        r = ihm.report.Reporter(s, sio)
        # Should warn about missing citation
        self.assertWarns(ihm.report.MissingDataWarning, r.report_software)
        c = ihm.Citation(pmid="foo", title="bar", journal="j", volume=1,
                         page_range=(10, 20), year=2023,
                         authors=["foo", "bar"], doi="test")
        soft.citation = c
        r.report_software()

    def test_databases(self):
        """Test report_databases"""
        sio = StringIO()
        s = ihm.System(title='test system')
        s.locations.append(
            ihm.location.BMRBLocation('27600', version='foo', details='bar'))
        s.locations.append(
            ihm.location.FileLocation(repo='mydoi', path='a'))
        r = ihm.report.Reporter(s, sio)
        r.report_databases()

    def test_files(self):
        """Test report_files"""
        sio = StringIO()
        s = ihm.System(title='test system')
        repo = ihm.location.Repository(doi='1.2.3.4')
        s.locations.append(
            ihm.location.BMRBLocation('27600', version='foo', details='bar'))
        s.locations.append(
            ihm.location.FileLocation(repo=repo, path='a'))
        r = ihm.report.Reporter(s, sio)
        r.report_files()

    def test_files_local(self):
        """Test report_files with local files"""
        sio = StringIO()
        s = ihm.System(title='test system')
        s.locations.append(
            ihm.location.BMRBLocation('27600', version='foo', details='bar'))
        s.locations.append(
            ihm.location.FileLocation(repo=None, path='.'))
        r = ihm.report.Reporter(s, sio)
        # Should warn about local files
        self.assertWarns(ihm.report.LocalFilesWarning, r.report_files)

    def test_protocols(self):
        """Test report_protocols"""
        sio = StringIO()
        s = ihm.System(title='test system')
        prot = ihm.protocol.Protocol(name='foo')
        prot.steps.append(ihm.protocol.Step(
            assembly='foo', dataset_group='bar',
            method='baz', num_models_begin=0, num_models_end=42))
        analysis = ihm.analysis.Analysis()
        analysis.steps.append(ihm.analysis.ClusterStep(
            feature='RMSD', num_models_begin=42, num_models_end=5))
        prot.analyses.append(analysis)
        s.orphan_protocols.append(prot)
        r = ihm.report.Reporter(s, sio)
        r.report_protocols()

    def test_restraints(self):
        """Test report_restraints"""
        sio = StringIO()
        s = ihm.System(title='test system')
        dist = ihm.restraint.UpperBoundDistanceRestraint(42.0)
        geom = ihm.geometry.XAxis(name='foo', description='bar')
        rsr = ihm.restraint.GeometricRestraint(
            dataset='foo', geometric_object=geom, feature='feat',
            distance=dist)
        s.restraints.append(rsr)
        r = ihm.report.Reporter(s, sio)
        r.report_restraints()

    def test_models(self):
        """Test report_models"""
        sio = StringIO()
        s = ihm.System(title='test system')
        sg = ihm.model.StateGroup()
        state = ihm.model.State(name='foo')
        mg = ihm.model.ModelGroup(name='bar')
        m = ihm.model.Model(assembly='foo', protocol='bar',
                            representation='baz')
        mg.append(m)
        state.append(mg)
        sg.append(state)
        s.state_groups.append(sg)
        r = ihm.report.Reporter(s, sio)
        r.report_models()

    def test_ensembles(self):
        """Test report_ensembles"""
        sio = StringIO()
        s = ihm.System(title='test system')
        e = ihm.Entity("ACG")
        a = ihm.AsymUnit(e, "my asym")
        a.id = 'A'
        s.asym_units.append(a)
        s.entities.append(e)
        e1 = ihm.model.Ensemble(model_group=None, num_models=10)
        e2 = ihm.model.Ensemble(model_group=None, num_models=5, precision=1.0,
                                name='test ensemble')
        mg = ihm.model.ModelGroup(name='bar')
        e3 = ihm.model.Ensemble(model_group=mg, num_models=1, file='file')
        e3.densities.append(ihm.model.LocalizationDensity(
            file='foo', asym_unit=a(1, 2)))
        s.ensembles.extend((e1, e2, e3))
        r = ihm.report.Reporter(s, sio)
        r.report_ensembles()
        # Should warn about extra models but no external file
        e4 = ihm.model.Ensemble(model_group=mg, num_models=1)
        s.ensembles.append(e4)
        self.assertWarns(ihm.report.MissingFileWarning, r.report_ensembles)


if __name__ == '__main__':
    unittest.main()
