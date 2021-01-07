# This example demonstrates a variety of ways an IHM mmCIF file can point
# to external resources, such as script files, modeling trajectories, or
# electron microscopy density maps. These may be too big to fit efficiently
# in the mmCIF file, or may already be deposited in an experiment-specific
# database (so it makes no sense to copy them).

import ihm.location
import ihm.dataset
import ihm.dumper

system = ihm.System()

# To point to an external file, we use one of the classes in the ihm.location
# module. Here we reference this Python script itself on the local disk (output
# paths in the mmCIF file will be relative to the current working directory):
loc = ihm.location.WorkflowFileLocation(
    "locations.py",
    details="The Python script used to generate this mmCIF file")
# Add the location to the system, so it gets output to the mmCIF file
system.locations.append(loc)

# For public mmCIF files, external files need to also be in a public location,
# for example, in an archive file stored at a service such as Zenodo that
# assigns a DOI. To handle this, we use a Repository object:
r = ihm.location.Repository(
    doi='10.5281/zenodo.820724',
    url='https://zenodo.org/record/820724/files/archive.zip')
loc = ihm.location.OutputFileLocation("densities/subunitA.mrc", repo=r)
system.locations.append(loc)

# Users of the mmCIF can then obtain the file subunitA.mrc by downloading
# archive.zip from the given DOI or URL, unzipping it, and then looking in the
# densities directory. Multiple files can share the same repository.

# Note that this URL is for example purposes only (there isn't really an
# 'archive.zip' at that DOI).

# Datasets are the most common users of external files. For example, to refer
# to an input PDB file in the current directory:
loc = ihm.location.InputFileLocation("simple.pdb", details="Input PDB file")
d = ihm.dataset.PDBDataset(loc)
# Add the dataset to the mmCIF file. (Normally a dataset would be added to the
# object that uses it, such as a restraint. If we want to include a dataset
# that isn't referenced from anything else, as in this example, we can add it
# to the 'orphan' list.)
system.orphan_datasets.append(d)

# Generally, datasets will be deposited in an experiment-specific database.
# We can point to such a database using a subclass of DatabaseLocation, for
# example to point to PDB:
loc = ihm.location.PDBLocation('1abc')
system.orphan_datasets.append(ihm.dataset.PDBDataset(loc))

# If the current working directory is itself a checkout of a repository which
# is archived at a DOI, we can retroactively update all 'local' paths added
# above to point to this DOI. After calling update_locations_in_repositories(),
# all files under the parent directory (..) are assumed to be available in
# the python-ihm.zip archive. For example, simple.pdb can be found as
# python-ihm-v0.1/examples/simple.pdb in the archive.
r = ihm.location.Repository(
    doi='10.5281/zenodo.802915',
    url='https://zenodo.org/record/802915/files/python-ihm.zip',
    top_directory="python-ihm-v0.1", root="..")
system.update_locations_in_repositories([r])

# Write out everything to an mmCIF file
with open('output.cif', 'w') as fh:
    ihm.dumper.write(fh, [system])
