# TODO
The todo list for POCKET LISP

## V1
- [ ] `struct-eq`

- [ ] Fix Int to Bytes

- [ ] fix debug stack dump
  - so we see frames clearly
  - print frame name
  
- [ ] PKNum Type
  - also implement this in the parser (for blobs)
- [ ] Public Number Conv
  - also for `as_string` and `as_blob`
- [ ] Vectorized Math
- [ ] Math functions
  need more to list here look at scheme reference
  - sin
  - cos
- [ ] floats
  - printing
  - parsing
  
- [ ] Remove ternary operator from all c code.

- [ ] Testing Suite

- [ ] Environment Letting (with Objects)
  - `(let-environment (var-object fun-object) ...)`

- [ ] Pretty Printing

- [ ] Variables
  - number printing style
    - decimal
    - octal
    - hexadecimal
  - byte number printing style
    - decimal
    - octal
    - hexadecimal
  - gc enabled
  - pretty printing
  - defun / defvar overrides
  - classic defvar behaviour
  
- [ ] Keyword Symbolizer

- [ ] User Review of Compiler
  - in particular, destructuring bind.
- [ ] User Review of Macro
- [ ] User Review of wether we are allocating too much on the stack in the bytecode VM
   - i.e we are not popping temp values
  
- [ ] keyword arguments
  `(def foo (a b :key c d e)
      (print $"{a}, {b}, {c}, {d}, {e}"))`
  
- [ ] Create CMake Build
- [ ] C Build
  - `pocket_all.c`

- [ ] Emacs Mode Syntax Highlighting
  - basic syntax highlighter

- [ ] Small, Big, Symbol, Pools
  
- [ ] Upgrade to Arbitary Precision Bytecode
  - Not limited to 8bit args
  
- [ ] Document C Functions
- [ ] Documentation Properties
- [ ] README

- [ ] Full Code Review

- [ ] Lock Special Form Symbols
  - Cannot Set the Function or Variable

- [ ] Rust Build
   - review `mlua` rust package
   
- [ ] Fug Bixes
  
- [ ] Think about blob endianness
  - control variable for endianness + system endianness
  
- [ ] Zig Build not Correctly Regenerating even after Generating Headers

## V2
- [ ] Seperation of std into multiple libraries (seperation in multiple files)
  - fs
  - system (shell)
  - math
  
- [ ] STD Filesystem API
  - Files
    - read / write
    - tempfile
  - Directories
    - mkdir
  
- [ ] STD Shell Command / Process API
  - Async Command -> Process
  - Sync Command
    - `(shell "echo hello")`
    
- [ ] Unicode Strings and Characters (utf8)
  - [ ] fn, λ, lambda
  
- [ ] Big Numbers
- [ ] Full Float Parsing and Printing
  - [ ] Parsing
  - [ ] Printing
- [ ] Vectors, Matrices, and Quaternions
  
- [ ] potential rename to `micro lisp`
  - `.ul` `.ulc` `.uld` `(udex:install "foo")`
- [ ] Custom cfmt
  - for printing atoms as well as c data
- [ ] Read Common Lisp Standard
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
