# TODO
The todo list for POCKET LISP

## TODAY
### LUA Stack Update
  Update the API to a LUA like api with `setjmp` `longjmp` errors / throws.
  - [x] Remove Errors and Throws
  - [x] Remove Refcounting
  - [x] Add Stack Logic
  - [x] Add Stack API
    - pop
    - popn
    - insert
    - ...
  - [x] C Funcall Update
  - [x] Update STD + pocket_user.c
  - [ ] Add Garbage Collection
  - [x] Remove pk_propagate
  - [ ] Add `setjmp` `longjmp` errors / throws
  - [ ] pk_protect
  
## Notes
- [ ] Add Garbage Collection
- [ ] Macroexpand has removed the code for checking lets
- [ ] underscored internal functions should be `pk_atom_*` instead
- [ ] `Pocket` should not be cached in PKVM and PKCompiler
- [ ] If we call a Lisp function from withing the VM we should not use the C stack to create a new VM, but reuse the old one.

## User Review
Stuff that I need to do a personal deep dive into.
- [ ] Compiler
- [ ] VM
- [ ] Hashmap

## TOMORROW
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
- [ ] Var should destructure bind ???
- [ ] Error Types
  - Error Symbol + Message
- [ ] Error Stack Traces
  - debug-trace variable

## V1
- [ ] Special Forms
  - [ ] fn, λ, lambda
  - [ ] defvar, defun, defmacro, defclass
  - [ ] defmacro needs full destructuring bind
- [ ] Special Form: setq, fsetq, psetq
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
