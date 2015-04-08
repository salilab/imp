git-imp
========

A version of git-flow customized for IMP from Vincent Driessen's [branching model](http://nvie.com/git-model "original
blog post").


Getting started
---------------
For the best introduction to get started with `git flow`, please read Jeff
Kreeftmeijer's blog post:

[http://jeffkreeftmeijer.com/2010/why-arent-you-using-git-flow/](http://jeffkreeftmeijer.com/2010/why-arent-you-using-git-flow/)

Or have a look at one of these screen casts:

* [How to use a scalable Git branching model called git-flow](http://buildamodule.com/video/change-management-and-version-control-deploying-releases-features-and-fixes-with-git-how-to-use-a-scalable-git-branching-model-called-gitflow) (by Build a Module)
* [A short introduction to git-flow](http://vimeo.com/16018419) (by Mark Derricutt)
* [On the path with git-flow](http://codesherpas.com/screencasts/on_the_path_gitflow.mov) (by Dave Bock)



License terms
-------------
git-flow is published under the liberal terms of the BSD License, see the
[LICENSE](LICENSE) file. Although the BSD License does not require you to share
any modifications you make to the source code, you are very much encouraged and
invited to contribute back your modifications to the community, preferably
in a Github fork, of course.


### Initialization

To initialize a new repo with the basic branch structure, use:

		git imp init

### Creating feature/release/hotfix/support branches

* To list/start/finish feature branches, use:

  		`git imp feature`
  		`git imp feature start <name> [<base>]`
  		`git imp feature finish <name>`

  For feature branches, the `<base>` arg must be a commit on `develop`.

* To push/pull a feature branch to the remote repository, use:

  		`git imp feature publish <name>`
		`git imp feature pull <remote> <name>`
