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
 "## Load code in from a file path FILE.\n" \
 "(defun load (file)\n" \
 "    (evlist (read-list (file-slurp file))))\n" \
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
 "         `(progn\n" \
 "            (let ((,temp ,seq))\n" \
 "                 ((,el nil))\n" \
 "                (while ,temp\n" \
 "                   (setq ,el (car ,temp))\n" \
 "                   (setq ,temp (cdr ,temp)))))))\n" \
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
 "       list))\n"

#endif // POCKET_STD_H
