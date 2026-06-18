#ifndef POCKET_REPL_H
#define POCKET_REPL_H

#define REPL_SRC "(fset 'square (lambda (x)\n      (* x x)))\n\n(set 'repl/running t)\n\n(fset 'repl/quit (lambda ()\n    (set 'repl/running nil)))\n\n(fset 'repl (lambda ()\n  (while repl/running\n      (puts \">> \")\n      (print (format (evlist (read (read-user-input))))))))\n\n(repl)\n\n(print \"REPL Quitting...\")"

#endif
