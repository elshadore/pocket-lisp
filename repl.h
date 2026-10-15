#ifndef REPL_H
#define REPL_H

#define REPL_CODE \
"(defvar repl/running true)\n" \
"\n" \
"(defun repl/quit ()\n" \
"  (setq repl/running nil))\n" \
"\n" \
"(defun read-eval-print (string)\n" \
"  (print (catch 'error (eval (read string)))))\n" \
"\n" \
"(when (= (cadr args) \"-e\")\n" \
"  (setq repl/running nil)\n" \
"  (read-eval-print (caddr args)))\n" \
"\n" \
"(while repl/running\n" \
"  (puts \"LISP>> \")\n" \
"  (var input (fs/read-user-input))\n" \
"  (if input\n" \
"      (read-eval-print input)\n" \
"    (repl/quit)))\n" \
"\n" \
"'quit\n"


#endif // REPL_H
