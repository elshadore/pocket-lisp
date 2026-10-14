#ifndef POCKET_STD_H
#define POCKET_STD_H

#define PK_STD_CODE \
"(fset 'defmacro (macro (symbol args :rest body)\n" \
"   `(fset ',symbol (macro ,args ,@body))))\n" \
"\n" \
"(defmacro defun (symbol args :rest body)\n" \
"   `(fset ',symbol (lambda ,args ,@body)))\n" \
"\n" \
"(defmacro defvar (symbol :opt value)\n" \
"   `(set ',symbol ,value))\n" \
"  \n" \
"(defmacro fsetq (symbol value)\n" \
"   `(fset ',symbol ,value))\n" \
"\n" \
"(defmacro setq (symbol value)\n" \
"   `(set ',symbol ,value))\n" \
"\n" \
"(defmacro when (cond :rest body)\n" \
"    `(if ,cond (progn ,@body) nil))\n" \
"    \n" \
"(defmacro unless (cond :rest body)\n" \
"    `(if ,cond nil (progn ,@body)))\n" \
"\n" \
"(defun square (x)\n" \
"   (* x x))\n" \
"\n" \
"(defun cube (x)\n" \
"   (* x x x))\n" \
"\n" \
"(defun load (file)\n" \
"    (evlist (read-list (fs/slurp file))))\n" \
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
"(defmacro catch (symbol :rest body)\n" \
"    `(funcatch ,symbol (lambda () ,@body)))\n"


#endif // POCKET_STD_H
