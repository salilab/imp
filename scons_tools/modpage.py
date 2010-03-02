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

def Publication(env, authors, title, journal, year):
    return ", ".join(authors)+", \""+title+"\", <em>"+journal+"</em>, "+str(year)

def Website(env, url, name):
    return "\\external{"+url +", " + name+"}"

def StandardLicense(env):
    return "None yet"

def StandardPublications(env):
    return [env.Publication(authors=["Daniel Russel", "Keren Lasker", "Ben Webb", "Dina Schneidman", "Javier Valesquez-Muriel", "Andrej Sali"],
                            title="Integrative assembly modeling using IMP",
                            journal="submitted",
                            year=2010)]


def _action_make_module_page(target, source, env):
    fh= file(target[0].path, 'w')
    print >> fh, '/** \\namespace '+env['IMP_MODULE_NAMESPACE']
    #print str(source[0])
    #print source[0].get_contents()
    #print type(source[0].get_contents())
    print >> fh, "\n\\author " + ", ".join(unmangle(source[0]))
    print >> fh, "\n\\version \n"+str(env['IMP_MODULE_VERSION'])
    print >> fh, "\n\\brief "+filter(env, source[1].get_contents())
    if source[2].get_contents() != "None":
        print >> fh, "\n\\overview "+filter(env, source[2].get_contents())
    print >> fh, '\nExamples can be found on the \\ref IMP_'+env['IMP_MODULE']+'_examples "IMP.'+env['IMP_MODULE']+' examples" page.\n'
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
