# TODO
The todo list for POCKET LISP

## TODAY
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
- [ ] Constant Errors
  - [ ] OOM
  - [ ] Corrupted Memory
  - [ ] Refcount Overflow
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
