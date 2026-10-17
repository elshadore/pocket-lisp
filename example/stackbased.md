# Example of Stackbased
An example of some stack based syntax.

```c

pk_push_int(lisp, 69);
pk_push_int(lisp, 420);
pk_add(-1, -2);
PKAtom foo = pk_ref_take(lisp, -1);
pk_push(foo);
pk_ref_dec(foo);
pk_print(-1);

```

```c
pk_protect(lisp, NULL, other_function);
```
