# TODO
The todo list for POCKET LISP

## V1
- [ ] Create CMake Build

- [ ] Zig Build not Correctly Regenerating even after Generating Headers

- [ ] PKNum Type
  - also implement this in the parser (for blobs)
- [ ] Public Number Conv
  - also for `as_string` and `as_blob`
- [ ] Vectorized Math
- [ ] Math functions
  need more to list here look at scheme reference
  - sin
  - cos
  - tan
  - exp
  - log
  - asin
  - acos
  - atan
  - sqrt
  - infinite?
  - nan?
  - zero?
  - positive?
  - negetive?
  - odd?
  - even?
  - max
  - min
  - floor/
  - floor-quotient
  - floor-remainder
  - truncate/
  - truncate-quotient
  - truncate-remainder
  - floor
  - ciel
  - truncate
  - round
  - lerp
- [ ] floats
  - printing
  - parsing
  
- [ ] Other Functions
  - ->number for string
  
- [ ] Environment Letting (with Objects)
  - `(let-environment (var-object fun-object) ...)`

- [ ] Pretty Printing

- [ ] Variables
  - number printing style
    - decimal
    - octal
    - hexadecimal
  - gc enabled
  - pretty printing
  
- [ ] User Review of Compiler
  - in particular, destructuring bind.
- [ ] User Review of Macro
- [ ] User Review of wether we are allocating too much on the stack in the bytecode VM
   - i.e we are not popping temp values
  
- [ ] `unwind-protect`
    
- [ ] keyword arguments
  `(def foo (a b :key c d e)
      (print $"{a}, {b}, {c}, {d}, {e}"))`
  
- [ ] Emacs Mode Syntax Highlighting
  - basic syntax highlighter

- [ ] Small, Big, Symbol, Pools
  
- [ ] Upgrade to Arbitary Precision Bytecode
  - Not limited to 8bit args
  
- [ ] Document C Functions
- [ ] Documentation Properties
- [ ] README

- [ ] Remove ternary operator from all c code.

- [ ] Full Code Review
  - Fug Bixes
  
- [ ] Lock Special Form Symbols
  - Cannot Set the Function or Variable

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
