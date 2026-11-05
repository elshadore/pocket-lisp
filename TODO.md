# TODO
The todo list for POCKET LISP

- [ ] major bug in blob parsing
  - `(defvar foo [1 2 3 4 5 6])`
  
- [ ] another bug (this time a segfault)
 - `(member? '(1 2 3 4 5 6 69 420) 69)`
 
- [ ] possibility we are pushing a lot of nils onto the stack by not using `pk_protect_noreturn`
  
- [ ] Issues of GC particularly in protect calling in compiler
  - we might need to accept we cant gc collect when compiling
  
- [ ] methods for many things
  - [x] append
  - [x] el
  - [x] length
  - [x] put
  - [x] find
  - [x] remove
  - [x] member?
  - [ ] +
  - [ ] -
  - [ ] /
  - [ ] *
  - [ ] %
  - [ ] <=
  - [ ] <
  - [ ] >
  - [ ] >=
  - [ ] =
  - [ ] cat
  - [ ] ->list
  - [ ] ->string
  - [ ] ->char
  - [ ] ->chars
  - [ ] ->bytes
  - [ ] ->byte
  - [ ] ->int
  - [ ] ->float
  
- [ ] multiple append

- [ ] Streams
  - stdin
  - stdout
  - stderr
- [ ] Special Form Objects

- [ ] array arguments
  - `(array 10 'foo)`
  - better array api
  
- [ ] Unicode Strings and Characters (utf8)
  - [ ] fn, λ, lambda
- [ ] `string-split`
- [ ] `->bytes`
- [ ] `->string`
- [ ] `->list`
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
- [ ] `cleared` remove symbol.

- [ ] Testing Suite

- [ ] User Review of Compiler
  - in particular, destructuring bind.
- [ ] User Review of Macro
- [ ] User Review of wether we are allocating too much on the stack in the bytecode VM
   - i.e we are not popping temp values
- [ ] Typed Destructuring Bind
  - for lambda

- [ ] potential rename to `micro lisp`
  - `.ul` `.ulc` `.uld` `(udex:install "foo")`
- [ ] Custom cfmt
  - for printing atoms as well as datums
- [ ] Read Common Lisp Standard
- [ ] `struct-eq`
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
