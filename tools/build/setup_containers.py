#!/usr/bin/env python

"""
Generate the headers and source files for the container related types from the
templates in build/tools/container_templates. These are written to the
build directory.
"""

import sys
import os
import os.path
import glob
import tools

def filter((function_name, type_name, class_name, variable_type, argument_type, return_type,
            storage_type,
            plural_variable_type, plural_argument_type, plural_storage_type,
            index_type, plural_index_type, pass_index_type),
           infile, file_name):
    header_guard_class_name=class_name.upper()
    helpername= class_name.lower()
    return infile\
        .replace("BLURB", """WARNING This file was generated from %s
 *  in %s
 *  by tools/maintenance/setup_containers.py."""%(os.path.split(file_name)[1], os.path.split(file_name)[0]))\
        .replace("HELPERNAME", helpername)\
        .replace("CLASSFUNCTIONNAME", helpername)\
        .replace("FUNCTIONNAME", function_name)\
        .replace("TYPENAME", type_name)\
        .replace("classname", class_name.lower())\
        .replace("Classname", class_name)\
        .replace("CLASSNAME", class_name.upper())\
        .replace("PLURALVARIABLETYPE", plural_variable_type)\
        .replace("PLURALARGUMENTTYPE", plural_argument_type)\
        .replace("PLURALINDEXTYPE", plural_index_type)\
        .replace("PASSINDEXTYPE", pass_index_type)\
        .replace("INDEXTYPE", index_type)\
        .replace("RETURNTYPE", return_type)\
        .replace("VARIABLETYPE", variable_type)\
        .replace("PLURALSTORAGETYPE", plural_storage_type)\
        .replace("ARGUMENTTYPE", argument_type)\
        .replace("STORAGETYPE", storage_type)\
        .replace("FILESOURCE", file_name)


def make_one(source, params, test=True):
    (function_name, type_name, class_name, variable_type, argument_type, return_type,
     storage_type,
     plural_variable_type, plural_argument_type, plural_storage_type,
      index_type, plural_index_type, pass_index_type)= params
    path= os.path.join(source, "tools", "build", "container_templates")
    multi= class_name
    plural_multi= multi+"s"
    cname=function_name

    inputs= tools.get_glob([os.path.join(path,"*", "*.h"), \
                            os.path.join(path,"*", "internal", "*.h"), \
                            os.path.join(path,"*", "*.cpp"),
                            os.path.join(path,"*", "internal", "*.cpp")])
    files=[]
    for ip in inputs:
        p= ip[len(path)+1:]
        module= os.path.split(p)[0]
        rest= os.path.split(p)[1]
        if module.find("internal") != -1:
            module=os.path.split(module)[0]
            rest=os.path.join("internal", rest)
        name=filter(params, rest, rest)
        if p.endswith(".h"):
            out_path= os.path.join("include", "IMP", module, name)
        else:
            out_path= os.path.join("src", module, name)
        files.append((out_path, ip))


    if test:
        files.append(("test/container/test_"+cname + "_restraint.py",
                   path+"/test.py"))
        files.append(("test/container/test_"+cname + "_state.py",
                      path+"/test_state.py"))
    for p in files:
        contents = filter(params, open(p[1], 'r').read(), p[1])
        tools.rewrite(p[0], contents)

def main():
    source= sys.argv[1]
    #(function_name, class_name, variable_type, argument_type,
    # return_type, storage_type,
    #        plural_variable_type, plural_argument_type, plural_storage_type)
    tools.mkdir(os.path.join("src", "core"), clean=False)
    tools.mkdir(os.path.join("src", "container"), clean=False)
    tools.mkdir(os.path.join("src", "kernel"), clean=False)
    make_one(source, ("particle", "Particle", "Singleton", "Particle*", "Particle*",
              "Particle*", "Pointer<Particle>",
              "ParticlesTemp", "ParticlesTemp", "Particles",
              "ParticleIndex", "ParticleIndexes", "ParticleIndex"))
    make_one(source, ("particle_pair", "ParticlePair", "Pair", "ParticlePair", "const ParticlePair&",
              "const ParticlePair", "ParticlePair",
              "ParticlePairsTemp", "ParticlePairsTemp", "ParticlePairs",
              "ParticleIndexPair", "ParticleIndexPairs", "const ParticleIndexPair&"))
    make_one(source, ("particle_triplet", "ParticleTriplet", "Triplet", "ParticleTriplet", "const ParticleTriplet&",
              "const ParticleTriplet", "ParticleTriplet",
              "ParticleTripletsTemp", "ParticleTripletsTemp", "ParticleTriplets",
              "ParticleIndexTriplet", "ParticleIndexTriplets", "const ParticleIndexTriplet&"), test=False)
    make_one(source, ("particle_quad", "ParticleQuad", "Quad", "ParticleQuad", "const ParticleQuad&",
              "const ParticleQuad", "ParticleQuad",
              "ParticleQuadsTemp", "ParticleQuadsTemp", "ParticleQuads",
              "ParticleIndexQuad", "ParticleIndexQuads", "const ParticleIndexQuad&"), test=False)
    #make_one("particle tuple", "ParticlesTemp", "const ParticlesTemp&", "Particles",
    #         "Tuple", "particle tuples", "ParticlesList", "Tuples", test=False)




if __name__ == '__main__':
    main()
