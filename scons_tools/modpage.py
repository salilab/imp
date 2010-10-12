import imp_module
from SCons.Script import Glob, Dir, File, Builder, Action, Exit, Scanner
import SCons

# author
# brief
# overview
# publications
# license

def filter(env, line):
    return line.replace("MODULENAME", "This module")

def unmangle(value):
    return eval(value.get_contents())

def Publication(env, authors, title, journal, year, description=""):
    ret= ", ".join(authors)+", \\quote{"+title+"}, <em>"+journal+"</em>, "+str(year)+"."
    if len(description) >0:
        ret=ret+"\n  <SMALL>"+description+"</SMALL>"
    return ret

def Website(env, url, name):
    return "\\external{"+url +", " + name+"}"

def StandardLicense(env):
    return """\\external{www.gnu.org/licenses/old-licenses/lgpl-2.1.html,LGPL}.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
"""

def StandardPublications(env):
    return [env.Publication(authors=["Daniel Russel", "Keren Lasker", "Ben Webb", "Dina Schneidman", "Javier Vel&aacute;zquez-Muriel", "Andrej Sali"],
                            title="Integrative assembly modeling using IMP",
                            journal="submitted",
                            year=2010,
                            description="This paper provides an overview of the key concepts in \\imp and how to apply them to biological problems."),
            env.Publication(authors=["Frank Alber", "Friedrich Foerster", "Dmitry Korkin",  "Maya Topf", "Andrej Sali"], year=2008,
                            title="Integrating diverse data for structure determination of macromolecular assemblies", journal="Annual Review of Biochemistry",
                            description="This paper provides a review of the integrative structure determination methodology and various data sources that can be used.")]


def _action_make_module_page(target, source, env):
    fh= file(target[0].path, 'w')
    print >> fh, '/** \\namespace '+env['IMP_MODULE_NAMESPACE']
    print >> fh, "    \\nosubgrouping"
    #print str(source[0])
    #print source[0].get_contents()
    #print type(source[0].get_contents())
    #print >> fh, "\n\\version \n"+str(env['IMP_MODULE_VERSION'])
    #print >> fh, "\n\\brief "+filter(env, source[1].get_contents())
    if source[2].get_contents() != "None":
        print >> fh, "\n"+filter(env, source[2].get_contents())
    print >> fh, '\n\nExamples can be found on the \\ref IMP_'+env['IMP_MODULE']+'_examples "IMP.'+env['IMP_MODULE']+' examples" page.\n'
    print >> fh, "\n\\section auth Authorship\nVersion " + str(env['IMP_MODULE_VERSION'])\
          + " by "+", ".join(unmangle(source[0]))

    if source[4].get_contents() != "None":
        print >> fh, "\n\\license "+source[4].get_contents()
    pubs= unmangle(source[3])
    if pubs and len(pubs) != 0:
        print >> fh, "\n\\publications\n"+"\n".join([" - "+x for x in pubs])
    print >> fh, "*/"
    fh.close()

def _print_make_module_page(target, source, env):
    print "Making module main docpage "+str(target[0])

MakeModPage = Builder(action=Action(_action_make_module_page,
                                    _print_make_module_page))
