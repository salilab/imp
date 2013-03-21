#!/usr/bin/env python
from _decorators import *
import os.path
import os

try:
  os.makedirs(os.path.join("include", "RMF"))
except:
  pass

particle= Decorator(["REPRESENTATION"], "physics",
                    "Particle", "These particles has associated coordinates and radius information.",
                    [Attributes("Float", "Floats",
                                "coordinates", ["cartesian x",
                                                "cartesian y",
                                                "cartesian z"],
                                "The coordinates in angstroms."),
                     Attribute("Float", "radius", "radius", "The radius in angstroms."),
                     Attribute("Float", "mass", "mass", "The mass in Daltons.")])
iparticle= Decorator(["REPRESENTATION"], "physics",
                     "IntermediateParticle",
                     "These particles have associated coordinates and radius information.",
                   [Attributes("Float", "Floats",
                               "coordinates", ["cartesian x",
                                               "cartesian y",
                                               "cartesian z"],
                               "The coordinates in angstroms."),
                    Attribute("Float", "radius", "radius",
                              "The radius in angstroms.")])
pparticle= Decorator(["REPRESENTATION"], "physics",
                     "RigidParticle", "These particles have associated coordinates and orientation information.",
                     [Attributes("Float", "Floats",
                                 "orientation", ["orientation r",
                                                 "orientation i",
                                                 "orientation j",
                                                 "orientation k"],
                                 "The orientation as a quaternion."),
                      Attributes("Float", "Floats",
                                 "coordinates", ["cartesian x",
                                                 "cartesian y",
                                                 "cartesian z"],
                                 "The coordinates of the center in angstroms.")])
refframe= Decorator(["REPRESENTATION", "ORGANIZATIONAL"], "physics",
                    "ReferenceFrame",
                    "Define a transformation to be applied the the attributes of this and child particles, relative to the reference frame of the parent.",
                    [Attributes("Float", "Floats",
                                "rotation", ["reference frame orientation r",
                                             "reference frame orientation i",
                                             "reference frame orientation j",
                                             "reference frame orientation k"],
                                "The rotational part of the relative transformation as a quaternion."),
                     Attributes("Float", "Floats",
                                "translation", ["reference frame cartesian x",
                                                "reference frame cartesian y",
                                                "reference frame cartesian z"],
                                "The translation part of the relative transformion in angstroms.")])

bond= Decorator(["BOND"], "physics",
                "Bond", "A bond between particles.",
                [Children("bonded", "The bonded particles.")])

atom= Decorator(["REPRESENTATION"], "physics",
                "Atom", "Information regarding an atom.",
                [Attributes("Float", "Floats",
                            "coordinates", ["cartesian x",
                                            "cartesian y",
                                            "cartesian z"], "The coordinates in angstroms."),
                 Attribute("Float", "radius", "radius", "The radius in angstroms."),
                 Attribute("Float", "mass", "mass", "The mass in Daltons."),
                 Attribute("Index", "element", "element", "The atomic number of the element.")])

diffuser= Decorator(["REPRESENTATION"], "physics",
                    "Diffuser", "Information regarding diffusion coefficients.",
                   [Attribute("Float", "diffusion_coefficient",
                              "diffusion coefficient", "The diffusion coefficient in A^2/fs.")])

force = Decorator(["REPRESENTATION"], "physics",
                        "Force", "Forces acting on particles in kCal/mol/A.",
                        [Attributes("Float", "Floats",
                                   "force", ["force cartesian x",
                                              "force cartesian y",
                                              "force cartesian z"], "The force.")])

torque = Decorator(["REPRESENTATION"], "physics",
                  "Torque", "Torque acting on particles in kCal/mol/radian.",
                        [Attributes("Float", "Floats",
                                    "torque", ["torque cartesian x",
                                               "torque cartesian y",
                                               "torque cartesian z"], "The torque.")])

make_header("physics", [particle, iparticle, pparticle, diffuser,
                        atom, bond, refframe, force, torque],
            ["alias"])





score= Decorator(["FEATURE"], "feature",
                 "Score", "Associate a score with some set of particles. If the score is an energy, it should be in kJ/mol.",
                 [Children("representation", "The various components of this score node."),
                  Attribute("Float", "score", "score", "The score.")])

make_header("feature", [score], [])





colored= Decorator(["REPRESENTATION", "ORGANIZATIONAL", "ALIAS", "FEATURE", "GEOMETRY"],
                   "shape",
                   "Colored", "These particles have associated color information.",
                   [Attributes("Float", "Floats",
                               "rgb_color", ["rgb color red",
                                             "rgb color green",
                                             "rgb color blue"],
                               "The RGB color. Each component has a value in [0...1].")])

ball= Decorator(["GEOMETRY"], "shape",
                "Ball", "A geometric ball.",
                [PluralAttributes("Float", "Floats",
                                  "coordinates", ["cartesian x",
                                                  "cartesian y",
                                                  "cartesian z"],
                                  "Coordinates of the center in angstroms."),
                 Attribute("Float", "radius", "radius", "The radius in angstroms.")],
                internal_attributes=[Attribute("Index", "type", "type", "The type of the geometric object.")],
                init_function = ["nh.set_value(type_, 0);"],
                check_function = ["nh.get_value(type_)==0"])

cylinder= Decorator(["GEOMETRY"], "shape",
                    "Cylinder", "A geometric cylinder.",
                    [PluralAttributes("Floats", "FloatsList",
                                      "coordinates", ["cartesian xs",
                                                      "cartesian ys",
                                                      "cartesian zs"],
                                      "The coordinates of the endpoints in angstroms. The returned list has 3 components, a list of x coordinates, a list of y coordinates and a list of z coordinates."),
                     Attribute("Float", "radius", "radius", "The radius in angstroms.")],
                    internal_attributes=[Attribute("Index", "type", "type", "The type of the geometric object.")],
                    init_function = ["nh.set_value(type_, 1);"],
                    check_function = ["nh.get_value(type_)==1"])

segment= Decorator(["GEOMETRY"], "shape",
                   "Segment", "A geometric line setgment.",
                   [PluralAttributes("Floats", "FloatsList",
                                     "coordinates", ["cartesian xs",
                                                     "cartesian ys",
                                                     "cartesian zs"],
                                     "The coordinates of the endpoints in angstroms. The returned list has 3 components, a list of x coordinates, a list of y coordinates and a list of z coordinates.")],
                   internal_attributes = [Attribute("Index", "type", "type", "The type of the geometric object.")],
                   init_function = ["nh.set_value(type_, 1);"],
                   check_function = ["nh.get_value(type_)==1"])

make_header("shape", [colored, ball, cylinder, segment], [])



journal= Decorator(["ORGANIZATIONAL"], "publication",
                   "JournalArticle", "Information regarding a publication.",
                   [Attribute("String", "title", "title", "The article title."),
                    Attribute("String", "journal", "journal", "The journal title."),
                    Attribute("String", "pubmed_id", "pubmed id", "The pubmed ID."),
                    Attribute("Int", "year", "year", "The publication year."),
                    Attribute("Strings", "authors", "authors", "A list of authors as Lastname, Firstname")])

make_header("publication", [journal], [])





residue= Decorator(["REPRESENTATION"], "sequence",
                   "Residue", "Information regarding a residue.",
                   [SingletonRangeAttribute("Int", "index", "first residue index", "last residue index",
                                            "The index of the residue."),
                    Attribute("String", "type", "residue type", "The three letter name for the residue.")])

chain= Decorator(["REPRESENTATION"], "sequence",
                 "Chain", "Information regarding a chain.",
                 [Attribute("Index", "chain_id", "chain id", "The one letter id for the chain (A is 0 etc).")])

fragment= Decorator(["REPRESENTATION"], "sequence",
                    "Domain", "Information regarding a fragment of a molecule.",
                   [RangeAttribute("Int", "indexes", "first residue index",
                                   "last residue index",
                                   "The range for the residues, specified as [first_index...last_index].")])

copy= Decorator(["REPRESENTATION"], "sequence",
                "Copy", "Information regarding a copy of a molecule.",
                [Attribute("Index", "copy_index", "copy index",
                           "This is the copy_indexth copy of the original.")])

typed= Decorator(["REPRESENTATION"], "sequence",
                 "Typed", "A numeric tag for keeping track of types of molecules.",
                  [Attribute("String", "type_name", "type name", "An arbitrary tag representing the type.")])

make_header("sequence", [residue, chain, fragment, typed, copy], [])



salias= Decorator(["ALIAS"], "alias",
                  "Alias", "Store a reference to another node as an alias.",
                  [NodeAttribute("NodeID", "aliased", "aliased", "The node that is referenced.")])

make_header("alias", [salias], [])


external= Decorator(["REPRESENTATION"], "external",
                    "External", "A reference to something in an external file. A relative path is stored.",
                   [PathAttribute("String", "path", "path", "The absolute path to the external file.")])

make_header("external", [external], [])
