from SCons.Script import Builder, Action, File
import bug_fixes
import data

def IMPPublication(env, authors, title, journal, year, description=""):
    ret= ", ".join(authors)+", \\quote{"+title+"}, <em>"+journal+"</em>, "+str(year)+"."
    if len(description) >0:
        ret=ret+"\n  <SMALL>"+description+"</SMALL>"
    return ret

def IMPWebsite(env, url, name):
    return "\\external{"+url +", " + name+"}"

def IMPStandardLicense(env):
    return """\\external{www.gnu.org/licenses/old-licenses/lgpl-2.1.html,LGPL}.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
"""

def IMPStandardPublications(env):
    return [env.IMPPublication(authors=["Daniel Russel", "Keren Lasker", "Ben Webb", "Dina Schneidman", "Javier Vel&aacute;zquez-Muriel", "Andrej Sali"],
                            title="Integrative assembly modeling using IMP",
                            journal="submitted",
                            year=2010,
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
    print >> fh, "\n\\section auth Author(s)\n"+", ".join(_unmangle(source[1]))
    print >> fh, "\n\\section vers Version\n"+ source[2].get_contents()+"\n"

    if source[6].get_contents() != "None":
        print >> fh, "\n\\license "+source[6].get_contents()
    pubs= _unmangle(source[5])
    if pubs and len(pubs) != 0:
        print >> fh, "\n\\publications\n"+"\n".join([" - "+x for x in pubs])
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
                 license):
    env=bug_fixes.clone_env(env)
    pg=_MakeModPage(source=[env.Value(type),
                            env.Value(authors),
                            env.Value(version),
                            env.Value(brief),
                            env.Value(overview),
                            env.Value(publications),
                            env.Value(license)],
                    target='generated/overview.dox', env=env)
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

def _make_module_example_overview(target, source, env):
    module= source[0].get_contents()
    out= open(target[0].abspath, "w")
    print >> out, "/** \\page "+module+"_all_example_index IMP."+module+" example index"
    dta= data.get(env)
    for k in dta.examples.keys():
        if module in dta.examples[k].classes.keys() \
           or module in dta.examples[k].methods.keys():
            print >> out, "  - ", dta.examples[k].link
    print >> out, "*/"
def _print_module_example_overview(target, source, env):
    print "Making IMP."+ source[0].get_contents()+" example overview"
_ModuleExamplesOverview = Builder(action=Action(_make_module_example_overview,
                                         _print_module_example_overview))

def _make_example_links(target, source, env):
    out= open(target[0].abspath, "w")
    return
    dta= data.get(env)
    methods={}
    classes={}
    for m in dta.modules.keys():
        methods[m]={}
        classes[m]={}
    for k in dta.examples.keys():
        for m in dta.examples[k].classes.keys():
            for c in dta.examples[k].classes[m]:
                if not c in classes[m].keys():
                    classes[m][c]=[]
                classes[m][c].append(k)
        for m in dta.examples[k].methods.keys():
            for c in dta.examples[k].methods[m]:
                if not c in methods[m].keys():
                    methods[m][c]=[]
                methods[m][c].append(k)
    for m in dta.modules.keys():
        if len(methods[m])+ len(classes[m]) > 0:
            print >> out, "namespace IMP {"
            if m != "kernel":
                print >> out, "namespace", m, "{"
            print >> out, "/**"
            for c in classes[m].keys():
                print >> out, "\\class", c
                print >> out, "Examples:"
                for e in classes[m][c]:
                    print >> out, " -", dta.examples[e].link
            for c in methods[m].keys():
                print >> out, "\\fn", c
                print >> out, "Examples:"
                for e in methods[m][c]:
                    print >> out, " -", dta.examples[e].link
            print >> out, "*/"
            if m != "kernel":
                print >> out, "}"
            print >> out, "}"
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
    dta= data.get(env)
    for k in dta.systems.keys():
        print >> out, "  -", dta.systems[k].link
    print >> out, "*/"
def _print_systems_overview(target, source, env):
    print "Making systems overview"
_SystemsOverview = Builder(action=Action(_make_systems_overview,
                                         _print_systems_overview))


def add_overview_pages(env):
    dta= data.get(env)
    sources= [File(str(dta.examples[k].file)+".parsed") for k in dta.examples.keys()]
    #print [str(x) for x in sources]
    _ExamplesOverview(source=[], target=File("#/doc/generated/example_overview.dox"),
                     env=env)
    _ExamplesLinks(source=sources, target=File("#/doc/generated/example_links.dox"),
                  env=env)
    _ApplicationsOverview(source=sources,
                          target=File("#/doc/generated/applications_overview.dox"),
                          env=env)
    _SystemsOverview(source=sources,
                          target=File("#/doc/generated/systems_overview.dox"),
                          env=env)
    for m in dta.modules.keys():
        _ModuleExamplesOverview(source=[env.Value(m)]+sources,
                                target=File("#/doc/generated/"+m+"examples_index.dox"),
                                env=env)
