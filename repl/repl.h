#ifndef REPL_H
#define REPL_H

#define REPL_CODE \
 "(defvar repl-running true)\n" \
 "\n" \
 "(defun repl-quit ()\n" \
 "  (setq repl-running nil))\n" \
 "\n" \
 "(defun parse-eval-print (string)\n" \
 "  (print (catch 'error\n" \
 "      (var forms (parse-list string))\n" \
 "      (var result nil)\n" \
 "      (while (not (nil? forms))\n" \
 "         (setq result (eval (car forms)))\n" \
 "         (setq forms (cdr forms)))\n" \
 "      result)))\n" \
 "\n" \
 "(when (= (cadr args) \"-e\")\n" \
 "  (setq repl-running nil)\n" \
 "  (parse-eval-print (caddr args)))\n" \
 "\n" \
 "(while repl-running\n" \
 "  (write \"LISP>> \")\n" \
 "  (var input (read-user-input))\n" \
 "  (if input\n" \
 "      (parse-eval-print input)\n" \
 "    (repl-quit)))\n" \
 "\n" \
 "'quit\n"

#endif // REPL_H
