# TODO
The todo list for POCKET LISP

## User Review
Stuff that I need to do a personal deep dive into.
- [ ] Compiler
  - Really need the testing suite for this lmao.
- [ ] Macro

## V1
- [ ] Add element to array types
- [ ] Add all missing functions to std
- [ ] Cleanup TEMPLATE functions
- [ ] Type idents from symbol name
- [ ] Add printing for Table and Array
```
last time we added code for recursive printing. int pocket_writer.c. we now want to add printing for the array and table types. these should print like such

&<ARRAY::(1 2 3 4 5 6)>

&<TABLE::(("key" . value) (1 . 2) ("foo" . "bar"))>

we obviously need recursive printing checks as well.

after this is done add std functions for append, put, put-unique
```
- [ ] Slurp and read-user-input need to be removed from `pocket.h`
- [ ] Remove template macros
- [ ] Printing
  - Tables
  - Arrays
  - String-Arrays
  - Byte-Arrays
- [ ] `(lambda ())` creates a unimplemented function.
- [ ] `cleared` default throw symbol
- [ ] Type Symbols
- [ ] Object System
  - [ ] CObjects
  - [ ] LObjects
  - [ ] Slots
  - [ ] Methods
- [ ] Method Calling
- [ ] defclass
- [ ] Documentation Tables
- [ ] Recursive Cloning
  - When loading data from bytecode clone.
- [ ] `circular?` function.
- [ ] All possible memory leaks from throwing removed.
- [ ] defvar, defun, defmacro
  - Return symbol
- [ ] Var should return it's new value let setq does.
- [ ] Special Form Objects
- [ ] Types
  - [ ] Streams
    - stdin
    - stdout
    - stderr
  - [ ] Char (Unicode)
    - printing and reading `^\space`
  - [ ] Table
- [ ] Testing Suite
- [ ] `struct-eq`
  
- [ ] defgeneric
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
- [ ] `Pocket` should not be cached in PKVM and PKCompiler
- [ ] If we call a Lisp function from withing the VM we should not use the C stack to create a new VM, but reuse the old one.
- [ ] Document C Functions

## V2
- [ ] Environment Objects
- [ ] Immutable Vector Update
  - Move from cons cells to immutable vectors.
- [ ] Remove Libc
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
