"""Representation of an IHM mmCIF file as a set of Python classes.

   Generally class names correspond to mmCIF table names and class
   attributes to mmCIF attributes (with prefixes like `pdbx_` stripped).
   For example, the data item _entity.details is found in the
   :class:`Entity` class, as the `details` member.

   Ordinals and IDs are generally not used in this representation (instead,
   pointers to objects are used).
"""

import itertools
import re
import sys
# Handle different naming of urllib in Python 2/3
try:
    import urllib.request as urllib2
except ImportError:
    import urllib2
import json
from . import util

__version__ = '0.38'


class __UnknownValue(object):
    # Represent the mmCIF 'unknown' special value

    def __str__(self):
        return '?'
    __repr__ = __str__

    def __bool__(self):
        return False
    # Python2 compatibility
    __nonzero__ = __bool__

    # Needs to be hashable so that classes like Software (that might
    # use unknown values as attributes) are hashable
    def __hash__(self):
        return 0

    # Unknown value is a singleton and should only compare equal to itself
    def __eq__(self, other):
        return self is other

    def __lt__(self, other):
        return False
    __gt__ = __lt__
    __le__ = __ge__ = __eq__


#: A value that isn't known. Note that this is distinct from a value that
#: is deliberately omitted, which is represented by Python None.
unknown = __UnknownValue()


def _remove_identical(gen):
    """Return only unique objects from `gen`.
       Objects that are identical are only returned once, although multiple
       non-identical objects that compare equal may be returned."""
    seen_objs = {}
    for obj in gen:
        if id(obj) in seen_objs:
            continue
        seen_objs[id(obj)] = None
        yield obj


class System(object):
    """Top-level class representing a complete modeled system.

       :param str title: Title (longer text description) of the system.
       :param str id: Unique identifier for this system in the mmCIF file.
       :param str model_details: Detailed description of the system, like an
                                 abstract.
    """

    structure_determination_methodology = "integrative"

    def __init__(self, title=None, id='model', model_details=None):
        self.id = id
        self.title = title
        self.model_details = model_details

        #: List of plain text comments. These will be added to the top of
        #: the mmCIF file.
        self.comments = []

        #: List of all software used in the modeling. See :class:`Software`.
        self.software = []

        #: List of all authors of this system, as a list of strings (last name
        #: followed by initials, e.g. "Smith, A.J."). When writing out a file,
        #: if this list is empty, the set of all citation authors (see
        #: :attr:`Citation.authors`) is used instead.
        self.authors = []

        #: List of all grants that supported this work. See :class:`Grant`.
        self.grants = []

        #: List of all citations. See :class:`Citation`.
        self.citations = []

        #: All entities used in the system. See :class:`Entity`.
        self.entities = []

        #: All asymmetric units used in the system. See :class:`AsymUnit`.
        self.asym_units = []

        #: Collections (if any) to which this entry belongs.
        #: These are used to group depositions of related entries.
        #: See :class:`Collection`.
        self.collections = []

        #: All orphaned chemical descriptors in the system.
        #: See :class:`ChemDescriptor`. This can be used to track descriptors
        #: that are not otherwise used - normally one is assigned to a
        #: :class:`ihm.restraint.CrossLinkRestraint`.
        self.orphan_chem_descriptors = []

        #: All orphaned assemblies in the system. See :class:`Assembly`.
        #: This can be used to keep track of all assemblies that are not
        #: otherwise used - normally one is assigned to a
        #: :class:`~ihm.model.Model`,
        #: :class:`ihm.protocol.Step`, or
        #: :class:`~ihm.restraint.Restraint`.
        self.orphan_assemblies = []

        #: The assembly of the entire system. By convention this is always
        #: the first assembly in the mmCIF file (assembly_id=1). Note that
        #: currently this isn't filled in on output until dumper.write()
        #: is called. See :class:`Assembly`.
        self.complete_assembly = Assembly((), name='Complete assembly',
                                          description='All known components')

        #: Locations of all extra resources.
        #: See :class:`~ihm.location.Location`.
        self.locations = []

        #: All orphaned datasets.
        #: This can be used to keep track of all datasets that are not
        #: otherwise used - normally a dataset is assigned to a
        #: :class:`~ihm.dataset.DatasetGroup`,
        #: :class:`~ihm.startmodel.StartingModel`,
        #: :class:`~ihm.restraint.Restraint`,
        #: :class:`~ihm.startmodel.Template`,
        #: or as the parent of another :class:`~ihm.dataset.Dataset`.
        #: See :class:`~ihm.dataset.Dataset`.
        self.orphan_datasets = []

        #: All orphaned groups of datasets.
        #: This can be used to keep track of all dataset groups that are not
        #: otherwise used - normally a group is assigned to a
        #: :class:`~ihm.protocol.Protocol`.
        #: See :class:`~ihm.dataset.DatasetGroup`.
        self.orphan_dataset_groups = []

        #: All orphaned representations of the system.
        #: This can be used to keep track of all representations that are not
        #: otherwise used - normally one is assigned to a
        #: :class:`~ihm.model.Model`.
        #: See :class:`~ihm.representation.Representation`.
        self.orphan_representations = []

        #: All orphaned starting models for the system.
        #: This can be used to keep track of all starting models that are not
        #: otherwise used - normally one is assigned to an
        #: :class:`ihm.representation.Segment`.
        #: See :class:`~ihm.startmodel.StartingModel`.
        self.orphan_starting_models = []

        #: All restraints on the system.
        #: See :class:`~ihm.restraint.Restraint`.
        self.restraints = []

        #: All restraint groups.
        #: See :class:`~ihm.restraint.RestraintGroup`.
        self.restraint_groups = []

        #: All orphaned modeling protocols.
        #: This can be used to keep track of all protocols that are not
        #: otherwise used - normally a protocol is assigned to a
        #: :class:`~ihm.model.Model`.
        #: See :class:`~ihm.protocol.Protocol`.
        self.orphan_protocols = []

        #: All ensembles.
        #: See :class:`~ihm.model.Ensemble`.
        self.ensembles = []

        #: All ordered processes.
        #: See :class:`~ihm.model.OrderedProcess`.
        self.ordered_processes = []

        #: All state groups (collections of models).
        #: See :class:`~ihm.model.StateGroup`.
        self.state_groups = []

        #: All orphaned geometric objects.
        #: This can be used to keep track of all objects that are not
        #: otherwise used - normally an object is assigned to a
        #: :class:`~ihm.restraint.GeometricRestraint`.
        #: See :class:`~ihm.geometry.GeometricObject`.
        self.orphan_geometric_objects = []

        #: All orphaned features.
        #: This can be used to keep track of all features that are not
        #: otherwise used - normally a feature is assigned to a
        #: :class:`~ihm.restraint.GeometricRestraint`.
        #: See :class:`~ihm.restraint.Feature`.
        self.orphan_features = []

        #: All orphaned pseudo sites.
        #: This can be used to keep track of all pseudo sites that are not
        #: otherwise used - normally a site is used in a
        #: :class:`~ihm.restraint.PseudoSiteFeature` or a
        #: :class:`~ihm.restraint.CrossLinkPseudoSite`.
        self.orphan_pseudo_sites = []

        #: Contains the fluorescence (FLR) part.
        #: See :class:`~ihm.flr.FLRData`.
        self.flr_data = []

    def _make_complete_assembly(self):
        """Fill in the complete assembly with all asym units"""
        # Clear out any existing components
        self.complete_assembly[:] = []

        # Include all asym units
        for asym in self.asym_units:
            self.complete_assembly.append(asym)

    def _all_models(self):
        """Iterate over all Models in the system"""
        # todo: raise an error if a model is present in multiple groups
        for group in self._all_model_groups():
            seen_models = {}
            for model in group:
                if model in seen_models:
                    continue
                seen_models[model] = None
                yield group, model

    def update_locations_in_repositories(self, repos):
        """Update all :class:`Location` objects in the system that lie within
           a checked-out :class:`Repository` to point to that repository.

           This is intended for the use case where the current working
           directory is a checkout of a repository which is archived somewhere
           with a DOI. Locations can then be simply constructed pointing to
           local files, and retroactively updated with this method to point
           to the DOI if appropriate.

           For each Location, if it points to a local file that is below the
           `root` of one of the `repos`, update it to point to that repository.
           If is under multiple roots, pick the one that gives the shortest
           path. For example, if run in a subdirectory `foo` of a repository
           archived as `repo.zip`, the local path `simple.pdb` will
           be updated to be `repo-top/foo/simple.pdb` in `repo.zip`::

               l = ihm.location.InputFileLocation("simple.pdb")
               system.locations.append(l)

               r = ihm.location.Repository(doi='1.2.3.4',
                         url='https://example.com/repo.zip',)
                         top_directory="repo-top", root="..")
               system.update_locations_in_repositories([r])
        """
        import ihm.location
        for loc in self._all_locations():
            if isinstance(loc, ihm.location.FileLocation):
                ihm.location.Repository._update_in_repos(loc, repos)

    def _all_restraints(self):
        """Iterate over all Restraints in the system.
           Duplicates may be present."""
        def _all_restraints_in_groups():
            for rg in self.restraint_groups:
                for r in rg:
                    yield r
        return itertools.chain(self.restraints, _all_restraints_in_groups())

    def _all_chem_descriptors(self):
        """Iterate over all ChemDescriptors in the system.
           Duplicates may be present."""
        return itertools.chain(
            self.orphan_chem_descriptors,
            (restraint.linker for restraint in self._all_restraints()
                if hasattr(restraint, 'linker') and restraint.linker),
            (itertools.chain.from_iterable(
                f._all_flr_chemical_descriptors() for f in self.flr_data)))

    def _all_model_groups(self, only_in_states=True):
        """Iterate over all ModelGroups in the system.
           If only_in_states is True, only return ModelGroups referenced
           by a State object; otherwise, also include ModelGroups referenced
           by an OrderedProcess or Ensemble."""
        # todo: raise an error if a modelgroup is present in multiple states
        for state_group in self.state_groups:
            for state in state_group:
                for model_group in state:
                    yield model_group
        if not only_in_states:
            for ensemble in self.ensembles:
                if ensemble.model_group:
                    yield ensemble.model_group
                for ss in ensemble.subsamples:
                    if ss.model_group:
                        yield ss.model_group
            for proc in self.ordered_processes:
                for step in proc.steps:
                    for edge in step:
                        yield edge.group_begin
                        yield edge.group_end

    def _all_representations(self):
        """Iterate over all Representations in the system.
           This includes all Representations referenced from other objects,
           plus any orphaned Representations. Duplicates are filtered out."""
        return _remove_identical(itertools.chain(
            self.orphan_representations,
            (model.representation for group, model in self._all_models()
                if model.representation)))

    def _all_segments(self):
        for representation in self._all_representations():
            for segment in representation:
                yield segment

    def _all_starting_models(self):
        """Iterate over all StartingModels in the system.
           This includes all StartingModels referenced from other objects, plus
           any orphaned StartingModels. Duplicates are filtered out."""
        return _remove_identical(itertools.chain(
            self.orphan_starting_models,
            (segment.starting_model for segment in self._all_segments()
                if segment.starting_model)))

    def _all_protocols(self):
        """Iterate over all Protocols in the system.
           This includes all Protocols referenced from other objects, plus
           any orphaned Protocols. Duplicates are filtered out."""
        return _remove_identical(itertools.chain(
            self.orphan_protocols,
            (model.protocol for group, model in self._all_models()
                if model.protocol)))

    def _all_protocol_steps(self):
        for protocol in self._all_protocols():
            for step in protocol.steps:
                yield step

    def _all_analysis_steps(self):
        for protocol in self._all_protocols():
            for analysis in protocol.analyses:
                for step in analysis.steps:
                    yield step

    def _all_assemblies(self):
        """Iterate over all Assemblies in the system.
           This includes all Assemblies referenced from other objects, plus
           any orphaned Assemblies. Duplicates may be present."""
        return itertools.chain(
            # Complete assembly is always first
            (self.complete_assembly,),
            self.orphan_assemblies,
            (model.assembly for group, model in self._all_models()
             if model.assembly),
            (step.assembly for step in self._all_protocol_steps()
             if step.assembly),
            (step.assembly for step in self._all_analysis_steps()
             if step.assembly),
            (restraint.assembly
             for restraint in self._all_restraints() if restraint.assembly))

    def _all_dataset_groups(self):
        """Iterate over all DatasetGroups in the system.
           This includes all DatasetGroups referenced from other objects, plus
           any orphaned groups. Duplicates may be present."""
        return itertools.chain(
            self.orphan_dataset_groups,
            (step.dataset_group for step in self._all_protocol_steps()
             if step.dataset_group),
            (step.dataset_group for step in self._all_analysis_steps()
             if step.dataset_group))

    def _all_templates(self):
        """Iterate over all Templates in the system."""
        for startmodel in self._all_starting_models():
            for template in startmodel.templates:
                yield template

    def _all_datasets_except_parents(self):
        """Iterate over all Datasets except those referenced only
           as the parent of another Dataset. Duplicates may be present."""
        def _all_datasets_in_groups():
            for dg in self._all_dataset_groups():
                for d in dg:
                    yield d
        return itertools.chain(
            self.orphan_datasets,
            _all_datasets_in_groups(),
            (sm.dataset for sm in self._all_starting_models()
             if sm.dataset),
            (restraint.dataset for restraint in self._all_restraints()
             if restraint.dataset),
            (template.dataset for template in self._all_templates()
             if template.dataset))

    def _all_datasets(self):
        """Iterate over all Datasets in the system.
           This includes all Datasets referenced from other objects, plus
           any orphaned datasets. Duplicates may be present."""
        def _all_datasets_and_parents(d):
            for p in d.parents:
                # Handle transformed datasets
                if hasattr(p, 'dataset'):
                    pd = p.dataset
                else:
                    pd = p
                for alld in _all_datasets_and_parents(pd):
                    yield alld
            yield d
        for d in self._all_datasets_except_parents():
            for alld in _all_datasets_and_parents(d):
                yield alld

    def _all_densities(self):
        for ensemble in self.ensembles:
            for density in ensemble.densities:
                yield density

    def _all_locations(self):
        """Iterate over all Locations in the system.
           This includes all Locations referenced from other objects, plus
           any referenced from the top-level system.
           Duplicates may be present."""
        def _all_ensemble_locations():
            for ensemble in self.ensembles:
                if ensemble.file:
                    yield ensemble.file
                for ss in ensemble.subsamples:
                    if ss.file:
                        yield ss.file
        return itertools.chain(
            self.locations,
            (dataset.location for dataset in self._all_datasets()
                if hasattr(dataset, 'location') and dataset.location),
            _all_ensemble_locations(),
            (density.file for density in self._all_densities()
                if density.file),
            (sm.script_file for sm in self._all_starting_models()
                if sm.script_file),
            (template.alignment_file for template in self._all_templates()
                if template.alignment_file),
            (step.script_file for step in self._all_protocol_steps()
                if step.script_file),
            (step.script_file for step in self._all_analysis_steps()
                if step.script_file))

    def _all_geometric_objects(self):
        """Iterate over all GeometricObjects in the system.
           This includes all GeometricObjects referenced from other objects,
           plus any referenced from the top-level system.
           Duplicates may be present."""
        return itertools.chain(
            self.orphan_geometric_objects,
            (restraint.geometric_object
             for restraint in self._all_restraints()
             if hasattr(restraint, 'geometric_object')
             and restraint.geometric_object))

    def _all_features(self):
        """Iterate over all Features in the system.
           This includes all Features referenced from other objects,
           plus any referenced from the top-level system.
           Duplicates may be present."""
        def _all_restraint_features():
            for r in self._all_restraints():
                if hasattr(r, '_all_features'):
                    for feature in r._all_features:
                        if feature:
                            yield feature
        return itertools.chain(self.orphan_features, _all_restraint_features())

    def _all_pseudo_sites(self):
        """Iterate over all PseudoSites in the system.
           This includes all PseudoSites referenced from other objects,
           plus any referenced from the top-level system.
           Duplicates may be present."""
        def _all_restraint_sites():
            for r in self._all_restraints():
                if hasattr(r, 'cross_links'):
                    for xl in r.cross_links:
                        if xl.pseudo1:
                            for x in xl.pseudo1:
                                yield x.site
                        if xl.pseudo2:
                            for x in xl.pseudo2:
                                yield x.site
        return itertools.chain(self.orphan_pseudo_sites,
                               _all_restraint_sites(),
                               (f.site for f in self._all_features()
                                if hasattr(f, 'site') and f.site))

    def _all_software(self):
        """Iterate over all Software in the system.
           This includes all Software referenced from other objects, plus
           any referenced from the top-level system.
           Duplicates may be present."""
        return (itertools.chain(
            self.software,
            (sm.software for sm in self._all_starting_models()
             if sm.software),
            (step.software for step in self._all_protocol_steps()
             if step.software),
            (step.software for step in self._all_analysis_steps()
             if step.software),
            (r.software for r in self._all_restraints()
             if hasattr(r, 'software') and r.software)))

    def _all_citations(self):
        """Iterate over all Citations in the system.
           This includes all Citations referenced from other objects, plus
           any referenced from the top-level system.
           Duplicates are filtered out."""
        return _remove_identical(itertools.chain(
            self.citations,
            (software.citation for software in self._all_software()
             if software.citation),
            (restraint.fitting_method_citation_id
             for restraint in self._all_restraints()
             if hasattr(restraint, 'fitting_method_citation_id')
             and restraint.fitting_method_citation_id)))

    def _all_entity_ranges(self):
        """Iterate over all Entity ranges in the system (these may be
           :class:`Entity`, :class:`AsymUnit`, :class:`EntityRange` or
           :class:`AsymUnitRange` objects).
           Note that we don't include self.entities or self.asym_units here,
           as we only want ranges that were actually used.
           Duplicates may be present."""
        return (itertools.chain(
            (sm.asym_unit for sm in self._all_starting_models()),
            (seg.asym_unit for seg in self._all_segments()),
            (comp for a in self._all_assemblies() for comp in a),
            (comp for f in self._all_features()
                for comp in f._all_entities_or_asyms()),
            (d.asym_unit for d in self._all_densities())))

    def _before_write(self):
        """Do any setup necessary before writing out to a file"""
        # Here, we initialize all RestraintGroups by removing any assigned ID
        for g in self.restraint_groups:
            util._remove_id(g)
        # Fill in complete assembly
        self._make_complete_assembly()

    def _check_after_write(self):
        """Make sure everything was successfully written"""
        # Here, we check that all RestraintGroups were successfully dumped"""
        for g in self.restraint_groups:
            if len(g) > 0 and not hasattr(g, '_id'):
                raise TypeError(
                    "RestraintGroup(%s) contains an unsupported combination "
                    "of Restraints. Due to limitations of the underlying "
                    "dictionary, all objects in a RestraintGroup must be of "
                    "the same type, and only certain types (currently only "
                    "DerivedDistanceRestraint or PredictedContactRestraint) "
                    "can be grouped." % g)


class Software(object):
    """Software used as part of the modeling protocol.

       :param str name: The name of the software.
       :param str classification: The major function of the sofware, for
              example 'model building', 'sample preparation',
              'data collection'.
       :param str description: A longer text description of the software.
       :param str location: Place where the software can be found (e.g. URL).
       :param str type: Type of software (program/package/library/other).
       :param str version: The version used.
       :param citation: Publication describing the software.
       :type citation: :class:`Citation`

       Generally these objects are added to :attr:`System.software` or
       passed to :class:`ihm.startmodel.StartingModel`,
       :class:`ihm.protocol.Step`,
       :class:`ihm.analysis.Step`, or
       :class:`ihm.restraint.PredictedContactResstraint` objects.
    """
    def __init__(self, name, classification, description, location,
                 type='program', version=None, citation=None):
        self.name = name
        self.classification = classification
        self.description = description
        self.location = location
        self.type = type
        self.version = version
        self.citation = citation

    # Software compares equal if the names and versions are the same
    def _eq_vals(self):
        return (self.name, self.version)

    def __eq__(self, other):
        return self._eq_vals() == other._eq_vals()

    def __hash__(self):
        return hash(self._eq_vals())


class Grant(object):
    """Information on funding support for the modeling.
       See :attr:`System.grants`.

       :param str funding_organization: The name of the organization providing
              the funding, e.g. "National Institutes of Health".
       :param str country: The country that hosts the funding organization,
              e.g. "United States".
       :param str grant_number: Identifying information for the grant, e.g.
              "1R01GM072999-01".
    """
    def __init__(self, funding_organization, country, grant_number):
        self.funding_organization = funding_organization
        self.country = country
        self.grant_number = grant_number


class Citation(object):
    """A publication that describes the modeling.

       Generally citations are added to :attr:`System.citations` or
       passed to :class:`ihm.Software` or
       :class:`ihm.restraint.EM3DRestraint` objects.

       :param str pmid: The PubMed ID.
       :param str title: Full title of the publication.
       :param str journal: Abbreviated journal name.
       :param volume: Journal volume as int for a plain number or str for
                      journals adding a label to the number (e.g. "46(W1)" for
                      a web server issue).
       :param page_range: The page (int) or page range (as a 2-element
              int tuple). Using str also works for labelled page numbers.
       :param int year: Year of publication.
       :param authors: All authors in order, as a list of strings (last name
              followed by initials, e.g. "Smith, A.J.").
       :param str doi: Digital Object Identifier of the publication.
       :param bool is_primary: Denotes the most pertinent publication for the
              modeling itself (as opposed to a method or piece of software used
              in the protocol). Only one such publication is allowed, and it
              is assigned the ID "primary" in the mmCIF file.
    """
    def __init__(self, pmid, title, journal, volume, page_range, year, authors,
                 doi, is_primary=False):
        self.title, self.journal, self.volume = title, journal, volume
        self.page_range, self.year = page_range, year
        self.pmid, self.doi = pmid, doi
        self.authors = authors if authors is not None else []
        self.is_primary = is_primary

    @classmethod
    def from_pubmed_id(cls, pubmed_id):
        """Create a Citation from just a PubMed ID.
           This is done by querying NCBI's web API, so requires network access.

           :param int pubmed_id: The PubMed identifier.
           :return: A new Citation for the given identifier.
           :rtype: :class:`Citation`
        """
        def get_doi(ref):
            for art_id in ref['articleids']:
                if art_id['idtype'] == 'doi':
                    return enc(art_id['value'])

        def get_page_range(ref):
            rng = enc(ref['pages']).split('-')
            if len(rng) == 2 and len(rng[1]) < len(rng[0]):
                # map ranges like "2730-43" to 2730,2743 not 2730, 43
                rng[1] = rng[0][:len(rng[0]) - len(rng[1])] + rng[1]
            # Handle one page or empty page range
            if len(rng) == 1:
                rng = rng[0]
            if rng == '':
                rng = None
            return rng
        # JSON values are always Unicode, but on Python 2 we want non-Unicode
        # strings, so convert to ASCII
        if sys.version_info[0] < 3:
            def enc(s):
                return s.encode('ascii')
        else:
            def enc(s):
                return s

        url = ('https://eutils.ncbi.nlm.nih.gov/entrez/eutils/esummary.fcgi'
               '?db=pubmed&retmode=json&rettype=abstract&id=%s' % pubmed_id)
        fh = urllib2.urlopen(url)
        j = json.load(fh)
        fh.close()
        ref = j['result'][str(pubmed_id)]
        authors = [enc(x['name']) for x in ref['authors']
                   if x['authtype'] == 'Author']

        # PubMed authors are usually of the form "Lastname AB" but PDB uses
        # "Lastname, A.B." so map one to the other if possible
        r = re.compile(r'(^\w+.*?)\s+(\w+)$')

        def auth_sub(m):
            return m.group(1) + ", " + "".join(initial + "."
                                               for initial in m.group(2))
        authors = [r.sub(auth_sub, auth) for auth in authors]

        return cls(pmid=pubmed_id, title=enc(ref['title']),
                   journal=enc(ref['source']),
                   volume=enc(ref['volume']) or None,
                   page_range=get_page_range(ref),
                   year=enc(ref['pubdate']).split()[0],
                   authors=authors, doi=get_doi(ref))


class ChemComp(object):
    """A chemical component from which :class:`Entity` objects are constructed.
       Usually these are amino acids (see :class:`LPeptideChemComp`) or
       nucleic acids (see :class:`DNAChemComp` and :class:`RNAChemComp`).

       For standard amino and nucleic acids, it is generally easier to use
       a :class:`Alphabet` and refer to the components with their one-letter
       (amino acids, RNA) or two-letter (DNA) codes.

       :param str id: A globally unique identifier for this component (usually
              three letters).
       :param str code: A shorter identifier (usually one letter) that only
              needs to be unique in the entity.
       :param str code_canonical: Canonical version of `code` (which need not
              be unique).
       :param str name: A longer human-readable name for the component.
       :param str formula: The chemical formula. This is a space-separated
              list of the element symbols in the component, each followed
              by an optional count (if omitted, 1 is assumed). The formula
              is terminated with the formal charge (if not zero). The element
              list should be sorted alphabetically, unless carbon is present,
              in which case C and H precede the rest of the elements. For
              example, water would be "H2 O" and arginine (with +1 formal
              charge) "C6 H15 N4 O2 1".
       :param str ccd: The chemical component dictionary (CCD) where
              this component is defined. Can be "core" for the wwPDB CCD
              (https://www.wwpdb.org/data/ccd), "ma" for the ModelArchive CCD,
              or "local" for a novel component that is defined in the mmCIF
              file itself. If unspecified, defaults to "core" unless
              ``descriptors`` is given in which case it defaults to "local".
              This information is essentially ignored by python-ihm (since
              the IHM dictionary has no support for custom CCDs) but is used
              by python-modelcif.
       :param list descriptors: When ``ccd`` is "local", this can be one or
              more descriptor objects that describe the chemistry. python-ihm
              does not define any, but python-modelcif does.

       For example, glycine would have
       ``id='GLY', code='G', code_canonical='G'`` while selenomethionine would
       use ``id='MSE', code='MSE', code_canonical='M'``, guanosine (RNA)
       ``id='G', code='G', code_canonical='G'``, and deoxyguanosine (DNA)
       ``id='DG', code='DG', code_canonical='G'``.
    """

    type = 'other'

    _element_mass = {'H': 1.008, 'C': 12.011, 'N': 14.007, 'O': 15.999,
                     'P': 30.974, 'S': 32.060, 'Se': 78.971, 'Fe': 55.845,
                     'Ac': 227.028, 'Ag': 107.868, 'Al': 26.982, 'Ar': 39.948,
                     'As': 74.922, 'Au': 196.966, 'B': 10.81, 'Ba': 137.327,
                     'Be': 9.012, 'Bi': 208.98, 'Br': 79.904, 'Ca': 40.078,
                     'Cd': 112.414, 'Ce': 140.116, 'Cl': 35.453, 'Co': 58.933,
                     'Cr': 51.996, 'Cs': 132.905, 'Cu': 63.546, 'Dy': 162.5,
                     'Er': 167.259, 'Eu': 151.964, 'F': 18.998, 'Ga': 69.723,
                     'Gd': 157.25, 'Ge': 72.53, 'He': 4.003, 'Hf': 178.486,
                     'Hg': 200.592, 'Ho': 164.93, 'I': 126.904, 'In': 114.818,
                     'Ir': 192.217, 'K': 39.098, 'Kr': 83.798, 'La': 138.905,
                     'Li': 6.938, 'Lu': 174.967, 'Mg': 24.305, 'Mn': 54.938,
                     'Mo': 95.95, 'Na': 22.99, 'Nb': 92.906, 'Nd': 144.242,
                     'Ne': 20.180, 'Ni': 58.693, 'Np': 237.0, 'Os': 190.23,
                     'Pa': 231.036, 'Pb': 207.2, 'Pd': 106.42, 'Pr': 140.908,
                     'Pt': 195.084, 'Ra': 226.025, 'Rb': 85.468, 'Re': 186.207,
                     'Rh': 102.906, 'Ru': 101.07, 'Sb': 121.760, 'Sc': 44.956,
                     'Si': 28.086, 'Sm': 150.36, 'Sn': 118.710, 'Sr': 87.62,
                     'Ta': 180.948, 'Tb': 158.925, 'Te': 127.6, 'Th': 232.038,
                     'Ti': 47.867, 'Tl': 204.383, 'Tm': 168.934, 'U': 238.029,
                     'V': 50.942, 'W': 183.84, 'Xe': 131.293, 'Y': 88.906,
                     'Yb': 173.045, 'Zn': 65.38, 'Zr': 91.224}

    def __init__(self, id, code, code_canonical, name=None, formula=None,
                 ccd=None, descriptors=None):
        self.id = id
        self.code, self.code_canonical, self.name = code, code_canonical, name
        self.formula = formula
        self.ccd, self.descriptors = ccd, descriptors

    def __str__(self):
        return ('<%s.%s(%s)>'
                % (self.__class__.__module__, self.__class__.__name__,
                   self.id))

    def __get_weight(self):
        # Calculate weight from formula
        if self.formula is None:
            return
        spl = self.formula.split()
        # Remove formal charge if present
        if len(spl) > 0 and spl[-1].isdigit():
            del spl[-1]
        r = re.compile(r'(\D+)(\d*)$')
        weight = 0.
        for s in spl:
            m = r.match(s)
            if m is None:
                raise ValueError("Bad formula fragment: %s" % s)
            emass = self._element_mass.get(m.group(1), None)
            if emass:
                weight += emass * (int(m.group(2)) if m.group(2) else 1)
            elif m.group(1) != 'X':
                # If element is unknown, weight is unknown too
                # Element 'X' is used for GLX/ASX and has zero weight
                return None
        return weight

    formula_weight = property(
        __get_weight,
        doc="Formula weight (dalton). This is calculated automatically from "
            "the chemical formula and known atomic masses.")

    # Equal if all identifiers are the same
    def __eq__(self, other):
        return ((self.code, self.code_canonical, self.id, self.type) ==
                (other.code, other.code_canonical, other.id, other.type))

    def __hash__(self):
        return hash((self.code, self.code_canonical, self.id, self.type))


class PeptideChemComp(ChemComp):
    """A single peptide component. Usually :class:`LPeptideChemComp` is used
       instead (except for glycine) to specify chirality.
       See :class:`ChemComp` for a description of the parameters."""
    type = 'peptide linking'


class LPeptideChemComp(PeptideChemComp):
    """A single peptide component with (normal) L- chirality.
       See :class:`ChemComp` for a description of the parameters."""
    type = 'L-peptide linking'


class DPeptideChemComp(PeptideChemComp):
    """A single peptide component with (unusual) D- chirality.
       See :class:`ChemComp` for a description of the parameters."""
    type = 'D-peptide linking'


class DNAChemComp(ChemComp):
    """A single DNA component.
       See :class:`ChemComp` for a description of the parameters."""
    type = 'DNA linking'


class RNAChemComp(ChemComp):
    """A single RNA component.
       See :class:`ChemComp` for a description of the parameters."""
    type = 'RNA linking'


class NonPolymerChemComp(ChemComp):
    """A non-polymer chemical component, such as a ligand or a non-standard
       residue (for crystal waters, use :class:`WaterChemComp`).

       :param str id: A globally unique identifier for this component.
       :param str code_canonical: Canonical one-letter identifier. This is
              used for non-standard residues and should be the one-letter code
              of the closest standard residue (or by default, 'X').
       :param str name: A longer human-readable name for the component.
       :param str formula: The chemical formula. See :class:`ChemComp` for
              more details.
       :param str ccd: The chemical component dictionary (CCD) where
              this component is defined. See :class:`ChemComp` for
              more details.
       :param list descriptors: Information on the component's chemistry.
              See :class:`ChemComp` for more details.
    """
    type = "non-polymer"

    def __init__(self, id, code_canonical='X', name=None, formula=None,
                 ccd=None, descriptors=None):
        super(NonPolymerChemComp, self).__init__(
            id, id, code_canonical, name=name, formula=formula,
            ccd=ccd, descriptors=descriptors)


class WaterChemComp(NonPolymerChemComp):
    """The chemical component for crystal water.
    """
    def __init__(self):
        super(WaterChemComp, self).__init__('HOH', name='WATER',
                                            formula="H2 O")


class Alphabet(object):
    """A mapping from codes (usually one-letter, or two-letter for DNA) to
       chemical components.
       These classes can be used to construct sequences of components
       when creating an :class:`Entity`. They can also be used like a Python
       dict to get standard components, e.g.::

           a = ihm.LPeptideAlphabet()
           met = a['M']
           gly = a['G']

       See :class:`LPeptideAlphabet`, :class:`RNAAlphabet`,
       :class:`DNAAlphabet`.
    """
    def __getitem__(self, key):
        return self._comps[key]

    def __contains__(self, key):
        return key in self._comps

    keys = property(lambda self: self._comps.keys())
    values = property(lambda self: self._comps.values())
    items = property(lambda self: self._comps.items())


class LPeptideAlphabet(Alphabet):
    """A mapping from one-letter amino acid codes (e.g. H, M) to
       L-amino acids (as :class:`LPeptideChemComp` objects, except for achiral
       glycine which maps to :class:`PeptideChemComp`). Some other common
       modified residues are also included (e.g. MSE). For these their full
       name rather than a one-letter code is used.
    """
    _comps = dict([code, LPeptideChemComp(id, code, code, name,
                                          formula)]
                  for code, id, name, formula in [
                  ('A', 'ALA', 'ALANINE', 'C3 H7 N O2'),
                  ('C', 'CYS', 'CYSTEINE', 'C3 H7 N O2 S'),
                  ('D', 'ASP', 'ASPARTIC ACID', 'C4 H7 N O4'),
                  ('E', 'GLU', 'GLUTAMIC ACID', 'C5 H9 N O4'),
                  ('F', 'PHE', 'PHENYLALANINE', 'C9 H11 N O2'),
                  ('H', 'HIS', 'HISTIDINE', 'C6 H10 N3 O2 1'),
                  ('I', 'ILE', 'ISOLEUCINE', 'C6 H13 N O2'),
                  ('K', 'LYS', 'LYSINE', 'C6 H15 N2 O2 1'),
                  ('L', 'LEU', 'LEUCINE', 'C6 H13 N O2'),
                  ('M', 'MET', 'METHIONINE', 'C5 H11 N O2 S'),
                  ('N', 'ASN', 'ASPARAGINE', 'C4 H8 N2 O3'),
                  ('P', 'PRO', 'PROLINE', 'C5 H9 N O2'),
                  ('Q', 'GLN', 'GLUTAMINE', 'C5 H10 N2 O3'),
                  ('R', 'ARG', 'ARGININE', 'C6 H15 N4 O2 1'),
                  ('S', 'SER', 'SERINE', 'C3 H7 N O3'),
                  ('T', 'THR', 'THREONINE', 'C4 H9 N O3'),
                  ('V', 'VAL', 'VALINE', 'C5 H11 N O2'),
                  ('W', 'TRP', 'TRYPTOPHAN', 'C11 H12 N2 O2'),
                  ('Y', 'TYR', 'TYROSINE', 'C9 H11 N O3'),
                  ('B', 'ASX', 'ASP/ASN AMBIGUOUS', 'C4 H6 N O2 X2'),
                  ('Z', 'GLX', 'GLU/GLN AMBIGUOUS', 'C5 H8 N O2 X2'),
                  ('U', 'SEC', 'SELENOCYSTEINE', 'C3 H7 N O2 Se')])
    _comps['G'] = PeptideChemComp('GLY', 'G', 'G', name='GLYCINE',
                                  formula="C2 H5 N O2")

    # common non-standard L-amino acids
    _comps.update([id, LPeptideChemComp(id, id, canon, name, formula)]
                  for id, canon, name, formula in [
                  ('MSE', 'M', 'SELENOMETHIONINE', 'C5 H11 N O2 Se'),
                  ('UNK', 'X', 'UNKNOWN', 'C4 H9 N O2')])


class DPeptideAlphabet(Alphabet):
    """A mapping from D-amino acid codes (e.g. DHI, MED) to
       D-amino acids (as :class:`DPeptideChemComp` objects, except for achiral
       glycine which maps to :class:`PeptideChemComp`). See
       :class:`LPeptideAlphabet` for more details.
    """
    _comps = dict([code, DPeptideChemComp(code, code, canon, name, formula)]
                  for canon, code, name, formula in [
                  ('A', 'DAL', 'D-ALANINE', 'C3 H7 N O2'),
                  ('C', 'DCY', 'D-CYSTEINE', 'C3 H7 N O2 S'),
                  ('D', 'DAS', 'D-ASPARTIC ACID', 'C4 H7 N O4'),
                  ('E', 'DGL', 'D-GLUTAMIC ACID', 'C5 H9 N O4'),
                  ('F', 'DPN', 'D-PHENYLALANINE', 'C9 H11 N O2'),
                  ('H', 'DHI', 'D-HISTIDINE', 'C6 H10 N3 O2 1'),
                  ('I', 'DIL', 'D-ISOLEUCINE', 'C6 H13 N O2'),
                  ('K', 'DLY', 'D-LYSINE', 'C6 H14 N2 O2'),
                  ('L', 'DLE', 'D-LEUCINE', 'C6 H13 N O2'),
                  ('M', 'MED', 'D-METHIONINE', 'C5 H11 N O2 S'),
                  ('N', 'DSG', 'D-ASPARAGINE', 'C4 H8 N2 O3'),
                  ('P', 'DPR', 'D-PROLINE', 'C5 H9 N O2'),
                  ('Q', 'DGN', 'D-GLUTAMINE', 'C5 H10 N2 O3'),
                  ('R', 'DAR', 'D-ARGININE', 'C6 H15 N4 O2 1'),
                  ('S', 'DSN', 'D-SERINE', 'C3 H7 N O3'),
                  ('T', 'DTH', 'D-THREONINE', 'C4 H9 N O3'),
                  ('V', 'DVA', 'D-VALINE', 'C5 H11 N O2'),
                  ('W', 'DTR', 'D-TRYPTOPHAN', 'C11 H12 N2 O2'),
                  ('Y', 'DTY', 'D-TYROSINE', 'C9 H11 N O3')])
    _comps['G'] = PeptideChemComp('GLY', 'G', 'G', name='GLYCINE',
                                  formula="C2 H5 N O2")


class RNAAlphabet(Alphabet):
    """A mapping from one-letter nucleic acid codes (e.g. A) to
       RNA (as :class:`RNAChemComp` objects)."""
    _comps = dict([id, RNAChemComp(id, id, id, name, formula)]
                  for id, name, formula in [
                  ('A', "ADENOSINE-5'-MONOPHOSPHATE", 'C10 H14 N5 O7 P'),
                  ('C', "CYTIDINE-5'-MONOPHOSPHATE", 'C9 H14 N3 O8 P'),
                  ('G', "GUANOSINE-5'-MONOPHOSPHATE", 'C10 H14 N5 O8 P'),
                  ('U', "URIDINE-5'-MONOPHOSPHATE", 'C9 H13 N2 O9 P')])


class DNAAlphabet(Alphabet):
    """A mapping from two-letter nucleic acid codes (e.g. DA) to
       DNA (as :class:`DNAChemComp` objects)."""
    _comps = dict([code, DNAChemComp(code, code, canon, name, formula)]
                  for code, canon, name, formula in [
                      ('DA', 'A', "2'-DEOXYADENOSINE-5'-MONOPHOSPHATE",
                       'C10 H14 N5 O6 P'),
                      ('DC', 'C', "2'-DEOXYCYTIDINE-5'-MONOPHOSPHATE",
                       'C9 H14 N3 O7 P'),
                      ('DG', 'G', "2'-DEOXYGUANOSINE-5'-MONOPHOSPHATE",
                       'C10 H14 N5 O7 P'),
                      ('DT', 'T', "THYMIDINE-5'-MONOPHOSPHATE",
                       'C10 H15 N2 O8 P')])


class EntityRange(object):
    """Part of an entity. Usually these objects are created from
       an :class:`Entity`, e.g. to get a range covering residues 4 through
       7 in `entity` use::

           entity = ihm.Entity(sequence=...)
           rng = entity(4,7)
    """
    def __init__(self, entity, seq_id_begin, seq_id_end):
        if not entity.is_polymeric():
            raise TypeError("Can only create ranges for polymeric entities")
        self.entity = entity
        # todo: check range for validity (at property read time)
        self.seq_id_range = (seq_id_begin, seq_id_end)

    def __eq__(self, other):
        try:
            return (self.entity is other.entity
                    and self.seq_id_range == other.seq_id_range)
        except AttributeError:
            return False

    def __hash__(self):
        return hash((id(self.entity), self.seq_id_range))

    # Use same ID as the original entity
    _id = property(lambda self: self.entity._id)


class Atom(object):
    """A single atom in an entity or asymmetric unit. Usually these objects
       are created by calling :meth:`Residue.atom`.

       Note that this class does not store atomic coordinates of a given
       atom in a given model; for that, see :class:`ihm.model.Atom`.
    """

    __slots__ = ['residue', 'id']

    def __init__(self, residue, id):
        self.residue, self.id = residue, id

    entity = property(lambda self: self.residue.entity)
    asym = property(lambda self: self.residue.asym)
    seq_id = property(lambda self: self.residue.seq_id)


class Residue(object):
    """A single residue in an entity or asymmetric unit. Usually these objects
       are created by calling :meth:`Entity.residue` or
       :meth:`AsymUnit.residue`.
    """

    __slots__ = ['entity', 'asym', 'seq_id', '_range_id']

    def __init__(self, seq_id, entity=None, asym=None):
        self.entity = entity
        self.asym = asym
        # todo: check id for validity (at property read time)
        self.seq_id = seq_id

    def atom(self, atom_id):
        """Get a :class:`Atom` in this residue with the given name."""
        return Atom(residue=self, id=atom_id)

    def _get_auth_seq_id(self):
        return self.asym._get_auth_seq_id_ins_code(self.seq_id)[0]
    auth_seq_id = property(_get_auth_seq_id,
                           doc="Author-provided seq_id; only makes sense "
                               "for asymmetric units")

    def _get_ins_code(self):
        return self.asym._get_auth_seq_id_ins_code(self.seq_id)[1]
    ins_code = property(_get_ins_code,
                        doc="Insertion code; only makes sense "
                            "for asymmetric units")

    def _get_comp(self):
        entity = self.entity or self.asym.entity
        return entity.sequence[self.seq_id - 1]
    comp = property(_get_comp,
                    doc="Chemical component (residue type)")

    # Allow passing residues where a range is requested
    # (e.g. to ResidueFeature)
    seq_id_range = property(lambda self: (self.seq_id, self.seq_id))


class Entity(object):
    """Represent a CIF entity (with a unique sequence)

       :param sequence sequence: The primary sequence, as a sequence of
              :class:`ChemComp` objects, and/or codes looked up in `alphabet`.
       :param alphabet: The mapping from code to chemical components to use
              (it is not necessary to instantiate this class).
       :type alphabet: :class:`Alphabet`
       :param str description: A short text name for the sequence.
       :param str details: Longer text describing the sequence.
       :param source: The method by which the sample for this entity was
              produced.
       :type source: :class:`ihm.source.Source`
       :param references: Information about this entity stored in external
              databases (for example the sequence in UniProt)
       :type references: sequence of :class:`ihm.reference.Reference` objects

       The sequence for an entity can be specified explicitly as a list of
       chemical components, or (more usually) as a list or string of codes,
       or a mixture of both.
       For example::

           # Construct with a string of one-letter amino acid codes
           protein = ihm.Entity('AHMD')
           # Some less common amino acids (e.g. MSE) have three-letter codes
           protein_with_mse = ihm.Entity(['A', 'H', 'MSE', 'D'])

           # Can use a non-default alphabet to make DNA or RNA sequences
           dna = ihm.Entity(('DA', 'DC'), alphabet=ihm.DNAAlphabet)
           rna = ihm.Entity('AC', alphabet=ihm.RNAAlphabet)

           # Can pass explicit ChemComp objects by looking them up in Alphabets
           dna_al = ihm.DNAAlphabet()
           rna_al = ihm.RNAAlphabet()
           dna_rna_hybrid = ihm.Entity((dna_al['DG'], rna_al['C']))

           # For unusual components (e.g. modified residues or ligands),
           # new ChemComp objects can be constructed
           psu = ihm.RNAChemComp(id='PSU', code='PSU', code_canonical='U',
                                 name="PSEUDOURIDINE-5'-MONOPHOSPHATE",
                                 formula='C9 H13 N2 O9 P')
           rna_with_psu = ihm.Entity(('A', 'C', psu), alphabet=ihm.RNAAlphabet)

       For more examples, see the
       `ligands and water example <https://github.com/ihmwg/python-ihm/blob/main/examples/ligands_water.py>`_.

       All entities should be stored in the top-level System object;
       see :attr:`System.entities`.
    """  # noqa: E501

    _force_polymer = None

    def __get_type(self):
        if self.is_polymeric():
            return 'polymer'
        else:
            return 'water' if self.sequence[0].code == 'HOH' else 'non-polymer'
    type = property(__get_type)

    def __get_src_method(self):
        if self.source:
            return self.source.src_method
        elif self.type == 'water':
            return 'nat'
        else:
            return 'man'

    def __set_src_method(self, val):
        raise TypeError("src_method is read-only; assign an appropriate "
                        "subclass of ihm.source.Source to source instead")
    src_method = property(__get_src_method, __set_src_method)

    def __get_weight(self):
        weight = 0.
        for s in self.sequence:
            w = s.formula_weight
            # If any component's weight is unknown, the total is too
            if w:
                weight += w
            else:
                return None
        return weight
    formula_weight = property(
        __get_weight,
        doc="Formula weight (dalton). This is calculated automatically "
            "from that of the chemical components.")

    def __init__(self, sequence, alphabet=LPeptideAlphabet,
                 description=None, details=None, source=None, references=[]):
        def get_chem_comp(s):
            if isinstance(s, ChemComp):
                return s
            else:
                return alphabet._comps[s]
        self.sequence = tuple(get_chem_comp(s) for s in sequence)
        self.description, self.details = description, details
        self.source = source
        self.references = []
        self.references.extend(references)

    def __str__(self):
        return "<ihm.Entity(%s)>" % self.description

    def is_polymeric(self):
        """Return True iff this entity represents a polymer, such as an
           amino acid sequence or DNA/RNA chain (and not a ligand or water)"""
        return (self._force_polymer or
                len(self.sequence) == 0 or
                len(self.sequence) > 1
                and any(isinstance(x, (PeptideChemComp, DNAChemComp,
                                       RNAChemComp)) for x in self.sequence))

    def residue(self, seq_id):
        """Get a :class:`Residue` at the given sequence position"""
        return Residue(entity=self, seq_id=seq_id)

    # Entities are considered identical if they have the same sequence
    def __eq__(self, other):
        return isinstance(other, Entity) and self.sequence == other.sequence

    def __hash__(self):
        return hash(self.sequence)

    def __call__(self, seq_id_begin, seq_id_end):
        return EntityRange(self, seq_id_begin, seq_id_end)

    def __get_seq_id_range(self):
        if self.is_polymeric():
            return (1, len(self.sequence))
        else:
            # Nonpolymers don't have the concept of seq_id
            return (None, None)
    seq_id_range = property(__get_seq_id_range, doc="Sequence range")


class AsymUnitRange(object):
    """Part of an asymmetric unit. Usually these objects are created from
       an :class:`AsymUnit`, e.g. to get a range covering residues 4 through
       7 in `asym` use::

           asym = ihm.AsymUnit(entity)
           rng = asym(4,7)
    """
    def __init__(self, asym, seq_id_begin, seq_id_end):
        if asym.entity is not None and not asym.entity.is_polymeric():
            raise TypeError("Can only create ranges for polymeric entities")
        self.asym = asym
        # todo: check range for validity (at property read time)
        self.seq_id_range = (seq_id_begin, seq_id_end)

    def __eq__(self, other):
        try:
            return (self.asym is other.asym
                    and self.seq_id_range == other.seq_id_range)
        except AttributeError:
            return False

    def __hash__(self):
        return hash((id(self.asym), self.seq_id_range))

    # Use same ID and entity as the original asym unit
    _id = property(lambda self: self.asym._id)
    _ordinal = property(lambda self: self.asym._ordinal)
    entity = property(lambda self: self.asym.entity)


class AsymUnitSegment(object):
    """An aligned part of an asymmetric unit.

       Usually these objects are created from
       an :class:`AsymUnit`, e.g. to get a segment covering residues 1 through
       3 in `asym` use::

           asym = ihm.AsymUnit(entity)
           seg = asym.segment('--ACG', 1, 3)
    """
    def __init__(self, asym, gapped_sequence, seq_id_begin, seq_id_end):
        self.asym = asym
        self.gapped_sequence = gapped_sequence
        self.seq_id_range = (seq_id_begin, seq_id_end)


class AsymUnit(object):
    """An asymmetric unit, i.e. a unique instance of an Entity that
       was modeled.

       Note that this class should not be used to describe crystal waters;
       for that, see :class:`WaterAsymUnit`.

       :param entity: The unique sequence of this asymmetric unit.
       :type entity: :class:`Entity`
       :param str details: Longer text description of this unit.
       :param auth_seq_id_map: Mapping from internal 1-based consecutive
              residue numbering (`seq_id`) to "author-provided" numbering
              (`auth_seq_id` plus an optional `ins_code`). This can be either
              be an int offset, in which case
              ``auth_seq_id = seq_id + auth_seq_id_map`` with no insertion
              codes, or a mapping type (dict, list, tuple) in which case
              ``auth_seq_id = auth_seq_id_map[seq_id]`` with no insertion
              codes, or
              ``auth_seq_id, ins_code = auth_seq_id_map[seq_id]`` - i.e. the
              output of the mapping is either the author-provided number, or a
              2-element tuple containing that number and an insertion code.
              (Note that if a `list` or `tuple` is used for the mapping, the
              first element in the list or tuple does
              **not** correspond to the first residue and will never be used -
              since `seq_id` can never be zero.) The default if
              not specified, or not in the mapping, is for
              ``auth_seq_id == seq_id`` and for no insertion codes to be used.
       :param str id: User-specified ID (usually a string of one or more
              upper-case letters, e.g. A, B, C, AA). If not specified,
              IDs are automatically assigned alphabetically.
       :param str strand_id: PDB or "author-provided" strand/chain ID.
              If not specified, it will be the same as the regular ID.

       See :attr:`System.asym_units`.
    """

    number_of_molecules = 1

    def __init__(self, entity, details=None, auth_seq_id_map=0, id=None,
                 strand_id=None):
        if (entity is not None and entity.type == 'water'
                and not isinstance(self, WaterAsymUnit)):
            raise TypeError("Use WaterAsymUnit instead for creating waters")
        self.entity, self.details = entity, details
        self.auth_seq_id_map = auth_seq_id_map
        self.id = id
        self._strand_id = strand_id

    def _get_auth_seq_id_ins_code(self, seq_id):
        if isinstance(self.auth_seq_id_map, int):
            return seq_id + self.auth_seq_id_map, None
        else:
            try:
                ret = self.auth_seq_id_map[seq_id]
                if isinstance(ret, (int, str)):
                    return ret, None
                else:
                    return ret
            except (KeyError, IndexError):
                return seq_id, None

    def __call__(self, seq_id_begin, seq_id_end):
        return AsymUnitRange(self, seq_id_begin, seq_id_end)

    def residue(self, seq_id):
        """Get a :class:`Residue` at the given sequence position"""
        return Residue(asym=self, seq_id=seq_id)

    def segment(self, gapped_sequence, seq_id_begin, seq_id_end):
        """Get an object representing the alignment of part of this sequence.

           :param str gapped_sequence: Sequence of the segment, including gaps.
           :param int seq_id_begin: Start of the segment.
           :param int seq_id_end: End of the segment.
        """
        # todo: cache so we return the same object for same parameters
        return AsymUnitSegment(self, gapped_sequence, seq_id_begin, seq_id_end)

    seq_id_range = property(lambda self: self.entity.seq_id_range,
                            doc="Sequence range")

    sequence = property(lambda self: self.entity.sequence,
                        doc="Primary sequence")

    strand_id = property(lambda self: self._strand_id or self._id,
                         doc="PDB or author-provided strand/chain ID")


class WaterAsymUnit(AsymUnit):
    """A collection of crystal waters, all with the same "chain" ID.

       :param int number: The number of water molecules in this unit.

       For more information on this class and the rest of the parameters,
       see :class:`AsymUnit`.

    """

    def __init__(self, entity, number, details=None, auth_seq_id_map=0,
                 id=None, strand_id=None):
        if entity.type != 'water':
            raise TypeError(
                "WaterAsymUnit can only be used for water entities")
        super(WaterAsymUnit, self).__init__(
            entity, details=details, auth_seq_id_map=auth_seq_id_map,
            id=id, strand_id=strand_id)
        self.number = number
        self._water_sequence = [entity.sequence[0]] * number

    seq_id_range = property(lambda self: (1, self.number),
                            doc="Sequence range")

    sequence = property(lambda self: self._water_sequence,
                        doc="Primary sequence")

    number_of_molecules = property(lambda self: self.number,
                                   doc="Number of molecules")


class Assembly(list):
    """A collection of parts of the system that were modeled or probed
       together.

       :param sequence elements: Initial set of parts of the system.
       :param str name: Short text name of this assembly.
       :param str description: Longer text that describes this assembly.

       This is implemented as a simple list of asymmetric units (or parts of
       them), i.e. a list of :class:`AsymUnit` and/or :class:`AsymUnitRange`
       objects. An Assembly is typically assigned to one or more of

         - :class:`~ihm.model.Model`
         - :class:`ihm.protocol.Step`
         - :class:`ihm.analysis.Step`
         - :class:`~ihm.restraint.Restraint`

       See also :attr:`System.complete_assembly`
       and :attr:`System.orphan_assemblies`.

       Note that any duplicate assemblies will be pruned on output."""

    #: :class:`Assembly` that is the immediate parent in a hierarchy, or `None`
    parent = None

    def __init__(self, elements=(), name=None, description=None):
        super(Assembly, self).__init__(elements)
        self.name, self.description = name, description


class ChemDescriptor(object):
    """Description of a non-polymeric chemical component used in the
       experiment. For example, this might be a fluorescent probe or
       cross-linking agent. This class describes the chemical structure of
       the component, for example with a SMILES or INCHI descriptor, so that
       it is uniquely defined. A descriptor is typically assigned to a
       :class:`ihm.restraint.CrossLinkRestraint`.

       See :mod:`ihm.cross_linkers` for chemical descriptors of some
       commonly-used cross-linking agents.

       :param str auth_name: Author-provided name
       :param str chem_comp_id: If this chemical is listed in the Chemical
              Component Dictionary, its three-letter identifier
       :param str chemical_name: The systematic (IUPAC) chemical name
       :param str common_name: Common name for the component
       :param str smiles: SMILES string
       :param str smiles_canonical: Canonical SMILES string
       :param str inchi: IUPAC INCHI descriptor
       :param str inchi_key: Hashed INCHI key

       See also :attr:`System.orphan_chem_descriptors`.
    """
    def __init__(self, auth_name, chem_comp_id=None, chemical_name=None,
                 common_name=None, smiles=None, smiles_canonical=None,
                 inchi=None, inchi_key=None):
        self.auth_name, self.chem_comp_id = auth_name, chem_comp_id
        self.chemical_name, self.common_name = chemical_name, common_name
        self.smiles, self.smiles_canonical = smiles, smiles_canonical
        self.inchi, self.inchi_key = inchi, inchi_key


class Collection(object):
    """A collection of entries belonging to single deposition or group.
       These are used by the archive to group multiple related entries,
       e.g. all entries deposited as part of a given study, or all
       models for a genome. An entry (:class:`System`) can belong to
       multiple collections.

       :param str id: Unique identifier (assigned by the archive).
       :param str name: Short name for the collection.
       :param str details: Longer description of the collection.

       See also :attr:`System.collections`.
    """
    def __init__(self, id, name=None, details=None):
        self.id, self.name, self.details = id, name, details
