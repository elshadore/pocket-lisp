#ifndef POCKET_STD_H
#define POCKET_STD_H

#define PK_STD_CODE \
 "## Classic when macro. Executes the body is the condition is not nil (true).\n" \
 "(defmacro when (cond :rest body)\n" \
 "    `(if ,cond (progn ,@body) nil))\n" \
 "\n" \
 "## Classic unless macro. Executes the body if the condition is nil.\n" \
 "(defmacro unless (cond :rest body)\n" \
 "    `(if ,cond nil (progn ,@body)))\n" \
 "\n" \
 "## The identity function.\n" \
 "(defun identity (x) x)\n" \
 "\n" \
 "(defun square (x)\n" \
 "   (* x x))\n" \
 "\n" \
 "(defun cube (x)\n" \
 "   (* x x x))\n" \
 "\n" \
 "## Textbook implemenation of a recursive fibonacci seq function.\n" \
 "(defun fib (n)\n" \
 "   (if (<= n 1)\n" \
 "       n\n" \
 "       (+ (fib (- n 1)) (fib (- n 2)))))\n" \
 "\n" \
 "(defun caar (x)\n" \
 "   (car (car x)))\n" \
 "   \n" \
 "(defun cadr (x)\n" \
 "   (car (cdr x)))\n" \
 "   \n" \
 "(defun cdar (x)\n" \
 "   (cdr (car x)))\n" \
 "   \n" \
 "(defun cddr (x)\n" \
 "   (cdr (cdr x)))\n" \
 "\n" \
 "(defun caaar (x)\n" \
 "   (car (car (car x))))\n" \
 "   \n" \
 "(defun caadr (x)\n" \
 "   (car (car (cdr x))))\n" \
 "   \n" \
 "(defun cadar (x)\n" \
 "   (car (cdr (car x))))\n" \
 "   \n" \
 "(defun cdaar (x)\n" \
 "   (cdr (car (car x))))\n" \
 "   \n" \
 "(defun caddr (x)\n" \
 "   (car (cdr (cdr x))))\n" \
 "   \n" \
 "(defun cddar (x)\n" \
 "   (cdr (cdr (car x))))\n" \
 "   \n" \
 "(defun cdadr (x)\n" \
 "   (cdr (car (cdr x))))\n" \
 "   \n" \
 "(defun cdddr (x)\n" \
 "   (cdr (cdr (cdr x))))\n" \
 "\n" \
 "(defun not (x)\n" \
 "   (if x nil true))\n" \
 "\n" \
 "(defmacro cond (:rest clauses)\n" \
 "    (var result nil)\n" \
 "    (var iter (reverse clauses))\n" \
 "    (while iter\n" \
 "        (var clause (car iter))\n" \
 "        (setq result `(if ,(car clause) (progn ,@(cdr clause)) ,result))\n" \
 "        (setq iter (cdr iter)))\n" \
 "    result)\n" \
 "\n" \
 "(defun != (:rest comps)\n" \
 "   (not (apply '= comps)))\n" \
 "\n" \
 "(defmacro catch (symbol :rest body)\n" \
 "    `(funcatch ,symbol (lambda () ,@body)))\n" \
 "\n" \
 "(defmacro block (symbol :rest body)\n" \
 "    `(funcatch ',symbol (lambda () ,@body)))\n" \
 "\n" \
 "(defmacro break (symbol value)\n" \
 "    `(throw ',symbol ,value))\n" \
 "\n" \
 "(defmacro assert= (:rest comps)\n" \
 "    `(unless (= ,@comps) (error \"assert= failed\")))\n" \
 "\n" \
 "## Swap the value of two input symbols. Both inputs have to be symbols for this macro to work.\n" \
 "(defmacro swap (lhs rhs)\n" \
 "    (let ((temp (gensym)))\n" \
 "       `(progn\n" \
 "          (var ,temp ,lhs)\n" \
 "          (setq ,lhs ,rhs)\n" \
 "          (setq ,rhs ,temp))))\n" \
 "\n" \
 "## Basic for loop syntax for iterating over a list.\n" \
 "(defmacro dolist ((el seq) :rest body)\n" \
 "     (let ((temp (gensym)))\n" \
 "         `(let ((,temp ,seq) (,el nil))\n" \
 "            (while ,temp\n" \
 "               (setq ,el (car ,temp))\n" \
 "               ,@body\n" \
 "               (setq ,temp (cdr ,temp))))))\n" \
 "                   \n" \
 "## nil? -> (not x)\n" \
 "## A shortcut function essentially.\n" \
 "(defun nil? (x)\n" \
 "   (not x))\n" \
 "\n" \
 "## We expect a proper list as input. Not nil or a packed list.\n" \
 "(defun last-cons (list)\n" \
 "   (if (nil? (cdr list))\n" \
 "       list\n" \
 "       (last-cons (cdr list))))\n" \
 "\n" \
 "## Has nil checking but otherwise expects a proper list.\n" \
 "(defun make-circular-list (list)\n" \
 "   (when list\n" \
 "       (var last (last-cons list))\n" \
 "       (set-cdr last list)\n" \
 "       list))\n" \
 "\n" \
 "(defmacro setq-slot (object symbol value)\n" \
 "     `(set-slot ,object ',symbol ,value))\n" \
 "     \n" \
 "(defmacro getq-slot (object symbol)\n" \
 "     `(get-slot ,object ',symbol))\n" \
 "\n" \
 "(defun type= (object type)\n" \
 "    (= (typeof object) type))\n" \
 "\n" \
 "(defun assoc-key (alist key)\n" \
 "    (car (assoc alist key)))\n" \
 "     \n" \
 "(defun assoc-value (alist key)\n" \
 "    (cdr (assoc alist key)))\n" \
 "\n" \
 "(defun read-char (stream)\n" \
 "     (->char (read stream 1 :string)))\n" \
 "\n" \
 "(defun read-byte (stream)\n" \
 "     (read stream 1 :byte))\n" \
 "\n" \
 "(defun read-until (stream :opt delimiter)\n" \
 "     (var acc (string-array))\n" \
 "     (var cond true)\n" \
 "     (while cond\n" \
 "          (var char (read-char stream))\n" \
 "          (if (and char (!= char delimiter))\n" \
 "              (append acc char)\n" \
 "              (setq cond nil)))\n" \
 "     (->string acc))\n" \
 "\n" \
 "(defun read-line (stream)\n" \
 "     (read-until stream ^\\newline))\n" \
 "\n" \
 "(defun read-all (stream)\n" \
 "     (read-until stream nil))\n" \
 "\n" \
 "(defun read-user-input-char ()\n" \
 "     (read-char stdin))\n" \
 "\n" \
 "(defun read-user-input ()\n" \
 "     (read-line stdin))\n" \
 "\n" \
 "(defun file-close (stream)\n" \
 "    (when (type= stream 'Stream)\n" \
 "        (destroy stream)))\n" \
 "\n" \
 "## Open the file FILEPATH and read all its contents\n" \
 "(defun file-slurp (filepath)\n" \
 "    (var file (file-open filepath))\n" \
 "    (var result (read-all file))\n" \
 "    (destroy file)\n" \
 "    result)\n" \
 "     \n" \
 "## Load code in from a file path FILE.\n" \
 "(defun load (filepath)\n" \
 "    (evlist (parse-list (file-slurp file))))\n" \
 "\n" \
 "(defun puts (text :opt stream)\n" \
 "    (var stream (or stream stdout))\n" \
 "    (write stream text)\n" \
 "    (flush stream)\n" \
 "    text)\n"

#endif // POCKET_STD_H
