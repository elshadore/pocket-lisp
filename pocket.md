# Notes on ??? Lisp

- Names
  - Pocket Lisp
  - *Zombie Lisp*
    - zlisp, zldex, .zld, zombie, zmach, zli
  - Hyper Lisp
  - Power Lisp
  - Turbo Lisp
  - Delta Lisp
  - Proto Lisp
  - *Ultra Lisp*
    - ulisp, ultra dex, .uld, .ulc, ultra, ultra machine, uli
    

- Simple Namespace System
  - Global Namespace `(in-namespace nil)`
  - Namespace Imports
  - Namespace Objects
  
- Global Type System
  - Types are defined globally then bound to local LISP Interpretters.
    This allows for types to be shared between interpretters.
  - Type Objects
    The ability to refer to the global type by an object.
    
- Referance Counting

- Do not settle on the cons
  Use cons cells during development but be ready to remove them to try out other List implementations.
  
- Environment Objects
  - Store, variables, functions, and object types.
  - Then *let* them.
  - Useful for doing emacs style buffers.

- Variable Declarations
  - `(var foo 69)`
  - Can still use let, but procedural block structure code can be done.

- Gensyms
- Ghostsyms
