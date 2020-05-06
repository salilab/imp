Decorators and Attributes {#decoratorsattributes}
=========================

# Overview #

[TOC]

The attributes and decorators are divided into several categories, each associated with a different RMF::Category. Each decorator has three associated types eg RMF::decorator::Particle, RMF::decorator::ParticleConst and RMF::decorator::ParticleFactory. The non-const decorator inherits from the const one. The factory is used to test whether a given node has the needed attributes and to create decorators decorating a given node. Note, that if passed an RMF::NodeConstHandle, the node already having the required attributes is a precondition for the factory `get()` method.

# Static and Frame Attributes # {#staticandframe}

Each `get_` method and `set_` method has three variants. For `get` they are

- `get_foo` get a value for the attribute
- `get_frame_foo` get a value for the attribute as stored in the current frame
- `get_static_foo` get a value for the attribute as stored for the whole file

All have the precondition that the appropriate value exists. For `set`

- `set_frame_foo`: set a value that is stored once per frame
- `set_static_foo`: set a value that is stored once for the whole file
- `set_foo`: if there is no static value, set it, otherwise, if the passed value differs from the static value, set it as a frame value.


# Physics # {#physics}

The category name is `physics`. It includes information about the physical structure of molecules.

## Attributes ## {#physicsattributes}

| Name           | Type         | Description                               |
|---------------:|-------------:|:------------------------------------------|
| `coordinates`  | RMF::Vector3 |  The cartesian coordinates in angstroms   |
| `orientation`  | RMF::Vector4 |  The orientation quaternion               |
| `mass`         | float        |  The mass in Daltons                      |
| `radius`       | float        |  The radius in angstroms                  |
| `rotation`     | RMF::Vector4 |  The rotation quaternion                  |
| `translation`  | RMF::Vector3 |  The translation vector in angstroms      |
| `bonded {0,1}` | int          |  The RMF::NodeID of the bond endpoints    |
| `element`      | int          |  The atomic number of the element         |
| `diffusion coefficient` | float | A diffusion coefficient in A^2/fs  |


## Decorators ## {#physicsdecorators}

| Name                            |  Node Type              | Attributes                         |
|--------------------------------:|:-----------------------:|:-----------------------------------|
| RMF::decorator::Particle        |  RMF::REPRESENTATION    | coordinates, mass, radius          |
| RMF::decorator::ReferenceFrame  |  any                    | rotation, translation              |
| RMF::decorator::Bond            |  RMF::BOND              | bonded {0,1}                       |
| RMF::decorator::Atom            |  RMF::REPRESENTATION    | coordinates, mass, radius, element |
| RMF::decorator::Diffuser        |  RMF::REPRESENTATION    | diffusion coefficient, coordinates |

# Sequence # {#sequence}

The category name is `sequence` and it includes information about the types and indexes of residues.

## Attributes ## {#sequenceattributes}

| Name             | Type      | Description                                      |
|-----------------:|----------:|:-------------------------------------------------|
| `residue type`   | string    |  Three letter residue type                       |
| `residue index`  | int       |  The index of a residue                          |
| `chain id`       | string    |  The chain ID                                    |
| `sequence`       | string    |  Primary sequence of a chain                     |
| `chain type`     | string    |  Type of chain sequence (e.g. Protein, DNA, RNA) |
| `first residue index` | int  |  The index of the first residue (in a domain)    |
| `last residue index` | int   |  The index of the last residue (in a domain)     |
| `residue indexes` | ints      |  The list of indexes in a fragment               |
| `copy index`     | int       |  The index of this particular copy of a protein (when there are multiple copies of the same protein) |
| `type name`      | string       |  An arbitrary type name for a particle. It could be the protein type or ligand type. |
| `state index`    | int       |  An arbitrary integer to label different states of a hierarchy. |
| `explicit resolution` | float     |  Explicitly-specified resolution for a hierarchy. |
| `reference` | int          |  The RMF::NodeID of the reference node    |


## Decorators ## {#sequencedecorators}

| Name                            |  Node Type              | Attributes                         |
|--------------------------------:|:-----------------------:|:-----------------------------------|
| RMF::decorator::Residue         | RMF::REPRESENTATION     |  index, residue type               |
| RMF::decorator::Chain           | RMF::REPRESENTATION     |  chain id, sequence                |
| RMF::decorator::Domain          | RMF::REPRESENTATION     |  first residue index, last residue index |
| RMF::decorator::Fragment        | RMF::REPRESENTATION     |  residue indexes                   |
| RMF::decorator::Copy            | RMF::REPRESENTATION     |  copy index                        |
| RMF::decorator::Typed           | RMF::REPRESENTATION     |  type name                         |
| RMF::decorator::State           | RMF::REPRESENTATION     |  state index                       |
| RMF::decorator::ExplicitResolution | RMF::REPRESENTATION  |  explicit resolution               |
| RMF::decorator::Reference       | RMF::REPRESENTATION     |  pointer to reference node         |

# Uncertainty # {#uncertainty}

The category name is `uncertainty` and it expresses data or theory uncertainty
in the model.

## Attributes ## {#uncertaintyattributes}

| Name             | Type      | Description                                        |
|-----------------:|----------:|:---------------------------------------------------|
| `scale`          | float     | Bayesian nuisance parameter bounded to be positive |
| `scale lower`    | float     | Lower bound of scale value                         |
| `scale upper`    | float     | Upper bound of scale value                         |

# Feature # {#feature}

The category name is `feature` and the data stored relates to scoring functions, scores and which part of the structure they apply to.

## Attributes ## {#featureattributes}

| Name             | Type      | Description                                      |
|-----------------:|----------:|:-------------------------------------------------|
| `score`          | float     |  Some sort of score                              |
| `representation` | RMF::Ints |  The NodeIDs of the nodes involved in this score |

## Decorators ## {#featuredecorators}

| Name                            |  Node Type              | Attributes                         |
|--------------------------------:|:-----------------------:|:-----------------------------------|
| RMF::decorator::Score           | RMF::FEATURE            |  score                             |
| RMF::decorator::Representation  | RMF::FEATURE            |  representation                    |

# Shape # {#shape}

The category name is `shape` and the information relates to geometric markup of the structure.

## Attributes ## {#shapeattributes}

| Name             | Type             | Description                                      |
|-----------------:|-----------------:|:-------------------------------------------------|
| `rgb color`      | RMF::Vector3     |  The red, green, and blue color components; each component is in the range from 0 to 1. |
| `coordinates list` | RMF::Vector3s  |  A list of coordinates.                          |
| `coordinates`    | RMF::Vector3     |  The coordinates in angstroms.                   |
| `radius`         | float            |  The radius in angstroms.                        |
| `axis lengths`   | RMF::Vector3     |  The length along each coordinate axis.          |
| `orientation`    | RMF::Vector4     |  The orientation quaternion.                    |

## Decorators ## {#shapedecorators}

| Name                            |  Node Type              | Attributes                         |
|--------------------------------:|:-----------------------:|:-----------------------------------|
| RMF::decorator::Colored         | any                     |  color                             |
| RMF::decorator::Ball            | RMF::GEOMETRY           |  coordinates, radius               |
| RMF::decorator::Segment         | RMF::GEOMETRY           |  coordinates list                  |
| RMF::decorator::Cylinder        | RMF::GEOMETRY           |  coordinates list, radius          |
| RMF::decorator::Ellipsoid       | RMF::GEOMETRY           |  coordinates, axis lengths, orientation |

# Alternatives # {#alternatives}

The alternatives family is used to store alternative representation schemes for a given
subhierarchy. For example, there can be representations at multiple resolutions or using
a surface instead of particles. Since, in general, one wants to interact with one
representation at a time, the alternative ones are hidden away until explicitly requested.

Each alternative should be viewed as an alternative version of the tree rooted at the
alternative node.

## Attributes ## {#alternativesattributes}

| Name             | Type      | Description                                      |
|-----------------:|----------:|:-------------------------------------------------|
| `types`          | RMF::Ints |  The RMF::decorator::RepresentationType entries for the alternative representations. |
| `roots`          | RMF::Ints |  The RMF::NodeID for each alternative hierarchy root. |


## Decorators ## {#alternativesdecorators}

| Name                            |  Node Type     | Attributes                         |
|--------------------------------:|:--------------:|:-----------------------------------|
| RMF::decorator::Alternatives    | any            |  You can use RMF::decorator::Alternatives::add_alternative() to add an alternative representation and RMF::decorator::Alternatives::get_alternatives() or RMF::decorator::Alternatives::get_alternative() to get them. |


# Alias # {#alias}

The category name is `alias` and it stores information for nodes that provide a reference to another node, for example to provide an alternative organizational scheme.

## Attributes ## {#aliasattributes}

| Name             | Type      | Description                                      |
|-----------------:|----------:|:-------------------------------------------------|
| `alias`          | int       |  NodeID of the node being aliased                |

## Decorators ## {#aliasdecorators}

| Name                            |  Node Type              | Attributes                         |
|--------------------------------:|:-----------------------:|:-----------------------------------|
| RMF::decorator::Alias           | RMF::ALIAS              |  alias                             |

# External # {#external}

The category name is `external` and it stores references to all or part of external files, for example, a path to a PDB file that
should be loaded as a child of the current node.

## Attributes ## {#externalattributes}

| Name             | Type      | Description                                      |
|-----------------:|----------:|:-------------------------------------------------|
| `path`           | string    |  Relative path to the external file              |

## Decorators ## {#externaldecorators}

| Name                            |  Node Type              | Attributes                         |
|--------------------------------:|:-----------------------:|:-----------------------------------|
| RMF::decorator::External           | RMF::REPRESENTATION  |  path                              |


# Publication # {#publication}

## Attributes ## {#publicationattributes}

| Name             | Type      | Description                                      |
|-----------------:|----------:|:-------------------------------------------------|
| `title`          | string    |  Title of the article                            |
| `journal`        | string    |  Title of the journal                            |
| `pubmed id`      | string    |  The pubmed ID                                   |
| `year`           | int       |  The year                                        |
| `authors`        | RMF::Strings |  The author names as "First Last"             |


## Decorators ## {#publicationdecorators}

| Name                            |  Node Type              | Attributes                         |
|--------------------------------:|:-----------------------:|:-----------------------------------|
| RMF::decorator::JournalArticle  | RMF::ORGANIZATIONAL     |  title, journal, pubmed id, year, authors |

# Provenance # {#provenance}

The category name is `provenance`. It includes information about how the
structure was generated.

## Attributes ## {#provenanceattributes}

| Name                  | Type         | Description                               |
|----------------------:|-------------:|:------------------------------------------|
| `structure filename`  | string       |  File from which the structure was read   |
| `structure chain`     | string       |  Chain ID of the structure that was read  |
| `structure residue offset` | int     |  Model residue # = Structure residue # + offset |
| `sampling method`     | string       |  Sampling method utilized                 |
| `sampling frames`     | int          |  Number of frames in the sample ensemble  |
| `sampling iterations` | int          |  Number of sampling iterations used       |
| `sampling replicas`   | int          |  Number of sampling replicas used         |
| `combined runs`       | int          |  Number of sampling runs utilized         |
| `combined frames`     | int          |  Total number of frames combined          |
| `filter method`       | string       |  Filtering method utilized                |
| `filter threshold`    | float        |  Score threshold to discard bad models    |
| `filter frames`       | float        |  Number of frames after filtering         |
| `cluster members`     | int          |  Number of members in a cluster           |
| `script filename`     | string       |  Path of a script used to generate the model |
| `software name`       | string       |  Name of the software used to make the model |
| `software version`    | string       |  Version of the software used to make the model |
| `software location`   | string       |  URI where the software can be found      |


## Decorators ## {#provenancedecorators}

| Name                                |  Node Type          | Attributes                                            |
|------------------------------------:|:-------------------:|:------------------------------------------------------|
| RMF::decorator::StructureProvenance |  RMF::PROVENANCE    | structure filename, structure chain, structure residue offset |
| RMF::decorator::SampleProvenance    |  RMF::PROVENANCE    | sampling method, sampling frames, sampling iterations, sampling replicas |
| RMF::decorator::CombineProvenance   |  RMF::PROVENANCE    | combined runs, combined frames                        |
| RMF::decorator::FilterProvenance    |  RMF::PROVENANCE    | filter threshold, filter frames                       |
| RMF::decorator::ClusterProvenance   |  RMF::PROVENANCE    | cluster members, cluster precision, cluster density   |
| RMF::decorator::ScriptProvenance    |  RMF::PROVENANCE    | script filename                                       |
| RMF::decorator::SoftwareProvenance  |  RMF::PROVENANCE    | software name, software version, software location    |
