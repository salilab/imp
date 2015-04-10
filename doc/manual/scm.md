Working with source control {#scm}
===========================

[TOC]

# Avoiding source control entirely {#scm_avoid}

We strongly recommend the use of source control (ideally git). However, if
you have only a small change to make to %IMP, such as a bug fix, and do
not want to use source control, we'd rather get the fix than not! In this
case, make a patch and [email it to us](http://integrativemodeling.org/contact.html) or,
better yet, open up an [issue on GitHub](https://github.com/salilab/imp/issues)
with the patch as a [gist](https://gist.github.com/).

# Basic usage {#scm_basic}

See [the FAQ](@ref faq_git) for some resources on using git. For basic
development of %IMP, work in the default `develop` branch and do the
following:

 - Periodically run `git pull` to get latest 'upstream' changes into your
   copy (if you [made a fork](@ref devsetup), you should also periodically
   [sync it with the main repository](https://help.github.com/articles/syncing-a-fork/).
   `git imp update` is similar but will also show you any new entries from
   the changelog.

 - Commit any changes you make frequently, e.g. `git add <list_of_new_files>`
   and `git commit -a -m "<description of my recent changes>"`.
   git makes it easy to undo, but only if you commit things to git!

 - If working with a fork, use `git push origin develop` periodically to
   store your changes on GitHub.

# Feature branches {#scm_feature}

If you are making a large change that, for example, touches a bunch of files
or will not necessarily be finished before you would like to work on something
else %IMP related, use a `feature` branch. Doing this also allows you to share
the change with other people, before it is
committed into `develop` (e.g. to get them to check it doesn't break anything,
or to test it as you are developing it). We provide tools, based on
[git flow](http://nvie.com/posts/a-successful-git-branching-model/),
to facilitate this process.

 1. Run `git imp feature start feature_name` to start a new feature branch.
    This will open up up an editor where you can write a `README.md` for
    the feature. The contents of the `README.md` will be used to describe
    the branch as well as the final commit message for the branch when
    it is merged into `develop`.
 2. Work on the branch as above (`git commit`, `git add` etc.)
 3. Use `git imp feature publish feature_name` to publish the branch to
    GitHub, if desired. (If you want to use a branch someone else published in
    this way, use `git imp feature track feature_name`.)
 4. Use `git imp feature pull origin` to get any new changes.
 5. Use `git push origin feature/feature_name` to push any new changes to
    the branch to GitHub.

When you are done and want to merge into `develop` do

 1. `git imp feature finish <feature_name>` to merge into `develop`,
    using the contents of the `README.md` as the commit message.
    The `README.md` file is removed before this is done.
 2. `git push origin :feature/<feature_name>` to remove the branch from
    the GitHub repository if you shared it
