## \example bonds.py
## Show how to extract bonds from a file.
import RMF

def print_bonds(node):
    if (node.get_type()== RMF.BOND):
        print "bond", node.get_children()[0].get_name(), node.get_children()[1].get_name()
    else:
        for c in node.get_children():
            print_bonds(c)

fh = RMF.open_rmf_file_read_only(RMF.get_example_path("simple.rmf"))

print_bonds(fh.get_root_node())
