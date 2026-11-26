#ifndef POCKET_STD_H
#define POCKET_STD_H

#define PK_STD_CODE \
 "(defun document (object :opt doc)\n" \
 "     (put object 'doc doc)\n" \
 "     doc)\n" \
 "     \n" \
 "(defun document-variable (symbol :opt doc)\n" \
 "     (document symbol doc))\n" \
 "\n" \
 "(defun document-function (symbol :opt doc)\n" \
 "     (document (ffetch symbol) doc))\n" \
 "\n" \
 "(defun variable-documentation (symbol)\n" \
 "     (get symbol 'doc))\n" \
 "\n" \
 "(defun function-documentation (symbol)\n" \
 "     (get (ffetch symbol) 'doc))\n" \
 "\n" \
 "## Classic when macro. Executes the body is the condition is not nil (true).\n" \
 "(defmacro when (cond :rest body)\n" \
 "    `(if ,cond (progn ,@body) nil))\n" \
 "\n" \
 "## Classic unless macro. Executes the body if the condition is nil.\n" \
 "(defmacro unless (cond :rest body)\n" \
 "    `(if ,cond nil (progn ,@body)))\n" \
 "\n" \
 "(defun square (x)\n" \
 "   (* x x))\n" \
 "\n" \
 "(document-function 'square \"Squares the number X.\")\n" \
 "\n" \
 "(defun cube (x)\n" \
 "   (* x x x))\n" \
 "\n" \
 "(document-function 'cube \"Cubes the number X.\")\n" \
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
 "(defun type= (object type)\n" \
 "    (= (typeof object) type))\n" \
 "\n" \
 "(defun assoc-key (alist key)\n" \
 "    (car (assoc alist key)))\n" \
 "     \n" \
 "(defun assoc-value (alist key)\n" \
 "    (cdr (assoc alist key)))\n" \
 "\n" \
 "## Load code in from a file path FILE.\n" \
 "(defun load (filepath)\n" \
 "    (evlist (parse-list (file-read-string file))))\n" \
 "\n" \
 "(defun table (:rest properties)\n" \
 "    (apply 'object 'Table properties))\n" \
 "\n" \
 "(defun list? (x)\n" \
 "    (or (nil? x) (cons? x)))\n" \
 "\n" \
 "(defun sequence? (x)\n" \
 "    (or (nil? x) (cons? x) (string? x) (blob? x)))\n" \
 "\n" \
 "(defun cat-a-list (x)\n" \
 "    (when x\n" \
 "        (if (cdr x)\n" \
 "            (apply 'cat x)\n" \
 "            (car x))))\n" \
 "\n" \
 "(defun line-count (string)\n" \
 "    (length (split string \"\\n\")))\n" \
 "\n" \
 "(defun nand (a b)\n" \
 "    (not (and a b)))\n" \
 "\n" \
 "(defun nor (a b)\n" \
 "    (not (or a b)))\n" \
 "\n" \
 "(defvar *file-read-mode* 'string)\n" \
 "\n" \
 "(document-variable '*file-read-mode*\n" \
 "     \"Special variable controlling the read mode for `file-read', values expected to be symbol of `string', or `bytes'\")\n" \
 "\n" \
 "(defun file-read (file :opt mode)\n" \
 "     (var mode (or mode *file-read-mode*))\n" \
 "     (if (= mode 'string)\n" \
 "        (file-read-string file)\n" \
 "        (if (= mode 'bytes)\n" \
 "            (file-read-bytes file)\n" \
 "            (error $\"invalid file read mode: {mode}, expected: string, or bytes\"))))\n"

#endif // POCKET_STD_H
