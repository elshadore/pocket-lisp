# TODO
The todo list for POCKET LISP

## V1
- [ ] New Object System
  - Properties
  - C Data
  
- [ ] Properties on All Objects
  - Symbols
  - Functions
  - Macros

- [ ] Special Form Objects

- [ ] Iterator objects
  - maybe implemented in lisp (not c)

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
  
- [ ] Remove ternary operator from all c code.
- [ ] `cleared` remove symbol.

- [ ] Testing Suite

- [ ] Unicode Strings and Characters (utf8)
  - [ ] fn, λ, lambda
  
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

- [ ] Emacs Mode Syntax Highlighting
  - basic syntax highlighter

- [ ] Small, Big, Symbol, Pools
  
- [ ] Upgrade to Arbitary Precision Bytecode
  - Not limited to 8bit args
  
- [ ] Document C Functions
- [ ] Documentation Tables

- [ ] Full Code Review
  
## V2
- [ ] Big Numbers
- [ ] Full Float Parsing and Printing
  - [ ] Parsing
  - [ ] Printing
  
- [ ] potential rename to `micro lisp`
  - `.ul` `.ulc` `.uld` `(udex:install "foo")`
- [ ] Custom cfmt
  - for printing atoms as well as c data
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
