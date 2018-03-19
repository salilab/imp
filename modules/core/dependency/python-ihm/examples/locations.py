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
l = ihm.location.WorkflowFileLocation("locations.py",
                       details="The Python script used to generate this "
                               "mmCIF file")
# Add the location to the system, so it gets output to the mmCIF file
system.locations.append(l)

# For public mmCIF files, external files need to also be in a public location,
# for example, in an archive file stored at a service such as Zenodo that
# assigns a DOI. To handle this, we use a Repository object:
r = ihm.location.Repository(doi='10.5281/zenodo.820724',
        url='https://zenodo.org/record/820724/files/archive.zip')
l = ihm.location.OutputFileLocation("densities/subunitA.mrc", repo=r)
system.locations.append(l)

# Users of the mmCIF can then obtain the file subunitA.mrc by downloading
# archive.zip from the given DOI or URL, unzipping it, and then looking in the
# densities directory. Multiple files can share the same repository.

# Datasets are the most common users of external files. For example, to refer
# to an input PDB file in the current directory:
l = ihm.location.InputFileLocation("simple.pdb", details="Input PDB file")
d = ihm.dataset.PDBDataset(l)
# Add to set of all datasets
system.datasets.append(d)

# Generally, datasets will be deposited in an experiment-specific database.
# We can point to such a database using a subclass of DatabaseLocation, for
# example to point to PDB:
l = ihm.location.PDBLocation('1abc')
system.datasets.append(ihm.dataset.PDBDataset(l))

# If the current working directory is itself a checkout of a repository which
# is archived at a DOI, we can retroactively update all 'local' paths added
# above to point to this DOI. After calling update_locations_in_repositories(),
# all files under the parent directory (..) are assumed to be available in
# the python-ihm.zip archive. For example, simple.pdb can be found as
# python-ihm-v0.1/examples/simple.pdb in the archive.
r = ihm.location.Repository(doi='10.5281/zenodo.802915',
        url='https://zenodo.org/record/802915/files/python-ihm.zip',
        top_directory="python-ihm-v0.1", root="..")
system.update_locations_in_repositories([r])

# Write out everything to an mmCIF file
with open('output.cif', 'w') as fh:
    ihm.dumper.write(fh, [system])
