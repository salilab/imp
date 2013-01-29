def rewrite(filename, contents):
    try:
        old= open(filename, "r").read()
        if old == contents:
            return
        else:
            print "Different", filename
    except:
        print "Missing", filename
    open(filename, "w").write(contents)
