# TODO
The todo list for POCKET LISP

## User Review
Stuff that I need to do a personal deep dive into.
- [ ] Compiler
  - Really need the testing suite for this lmao.
- [ ] VM

## TODAY
- [ ] Object System
  - [ ] CObjects
  - [ ] LObjects
  - [ ] Slots
  - [ ] Methods
- [ ] Types
  - [ ] Streams
  - [ ] Char (Unicode)
    - printing and reading `^\space`
  - [ ] Table
- [ ] Var should return it's new value let setq does.

## Notes
- [ ] Move the throw stuff into the Lisp Environment itself.
  `pk_gets(lisp, pk_str("throw-type"))`
  `pk_gets(lisp, pk_str("throw-value"))`
- [ ] Remove ternary operator.
- [ ] underscored internal functions should be `pk_atom_*` instead
- [ ] special forms should be implemented on the symbol itself. rather than if else in the code.
  so we can tell if a symbol is a special form in the environment

## V1
- [ ] Error Stack Traces
  - debug-trace variable
- [ ] defclass
- [ ] Recursive Printing Check
- [ ] Big Numbers
- [ ] Unicode Strings (utf8)
- [ ] Emacs Mode
  - [ ] Syntax Highlighting
  - [ ] Identation
    - `(declare (indent 1))`
  - Connect to Lisp Server
- [ ] Environment Objects
- [ ] Full Float Parsing and Printing
  - [ ] Parsing
  - [ ] Printing
- [ ] Immutable Vector Update
  - Move from cons cells to immutable vectors.
- [ ] Standard Library
  - [ ] Filesystem API
  - [ ] Shell Command / Process API
  - [ ] TCP/UDP
  - [ ] JSON
  - [ ] HTTP
  - [ ] HTML
- [ ] Testing Suite
- [ ] Full Code Review
- [ ] Upgrade to Arbitary Precision Bytecode
  - Not limited to 8bit args

## V2
- [ ] `Pocket` should not be cached in PKVM and PKCompiler
- [ ] If we call a Lisp function from withing the VM we should not use the C stack to create a new VM, but reuse the old one.
- [ ] NIL and UNDEFINED should be static objects.
- [ ] fn, λ, lambda
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
