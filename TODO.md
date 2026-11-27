# TODO
The todo list for POCKET LISP

## V1
- [ ] User Review of Compiler
  - in particular, destructuring bind.
- [ ] User Review of Macro
- [ ] User Review of wether we are allocating too much on the stack in the bytecode VM
   - i.e we are not popping temp values
  
- [ ] Environment Letting (with Objects)
  - `(let-environment (var-object fun-object) ...)`

- [ ] Upgrade to Arbitary Precision Bytecode
  - Not limited to 8bit args
  
- [ ] `unwind-protect`
    
- [ ] keyword arguments
  `(def foo (a b :key c d e)
      (print $"{a}, {b}, {c}, {d}, {e}"))`

- [ ] Lock Special Form Symbols
  - Cannot Set the Function
  - Object on Function &<Object:Special-Form-Marker::(:doc "This is quote o algo.")>

- [ ] Create CMake Build

- [ ] Pretty Printing

- [ ] Variables
  - number printing style
    - decimal
    - octal
    - hexadecimal
  - gc enabled
  - pretty printing
  
- [ ] mathmatical constants
  - pi
  - tau
  - epsilon
  
- [ ] cxr Functions

- [ ] Logic Operations
  - nand
  - nor
  - xnor
  - xor
  - imply
  - nimply
  - converse
  - converse-nimply

- [ ] Generic Slice Functions
  list, array, string, blob
  - slice
  - first
  - second
  - third
  - rest
  
- [ ] array-map, array-filter
  - seq-map, seq-filter, seq-reduce
  
- [ ] floats
  - printing
  - parsing
  
- [ ] More Math Functions
  - `(clamp x min max)`
  - `(wrap x min max)`
  
- [ ] Byte Math
  - `(byte+ 255 1) -> 0`
  Basically just wrapping
  
- [ ] Form of Split that Keeps Blanks
  - `(split "foo  foo   foo " " ") -> "foo" "" "foo" "" "" "foo" ""`
  - `(split "foo  foo   foo " " " true) -> "foo" "foo" "foo"`
  
- [ ] String Slices
  - atom type
- [ ] Blob Slices
  - atom type

- [ ] Small, Big, Symbol, Pools
  
- [ ] Remove ternary operator from all c code.

- [ ] Memory Used
  - `(memory-used) -> 10000`
  - keep track of dynamically allocated memory count
  - also keep track of current pool usage
  
- [ ] Document C Functions
- [ ] Documentation Properties
- [ ] README

- [ ] Full Code Review
  - Fug Bixes
  
- [ ] Build fails without debug flag

- [ ] Emacs Mode Syntax Highlighting
  - basic syntax highlighter

- [ ] Rust Build
   - review `mlua` rust package
   
## V2
- [ ] Testing Suite

- [ ] Think about blob endianness
  - control variable for endianness + system endianness
  
- [ ] Unicode Strings and Characters (utf8)
  - [ ] fn, λ, lambda
- [ ] Inputting and Outputting Bytecode
  - Compiling bytecode to a `.pkc` format.
- [ ] Immutable Vector Update
  - Move from cons cells to immutable vectors.
- [ ] Interpretter Objects
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
- [ ] Seperation of std into multiple libraries (seperation in multiple files)
  - fs
  - system (shell)
  - math
  
- [ ] Big Numbers
- [ ] Full Float Parsing and Printing
  - [ ] Parsing
  - [ ] Printing
- [ ] Vectors, Matrices, and Quaternions
  
- [ ] Namespaces / Modules / Packages
- [ ] Self-like Object System

- [ ] Read Common Lisp Standard
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
- [ ] Remove Libc
- [ ] Lexical Capture
  - [ ] Upvalues
  - [ ] Compiling Non Special Variables as Stack Variables
  - [ ] Need Eval to Work
- [ ] Async
  - [ ] Green Threads
  - [ ] Async Dynamic Scope
- [ ] dragon game
  an example videogame
