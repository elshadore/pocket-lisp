#ifndef POCKET_REPL_H
#define POCKET_REPL_H

#define REPL_SRC "(fset 'square\n      (lambda (x)\n        (* x x)))\n\n(set 'repl/running t)\n\n(fset 'repl/quit\n      (lambda ()\n        (set 'repl/running nil)))\n\n(fset 'repl\n      (lambda ()\n        (while repl/running\n          (puts \">> \")\n          (print (format (evlist (read (read-user-input))))))))\n\n(set 'foo 69)\n\n(fset 'example\n      (lambda ()\n        (let ((foo 420))\n          (print foo))))\n\n(repl)\n\n(print \"REPL Quitting...\")\n\n;; (some-stuff)"

#endif
