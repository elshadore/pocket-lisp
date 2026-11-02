# TODO
The todo list for POCKET LISP

- [ ] list-funs
- [ ] list-vars
- [ ] list-methods
- [ ] list-slots
- [ ] list-implementors
- [ ] list-types

- [ ] method calling function (in c)
  - we need some kind of ability to method call in c
  - or rather symbol call?
  - push symbol then funcall?
  
- [ ] methods for many things
  - append
  - cat
  - put
  - +
  - eq?
  - everything
  
- [ ] filter
- [ ] map
- [ ] reduce
- [ ] fap
  
- [ ] multiple append

- [ ] `Pocket` should not be cached in PKVM and PKCompiler
- [ ] All possible memory leaks from throwing removed.
  - possible need to rework the way throwing / catching work
  - `pk_protcall`?

- [ ] Streams
  - stdin
  - stdout
  - stderr
- [ ] Special Form Objects

- [ ] Unicode Strings and Characters (utf8)
  - [ ] fn, λ, lambda
- [ ] `string-split`
- [ ] `->bytes`
- [ ] `->string`
- [ ] implement `cat`
  - dispatch on first type
  
- [ ] Create CMake Build
  
- [ ] exotic number parsing
  - 0b010101
  - 0xfeefee
  - 0oaaaaaa <- idk octal
- [ ] Seperation of std into multiple libraries
  - core
  - fs
  - streams
  
- [ ] Remove ternary operator from all c code.
- [ ] underscored internal functions should be `pk_atom_*` instead
- [ ] User Review of Compiler
  - in particular, destructuring bind.
- [ ] User Review of Macro
- [ ] User Review of wether we are allocating too much on the stack in the bytecode VM
   - i.e we are not popping temp values

- [ ] potential rename to `micro lisp`
  - `.ul` `.ulc` `.uld` `(udex:install "foo")`
- [ ] Custom cfmt
  - for printing atoms as well as datums
- [ ] `cleared` remove symbol.
- [ ] Testing Suite
- [ ] `struct-eq`
- [ ] defgeneric??? (maybe not???)
- [ ] Pretty Printing
- [ ] `unwind-protect`
- [ ] `ecase`
- [ ] Error Stack Trace
 - debug-trace variable
- [ ] Emacs Mode
  - [ ] Syntax Highlighting
  - [ ] Identation
    - `(declare (indent 1))`
  - Connect to Lisp Server
- [ ] Full Float Parsing and Printing
  - [ ] Parsing
  - [ ] Printing
- [ ] STD Filesystem API
  - Files (streams)
  - Directories
  - Paths
- [ ] STD Shell Command / Process API
  - Async Command -> Process
  - Sync Command
- [ ] STD TCP/UDP
- [ ] STD JSON
- [ ] STD HTTP
- [ ] STD HTML
- [ ] Upgrade to Arbitary Precision Bytecode
  - Not limited to 8bit args
- [ ] Small, Big, Symbol, Pools
- [ ] `progv`
- [ ] Big Numbers
- [ ] Full Code Review
- [ ] If we call a Lisp function from withing the VM we should not use the C stack to create a new VM, but reuse the old one.
- [ ] Document C Functions
- [ ] Documentation Tables
- [ ] Environment Objects
- [ ] Immutable Vector Update
  - Move from cons cells to immutable vectors.
- [ ] Remove Libc
- [ ] NIL, UNDEFINED, TRUE, should be static objects.
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
- [ ] dragon game
  an example videogame
