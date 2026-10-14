#ifndef REPL_H
#define REPL_H

#define REPL_CODE \
"(defvar repl/running true)\n" \
"\n" \
"(defvar repl/result nil)\n" \
"\n" \
"(defun repl/quit ()\n" \
"     (setq repl/running nil))\n" \
"\n" \
"(defun read-eval-print (string)\n" \
"     (setq repl/result (eval (read string)))\n" \
"     (print repl/result)\n" \
"     repl/result)\n" \
"     \n" \
"(when (= (cadr args) \"-e\")\n" \
"    (setq repl/running nil)\n" \
"    (read-eval-print (caddr args)))\n" \
"\n" \
"(while repl/running\n" \
"     (puts \"LISP>> \")\n" \
"     (var input (fs/read-user-input))\n" \
"     (if input\n" \
"        (print (catch 'error (eval (read input))))\n" \
"        (repl/quit)))\n" \
"\n" \
"repl/result\n"


#endif // REPL_H
