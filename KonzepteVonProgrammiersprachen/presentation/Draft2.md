Presentation Draft: where-clause in Haskell
---

# 1. Introduction: Local Scope in Haskell
When writing functional code, one often want to split complex tasks into smaller, more readable pieces. Haskell has two main ways to do this:

- `let ... in ...` (expression-based): Define variables *before* one use them. It evaluates to a value, so one can drop it anywhere an expression is allooned.
- `where` (declaration-based): Define variables *after* one use them. This gives one a clean, top-down reading flow. It is bound to the specific function definition.

---

# 2. Exercise: The Initial Example
look at a simple example:

```haskell
f x = y + 1
    where y = x * 2

main = print (f 3)
```

Question: What is the output here?

- Output: 7
- How it evaluates:
  1. one call `f 3`, which binds the parameter `x` to 3.
  2. The `where` clause calculates `y`: `3 * 2 = 6`.
  3. The main function body returns `y + 1`, which is `6 + 1 = 7`.

---

# 3. Exercise: Explanation of `where`
What makes `where` special?

- Syntactic construct (not an expression): It is not a standalone expression. one can't just throw it anywhere. It's part of a function definition or case branch.
- Top-down reading: one write the main goal/logic first, and put the details or helper functions below it.
- Lexical scope: Any variable defined in a `where` block is only visible inside that specific function or equation.

# 4. Exercise: Inlining the definition of y
Inlining is straightforward: one just replaces the variable name with its definition in the function body.

Using `where`:
```haskell
f x = y + 1
    where y = x * 2

-- Inlined version:
f x = (x * 2) + 1
```

---

# 5. Extension: Nested `where` clauses
one can also nest `where` clauses. This means a variable inside a `where` block can have its own local helper defined in another nested `where` block.

Nested `where` example:
```haskell
f x = y + 1
    where y = x' * 2
        where x' = x + 10
```

Who sees what here (Scoping)?
- The main function body (`y + 1`) only sees `y`. It cannot access `x'` directly.
- `y` can see the outer parameter `x` and its own helper `x'`.
- `x'` can see the parameter `x` because `x` is in the broader function scope.

---

# 6. The Rewrite: Nested structures with `let`
If one want to write the same nested structure using `let ... in`, one have to be careful with the scoping hierarchy. Since `let` is an expression, one end up nesting them.

Rewriting with `let ... in` (bottom-up):
```haskell
f x =
    let y = (let x' = x + 10 in x' * 2)
    in y + 1
```

How they read:
- `where` (top-down): "The result is y + 1, where y is twice x', and x' is x + 10."
- `let` (bottom-up): "Calculate x', use it for y, and finally return y + 1."

---

# 7. Advantages of `where`: Shared Scope in Guards
This is where `where` has an advantage. When using guards (conditional branches), one can share a variable across all conditions without repeating onerself.

Clean with `where` (no code repetition):
```haskell
f x
| x < 3     = y + 1
| x > 3     = y - 1
| otherwise = y
    where
    y = x * 2
```
Here, `y` is visible across all guards.

Messy with `let ... in` (lots of duplication):
```haskell
f x
    | x < 3     = let y = x * 2 in y + 1
    | x > 3     = let y = x * 2 in y - 1
    | otherwise = let y = x * 2 in y
```
Because `let` is bound to the individual branch expression, one have to calculate it separately for every guard.

---

# 8. GHC Core and a complex example 
The compiler translates everything into a simplified intermediate language called GHC Core.

Let's take this example:
```haskell
f5 :: Int -> Int
f5 x 
  | x < 3     = y + y + 1
  | x > 3     = y - 1
  | otherwise = y
  where 
    y = z + z  
      where z = x * x
```
one'll show the raw GHC Core briefly (it's pretty hard to read):
https://godbolt.org/z/38qaone8eq

```haskell
f5
  = \ x_azb ->
      let {
        y_azc
          = let { z_azd = * $fNumInt x_azb x_azb } in
            + $fNumInt z_azd z_azd } in
      case < $fOrdInt x_azb (I# 3#) of {
        False ->
          case > $fOrdInt x_azb (I# 3#) of {
            False -> y_azc;
            True -> - $fNumInt y_azc (I# 1#)
          };
        True -> + $fNumInt (+ $fNumInt y_azc y_azc) (I# 1#)
      }
```

Here is a much cleaner, readable translation of what's happening:
```haskell
f5 :: Int -> Int
f5 x = 
  let z = x * x
      y = z + z
  in if x < 3  
     then (y + y) + 1
     else if x > 3 
          then y - 1
          else y
```

As one can see, every `where` is internally transformed into a `let` construct. `where` is pure syntactic sugar to help us write code that's easier to read and structure.

---

# 9. Summary: `where` vs `let`

`where`
- A syntactic declaration construct.
- Read from top to bottom (result first).
- Visible across all guards of an equation.
- Best for structural clarity and keeping function scopes clean.

`let ... in`
- A standalone expression (produces a value).
- Read from bottom to top (derivation first).
- Only visible within its specific branch.
- Best for quick, local inline calculations (like inside a lambda).

---