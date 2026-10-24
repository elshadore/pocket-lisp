# TODO
The todo list for POCKET LISP

## User Review
Stuff that I need to do a personal deep dive into.
- [ ] Compiler
  - Really need the testing suite for this lmao.
- [ ] VM
- [ ] Macro

## V1
- [ ] Type Symbols
- [ ] Object System
  - [ ] CObjects
  - [ ] LObjects
  - [ ] Slots
  - [ ] Methods
- [ ] Method Calling
- [ ] defclass
- [ ] Documentation Tables
- [ ] Recursive Printing Check
- [ ] All possible memory leaks from throwing removed.
- [ ] Better Gensyms
- [ ] defvar, defun, defmacro
  - Return symbol
- [ ] Var should return it's new value let setq does.
- [ ] Special Form Objects
- [ ] Types
  - [ ] Streams
  - [ ] Char (Unicode)
    - printing and reading `^\space`
  - [ ] Table
- [ ] Testing Suite
- [ ] `struct-eq`
  
- [ ] Remove ternary operator.
- [ ] underscored internal functions should be `pk_atom_*` instead
- [ ] Pretty Printing
- [ ] Better Throwing / Erroring
  - Error not throw type
  - If throw cannot find symbol (error)
  - `unwind-protect`
  - `ecase`
  - Error Stack Traces
    - debug-trace variable
- [ ] Unicode Strings (utf8)
  - [ ] fn, λ, lambda
- [ ] Emacs Mode
  - [ ] Syntax Highlighting
  - [ ] Identation
    - `(declare (indent 1))`
  - Connect to Lisp Server
- [ ] Full Float Parsing and Printing
  - [ ] Parsing
  - [ ] Printing
- [ ] Standard Library
  - [ ] Filesystem API
  - [ ] Shell Command / Process API
  - [ ] TCP/UDP
  - [ ] JSON
  - [ ] HTTP
  - [ ] HTML
- [ ] Upgrade to Arbitary Precision Bytecode
  - Not limited to 8bit args
- [ ] Small, Big, Symbol, Pools
- [ ] `progv`
- [ ] Big Numbers
- [ ] Full Code Review

## V2
- [ ] Environment Objects
- [ ] Immutable Vector Update
  - Move from cons cells to immutable vectors.
- [ ] Remove Libc
- [ ] `Pocket` should not be cached in PKVM and PKCompiler
- [ ] If we call a Lisp function from withing the VM we should not use the C stack to create a new VM, but reuse the old one.
- [ ] NIL and UNDEFINED should be static objects.
- [ ] Interpretter Objects
- [ ] Lexical Capture
  - [ ] Upvalues
  - [ ] Compiling Non Special Variables as Stack Variables
  - [ ] Need Eval to Work
- [ ] Modules / Namespaces / Packages
  - `foo:bar-baz`
- [ ] Inputting and Outputting Bytecode
  - Compiling bytecode to a `.pkc` format.
- [ ] Async
  - [ ] Green Threads
  - [ ] Async Dynamic Scope
