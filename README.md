# cfind
A program that searches a directory for specified criteria and returns a list of all directories and files that match

This program was created as an assignment for CITS2002 Systems Programming at The University of Western Australia. the functionality and usage is as follows.

>> cfind [options] pathname [stat-expression]

where pathname is the directory to be searched, [stat-expression] is an optional predicate such as 'size < 2M' or 'mtime > [Jan 1 2000]'.

Options supported are:

-a
Normally, file-entries beginning with the '.' character are ignored. Specifying -a requests that all entries, except '.' and '..', be considered.

-c
Print only the count of the number of matching file-entries, then exit. Do not print or do anything else.

-d depth
Normally the indicated filepath is recursively searched (completely). Specifying the -d option limits the search to the indicated depth, descending at most depth levels (a non-negative integer) of directories. -d 0 means only apply the tests and actions to the command-line pathname, and is obviously implied if the command-line pathname is a file.

-l
Print a long listing of matching file-entries, printing in order (left to right): inode, each entry's permissions, number of links, owner's name, group-owner's name, size, modificate-date, and entry-name (similar to the output of /bin/ls -l -i ). 
The listing is sorted by name (the default).

-r
Reverse the order of any sorting options.

-s
Print matching file-entries, sorted by size. If both -s and -t are provided, -t takes precedence.

-t
Print matching file-entries, sorted by modification time. If both -s and -t are provided, -t takes precedence.

-u
Attempt to unlink (remove) as many matching file-entries as possible. The cfind utility should exit with failure if any attempt to unlink a file-entry was unsuccessful.

