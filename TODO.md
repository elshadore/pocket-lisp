# TODO
The todo list for POCKET LISP

## V1
- [ ] Special Form Objects

- [ ] Destroy Atom
  - `--destroy` method
  - we need some form of dispatch for methods. where we dont care if the method is there or not.
  - `(message hello 'method 1 2 3 4 5)`
  
- [ ] implement `reverse`
  - [ ] string
  - [ ] list
  - [ ] blob

- [ ] multiple append

- [ ] methods for many things
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
  - [ ] ->symbol
  - [ ] ->keyword
  
- [ ] fix debug stack dump
  - so we see frames clearly
  - print frame name
  
- [ ] `string-split`
  
- [ ] exotic number parsing
  - 0b010101
  - 0xfeefee
  - 0oaaaaaa <- idk octal
- [ ] Seperation of std into multiple libraries
  - core
  - fs
  - streams
  
- [ ] there are a bunch of user functions that need to be converted to method calling

- [ ] Remove ternary operator from all c code.
- [ ] underscored internal functions should be `pk_atom_*` instead
- [ ] `cleared` remove symbol.

- [ ] Testing Suite

- [ ] Unicode Strings and Characters (utf8)
  - [ ] fn, λ, lambda
  
- [ ] Big Numbers
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
  
- [ ] `progv`

- [ ] options
  - number printing style
  - gc enabled
  - default reading style

- [ ] User Review of Compiler
  - in particular, destructuring bind.
- [ ] User Review of Macro
- [ ] User Review of wether we are allocating too much on the stack in the bytecode VM
   - i.e we are not popping temp values
- [ ] Typed Destructuring Bind
  - for lambda
  
- [ ] defmethod
  - typed lambda form

- [ ] Create CMake Build

- [ ] Small, Big, Symbol, Pools
  
- [ ] Upgrade to Arbitary Precision Bytecode
  - Not limited to 8bit args
  
- [ ] Document C Functions
- [ ] Documentation Tables

- [ ] Full Code Review
  
## V2
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
- [ ] STD TCP/UDP
- [ ] STD JSON
- [ ] STD HTTP
- [ ] STD HTML
- [ ] STD ncurses
- [ ] If we call a Lisp function from withing the VM we should not use the C stack to create a new VM, but reuse the old one.
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
