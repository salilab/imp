#!/usr/bin/env python
from _decorators import *
import os.path
import os

try:
    os.makedirs(os.path.join("include", "RMF"))
except:
    pass

coordinates = Attribute(
    "coordinates",
    "Vector3")

orientation = Attribute(
    "orientation",
    "Vector4")
mass = Attribute("mass", "Float")
radius = Attribute("radius", "Float")

particle = Decorator(["REPRESENTATION"], "physics",
                     "Particle",
                     [mass, coordinates, radius])
gparticle = Decorator(["REPRESENTATION"], "physics",
                      "GaussianParticle",
                      [Attribute("variances", "Vector3"), mass])
iparticle = Decorator(["REPRESENTATION"], "physics",
                      "IntermediateParticle",
                      [radius, coordinates])
pparticle = Decorator(["REPRESENTATION"], "physics",
                      "RigidParticle",
                      [orientation, coordinates])
refframe = Decorator(["REPRESENTATION", "ORGANIZATIONAL"], "physics",
                     "ReferenceFrame",
                     [Attribute(
                         "rotation", "Vector4"),
                         Attribute(
                             "translation", "Vector3")])


atom = Decorator(["REPRESENTATION"], "physics",
                 "Atom",
                 [Attribute("element", "Int"), mass, radius
                  ])

diffuser = Decorator(["REPRESENTATION"], "physics",
                     "Diffuser",
                     [Attribute("diffusion coefficient", "Float")])

make_header(
    "physics", [particle, iparticle, gparticle, pparticle, diffuser,
                atom, refframe],
    ["alias"])


score = Decorator(["FEATURE"], "feature",
                  "Score",
                  [Attribute("score", "Float")])

make_header("feature", [score], [])


colored = Decorator(
    ["REPRESENTATION", "ORGANIZATIONAL", "ALIAS", "FEATURE", "GEOMETRY"],
    "shape",
    "Colored",
    [Attribute("rgb color", "Vector3")])

geometry_coordinates = Attribute("coordinates list", "Vector3s")

geometry_index = Attribute(
    "type", "Int")

ball = Decorator(["GEOMETRY"], "shape",
                 "Ball",
                 [coordinates, radius])

ellipsoid = Decorator(["GEOMETRY"], "shape",
                      "Ellipsoid",
                      [Attribute("axis lengths", "Vector3"), orientation, coordinates])


cylinder = Decorator(["GEOMETRY"], "shape",
                     "Cylinder",
                     [radius, geometry_coordinates], check_all_attributes=True)

segment = Decorator(["GEOMETRY"], "shape",
                    "Segment",
                    [geometry_coordinates])


make_header("shape", [colored, ball, ellipsoid, cylinder, segment], [])


journal = Decorator(["ORGANIZATIONAL"], "publication",
                    "JournalArticle",
                    [Attribute("title", "String"),
                     Attribute(
                         "journal", "String"),
                     Attribute("pubmed id", "String"),
                     Attribute("year", "Int"),
                     Attribute("authors", "Strings")])

make_header("publication", [journal], [])


residue = Decorator(["REPRESENTATION"], "sequence",
                    "Residue",
                    [Attribute("residue index", "Int"),
                     Attribute("residue type", "String")])

chain = Decorator(["REPRESENTATION"], "sequence",
                  "Chain",
                  [Attribute("chain id", "String")])

domain = Decorator(["REPRESENTATION"], "sequence",
                   "Domain",
                   [RangeAttribute(
                       "residue indexes", "Int", "first residue index",
                       "last residue index")])

fragment = Decorator(["REPRESENTATION"], "sequence",
                     "Fragment",
                     [Attribute("residue indexes", "Ints")])

bcfragment = Decorator(["REPRESENTATION"], "sequence",
                       "BackwardsCompatibilityFragment",
                       [Attribute("indexes", "Ints")])


copy = Decorator(["REPRESENTATION"], "sequence",
                 "Copy",
                 [Attribute("copy index", "Int")])

typed = Decorator(["REPRESENTATION"], "sequence",
                  "Typed",
                  [Attribute("type name", "String")])

state = Decorator(["REPRESENTATION"], "sequence",
                  "State",
                  [Attribute("state index", "Int")])

expres = Decorator(["REPRESENTATION"], "sequence",
                   "ExplicitResolution",
                   [Attribute("explicit resolution", "Float")])


make_header(
    "sequence",
    [residue,
     chain,
     fragment, bcfragment,
     domain,
     typed,
     copy,
     state,
     expres],
    [])


salias = Decorator(["ALIAS"], "alias",
                   "Alias",
                   [NodeAttribute("aliased")])

make_header("alias", [salias], [])


external = Decorator(["REPRESENTATION"], "external",
                     "External",
                     [PathAttribute("path")])

make_header("external", [external], [])
