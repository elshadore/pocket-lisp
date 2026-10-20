# TODO
The todo list for POCKET LISP

## User Review
Stuff that I need to do a personal deep dive into.
- [ ] Compiler
  - Really need the testing suite for this lmao.
- [ ] VM

## TODAY
- [ ] Special Forms
  - [ ] defvar, defun, defmacro
  - [ ] defmacro and macro needs full destructuring bind
- [ ] Object System
  - [ ] CObjects
  - [ ] LObjects
  - [ ] Slots
  - [ ] Methods
- [ ] Types
  - [ ] Streams
  - [ ] Char (Unicode)
  - [ ] Byte Type
  - [ ] Array
  - [ ] Table
  - [ ] Byte Blobs (Immutable like Strings)
  - [ ] StringArray (Mutable Append)
  - [ ] ByteArray (Mutable Append)
- [ ] Var should return it's new value let setq does.

## Notes
- [ ] Move the throw stuff into the Lisp Environment itself.
  `pk_gets(lisp, pk_str("throw-type"))`
  `pk_gets(lisp, pk_str("throw-value"))`
- [ ] Remove ternary operator.
- [ ] If we PROTECT all the way down to the base frame i.e we dont match on our throw. We should have a special error.
- [ ] underscored internal functions should be `pk_atom_*` instead

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
