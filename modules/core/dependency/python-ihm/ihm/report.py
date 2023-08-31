"""Helper classes to provide a summary report of an :class:`ihm.System`"""

from __future__ import print_function
import ihm
import sys
import warnings
import collections


class MissingDataWarning(UserWarning):
    pass


class LocalFilesWarning(UserWarning):
    pass


class MissingFileWarning(UserWarning):
    pass


def _get_name(name):
    if name:
        return repr(name)
    else:
        return "(unnamed)"


class _SectionReporter(object):
    def __init__(self, title, fh):
        self.fh = fh
        print("\n\n# " + title, file=self.fh)

    def report(self, txt):
        print("   " + str(txt), file=self.fh)


class Reporter(object):
    def __init__(self, system, fh=sys.stdout):
        self.system = system
        self.fh = fh

    def report(self):
        print("Title: %s" % self.system.title, file=self.fh)
        self.report_entities()
        self.report_asyms()
        self.report_representations()
        self.report_databases()
        self.report_files()
        self.report_citations()
        self.report_software()
        self.report_protocols()
        self.report_restraints()
        self.report_models()
        self.report_ensembles()

    def _section(self, title):
        return _SectionReporter(title, self.fh)

    def report_entities(self):
        r = self._section("Entities (unique sequences)")
        asyms_for_entity = collections.defaultdict(list)
        for a in self.system.asym_units:
            asyms_for_entity[a.entity].append(a)
        for e in self.system.entities:
            asyms = asyms_for_entity[e]
            r.report("- %s (length %d, %d instances, chain IDs %s)"
                     % (e.description, len(e.sequence), len(asyms),
                        ", ".join(a.id for a in asyms)))
            if len(e.references) == 0:
                warnings.warn(
                    "No reference sequence (e.g. from UniProt) provided "
                    "for %s" % e, MissingDataWarning)
            for ref in e.references:
                r.report("  - from %s" % str(ref))

    def report_asyms(self):
        r = self._section("Asyms/chains")
        for a in self.system.asym_units:
            r.report("- %s (chain ID %s)" % (a.details, a.id))

    def report_citations(self):
        r = self._section("Publications cited")
        for c in self.system._all_citations():
            r.report('- "%s", %s, %s' % (c.title, c.journal, c.year))

    def report_software(self):
        r = self._section("Software used")
        for s in ihm._remove_identical(self.system._all_software()):
            r.report("- %s (version %s)" % (s.name, s.version))
            if not s.citation:
                warnings.warn(
                    "No citation provided for %s" % s, MissingDataWarning)

    def report_databases(self):
        r = self._section("External databases referenced")
        for loc in ihm._remove_identical(self.system._all_locations()):
            if isinstance(loc, ihm.location.DatabaseLocation):
                r.report(" - %s accession %s"
                         % (loc.db_name, loc.access_code))

    def report_files(self):
        r = self._section("Additional files referenced")
        locs_by_repo = collections.defaultdict(list)
        for loc in ihm._remove_identical(self.system._all_locations()):
            if not isinstance(loc, ihm.location.DatabaseLocation):
                locs_by_repo[loc.repo].append(loc)
        for repo, locs in locs_by_repo.items():
            r.report("- %s" % ("DOI: " + repo.doi if repo else "Local files"))
            for loc in locs:
                r.report("  - %r, %s" % (loc.path, loc.details))
        if None in locs_by_repo:
            warnings.warn(
                "The following local files are referenced (they will need to "
                "be deposited in a database or with a DOI): %s"
                % [loc.path for loc in locs_by_repo[None]], LocalFilesWarning)

    def report_representations(self):
        r = self._section("Model representation")
        for rep in self.system._all_representations():
            if hasattr(rep, '_id'):
                r.report("- Representation %s" % rep._id)
            else:
                r.report("- Representation")
            for segment in rep:
                r.report("  - " + segment._get_report())

    def report_protocols(self):
        r = self._section("Modeling protocols")
        for prot in self.system._all_protocols():
            r.report("- " + (prot.name if prot.name else "Unnamed protocol"))
            for step in prot.steps:
                r.report("  - " + step._get_report())
            for analysis in prot.analyses:
                r.report("  - Analysis")
                for step in analysis.steps:
                    r.report("    - " + step._get_report())

    def report_restraints(self):
        r = self._section("Restraints")
        for rsr in ihm._remove_identical(self.system._all_restraints()):
            r.report("- " + rsr._get_report())

    def report_models(self):
        r = self._section("Models")
        for sg in self.system.state_groups:
            r.report("- State group")
            for state in sg:
                r.report("  - State %s" % _get_name(state.name))
                for mg in state:
                    r.report("    - Model group %s containing %d models"
                             % (_get_name(mg.name), len(mg)))

    def report_ensembles(self):
        r = self._section("Ensembles")
        for e in self.system.ensembles:
            r.report("- Ensemble %s containing %d models"
                     % (_get_name(e.name), e.num_models))
            if e.model_group is not None:
                r.report("  - From model group %s"
                         % _get_name(e.model_group.name))
            if e.precision is not None:
                r.report("  - Precision %.1f" % e.precision)
            if e.file:
                r.report("  - In external file %s" % e.file)
            if (e.model_group is not None and not e.file
                    and e.num_models > len(e.model_group)):
                warnings.warn(
                    "%s references more models (%d) than are deposited in "
                    "its model group, but does not reference an external file"
                    % (e, e.num_models), MissingFileWarning)
            for d in e.densities:
                asym = d.asym_unit
                r.report("  - Localization density for %s %d-%d"
                         % (asym.details, asym.seq_id_range[0],
                            asym.seq_id_range[1]))
