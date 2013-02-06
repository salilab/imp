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
