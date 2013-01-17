from SCons.Script import Builder, Action, File
import bug_fixes
import data
import install
import environment

def IMPPublication(env, authors, title, journal, year, description="", url=None):
    basetitle="\\quote{"+title+"}"
    if url:
        title= "\external{"+url+","+basetitle+"}"
    else:
        title=basetitle
    ret= ", ".join(authors)+ ", "+title+", <em>"+journal+"</em>, "+str(year)+"."
    if len(description) >0:
        ret=ret+"\n  <SMALL>"+description+"</SMALL>"
    return ret

def IMPWebsite(env, url, name):
    return "\\external{"+url +", " + name+"}"

def IMPStandardLicense(env):
    return """\\external{http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html,LGPL}.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
"""

def IMPStandardPublications(env):
    return [env.IMPPublication(authors=["Daniel Russel", "Keren Lasker", "Ben Webb", "Dina Schneidman", "Javier Vel&aacute;zquez-Muriel", "Andrej Sali"],
                            title="Putting the pieces together: integrative structure determination of macromolecular assemblies",
                            journal="PLoS Biology",
                            year=2012,
                            description="This paper provides an overview of the key concepts in \\imp and how to apply them to biological problems."),
            env.IMPPublication(authors=["Frank Alber", "Friedrich Foerster", "Dmitry Korkin",  "Maya Topf", "Andrej Sali"], year=2008,
                            title="Integrating diverse data for structure determination of macromolecular assemblies", journal="Annual Review of Biochemistry",
                            description="This paper provides a review of the integrative structure determination methodology and various data sources that can be used.")]

def _unmangle(value):
    return eval(value.get_contents())


def _action_make_module_page(target, source, env):
    fh= file(target[0].path, 'w')
    print >> fh, '/** '+source[0].get_contents()
    print >> fh, "    \\nosubgrouping"
    #print str(source[0])
    #print source[0].get_contents()
    #print type(source[0].get_contents())
    #print >> fh, "\n\\version \n"+str(env['IMP_MODULE_VERSION'])
    #print >> fh, "\n\\brief "+filter(env, source[1].get_contents())
    if source[4].get_contents() != "None":
        print >> fh, "\n"+ source[4].get_contents()
    print >> fh, "\n<b>Author(s):</b> "+", ".join(_unmangle(source[1]))
    print >> fh, "\n<b>Version:</b> "+ source[2].get_contents()+"\n"

    if source[6].get_contents() != "None":
        print >> fh, "\n<b>License:</b> "+source[6].get_contents()
    if source[7].get_contents() != "":
        print >> fh, source[7].get_contents()
    pubs= _unmangle(source[5])
    if pubs and len(pubs) != 0:
        print >> fh, "\n<b>Publications:</b>\n"+"\n".join([" - "+x for x in pubs])
    print >> fh, "*/"
    fh.close()

def _print_module_page(target, source, env):
    print "Making main docpage "+str(target[0])

_MakeModPage = Builder(action=Action(_action_make_module_page,
                                        _print_module_page))

def add_doc_page(env, type,
                 authors,
                 version,
                 brief, overview,
                 publications,
                 license,
                 extra_sections=[]):
    env=bug_fixes.clone_env(env)
    if extra_sections!=[]:
        extras=".\n\n".join(["<b>"+ s[0] +":</b> "+s[1] for s in extra_sections])
    else:
        extras=""
    module= environment.get_current_name(env)
    pg=_MakeModPage(source=[env.Value(type),
                            env.Value(authors),
                            env.Value(version),
                            env.Value(brief),
                            env.Value(overview),
                            env.Value(publications),
                            env.Value(license),
                            env.Value(extras)],
                    target='#/build/doxygen/'+module+'/overview.dox', env=env)
    return pg


def _make_example_overview(target, source, env):
    out= open(target[0].abspath, "w")
    print >> out, "/** \\page examples_index Example index"
    dta= data.get(env)
    for k in dta.examples.keys():
        print >> out, "  -", dta.examples[k].link
    print >> out, "*/"
def _print_example_overview(target, source, env):
    print "Making example overview"
_ExamplesOverview = Builder(action=Action(_make_example_overview,
                                         _print_example_overview))

def _make_example_links(target, source, env):
    print "making example links:", target[0].abspath
    out= open(target[0].abspath, "w")
    # this does not work properly with newer doxygens, or, more likely
    # we were exploiting undefined behavior
    return
    dta= data.get(env)
    methods={}
    classes={}
    for m in dta.modules.keys():
        print m
        methods[m]={}
        classes[m]={}
    for k in dta.examples.keys():
        print k
        for m in dta.examples[k].classes.keys():
            for c in dta.examples[k].classes[m]:
                if not classes[m].has_key(c):
                    classes[m][c]=[]
                classes[m][c].append(k)
        for m in dta.examples[k].methods.keys():
            for c in dta.examples[k].methods[m]:
                if not methods[m].has_key(c):
                    methods[m][c]=[]
                methods[m][c].append(k)
    print classes
    print methods
    for m in dta.modules.keys():
        print m
        if len(methods[m])+ len(classes[m]) > 0:
            if m != "kernel":
                ns= "IMP::"+m+"::"
            else:
                ns="IMP::"
            print >> out, "/**"
            for c in classes[m].keys():
                # don't want those to show up in the class list
                if c != "USAGE" and c != "VERBOSE" and c[0] != '_':
                    print >> out, "\\class", ns+c
                    print >> out, "Examples:"
                    lst= [dta.examples[e].link for e in classes[m][c]]
                print >> out, ", ".join(lst)
            for c in methods[m].keys():
                print >> out, "\\fn", ns+c
                print >> out, "Examples:"
                lst= [dta.examples[e].link for e in methods[m][c]]
                print >> out, ", ".join(lst)
            print >> out, "*/"
def _print_example_links(target, source, env):
    print "Making example links"
_ExamplesLinks = Builder(action=Action(_make_example_links,
                                       _print_example_links))


def _make_applications_overview(target, source, env):
    out= open(target[0].abspath, "w")
    print >> out, "/** \\page applications_index Application index"
    dta= data.get(env)
    for k in dta.applications.keys():
        if dta.applications[k].ok:
            print >> out, "  -", dta.applications[k].link
    print >> out, "*/"
def _print_applications_overview(target, source, env):
    print "Making applications overview"
_ApplicationsOverview = Builder(action=Action(_make_applications_overview,
                                         _print_applications_overview))


def _make_systems_overview(target, source, env):
    out= open(target[0].abspath, "w")
    print >> out, "/** \\page systems_index Systems index"
    print >> out, """A biological system should have each of a several scripts
    - \c setup.py which should be run first in order to set everything up (if it is missing, no setup is required)
    And then one or move of
    - \c sample_i.py: These scripts can take the following arguments:
            - \c -j \c i: an index
            - \c -n \c i: the total number of independent parts to divide the task in to
            - \c --test: run a more limited version of the sampling to test the script
            - \c -i \c path_to_directory: where to find the outputs of the last script (if there was one)
            - \c -o \c path_to_directory: where to write output files
            - \c -d \c path_to_data: where to find input data (defaults to the 'data' directory in the directory containing the script
            .
      That is, running \command{sample_0.py 3 13 output_0} divides the job into 13 independent parts and
      runs part 3 of it. The output will be placed in output_0.
    - \c analyze_i.py which, after all parts of run_i have have completed, performs analysis
      on the results. These scripts also take the -i, -o, and -d arguments.
    .
    The scripts for a given iteration must have completed before the scripts for the next
    iteration may be run (although some applications will include intermediate data, allowing
    users to avoid running all the scripts in order).

    A biological system can be marked as testable meaning that it can be run with only partial sampling by using the "--test" argument.

    Also, a biological system can be marked as parallelizeable which means that, given  \c sample_0.py, \c sample_1.py and \c analyze_0.py exist, the following will work
    work (from any directory):
    - \command{path_to_application/sample_0.py -j 0 -n 3 -o output_0}
    - \command{path_to_application/sample_0.py -j 1 -n 3 -o output_0}
    - \command{path_to_application/sample_0.py -j 2 -n 3 -o output_0}
    to divide the sampling into three parts and run them separately (eg in different machines or cores).

    An application can also contain an local module in a directory called "local". The module will
    be named "system_name_local" where "system_name" is the name of the system in question.


    The names of the directories where to look for inputs and outputs can be specified
    for each of the script. By default, data is found in a directory called "data"
    in the directory containing the scripts. The output for \c sample_i.py is
    \c sample_i in the current directory. The input for \c sample_i.py for i>0
    is \c sample_{i-1}. The input for \c analyze_0.py is the last sample output and the
    output is \c analyzed_0 (and accordingly for later analyze scripts).

    The IMP::system module provides support for writing biological system scripts.
    """
    dta= data.get(env)
    for k in dta.systems.keys():
        print >> out, "  -", dta.systems[k].link
    print >> out, "*/"
def _print_systems_overview(target, source, env):
    print "Making systems overview"
_SystemsOverview = Builder(action=Action(_make_systems_overview,
                                         _print_systems_overview))

def add_doc_files(env, files):
    #pass
    # currently they are all globbed, should fix
    for f in files:
        #print f
        if str(f).endswith(".dox") or str(f).endswith(".dot"):
            pass
        else:
            b=env.Install("#/build/doc/html/", f)
            data.get(env).add_to_alias('doc-files', [b])


def add_overview_pages(env):
    dta= data.get(env)
    sources= [File(str(dta.examples[k].file)) for k in dta.examples.keys()]
    #print [str(x) for x in sources]
    _ExamplesOverview(source=[], target=File("#/build/doxygen/example_overview.dox"),
                     env=env)
    _ExamplesLinks(source=sources, target=File("#/build/doxygen/example_links.dox"),
                  env=env)
    _ApplicationsOverview(source=sources,
                          target=File("#/build/doxygen/applications_overview.dox"),
                          env=env)
    _SystemsOverview(source=sources,
                          target=File("#/build/doxygen/systems_overview.dox"),
                          env=env)
