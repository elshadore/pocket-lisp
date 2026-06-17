#ifndef POCKET_REPL_H
#define POCKET_REPL_H

#define REPL_SRC "(fset 'square (lambda (x)\n      (* x x)))\n\n(fset 'repl (lambda ()\n      (puts \">> \")\n      (print (format (evlist (read (read-user-input)))))\n      (repl)))\n\n(repl)"

#endif
