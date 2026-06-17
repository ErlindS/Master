Presentation Draft: where-clause in Haskell
---

# 1. Introduction: Local Scope in Haskell
In functional programming, computations often need to be broken down into manageable intermediate steps. Haskell offers two primary constructs for this:

- "let ... in ..." (expression-based): Defines variables before their use. Behaves like a standalone value (expression) and can appear anywhere.
- "where" (declaration-based): Defines variables after their use. Allows for a top-down reading flow and is bound to a specific equation.

---

# 2. Exercise: The Initial Example
Consider the following Haskell code:

```haskell
f x = y + 1
    where y = x * 2

main = print (f 3)
```

Sub-task 1: What is the output?
- Answer: 7
- Evaluation steps:
1. `f 3` is called; i.e., the formal parameter `x` is bound to 3.
2. `y` is calculated in the `where` clause: 3 * 2 = 6.
3. The function body returns `y + 1`, so 6 + 1 = 7.

---

# 3. Exercise: Explanation of `where`
The `where` clause is characterized by the following properties:

- Syntactic construct (not an expression): A `where` clause is not a standalone expression but an integral part of a function declaration or a case branch.
- Top-down readability: It allows the main objective to be formulated in the body first, with auxiliary variables placed below it.
- Scope (lexical scope): Identifiers defined in `where` are visible only within the equation to which the clause is attached. 

# 4. Exercise: Inlining the definition of y
Inlining involves replacing the name of a local variable directly with its definition within the function body.

Original version using `where`:
```haskell
f x = y + 1
    where y = x * 2

-- Inlined version:
f x = (x * 2) + 1
````


---

# 5. Extension: Nested `where` clauses
As required, we now consider the deeper nesting of local bindings. Each definition within a `where` block can itself contain its own `where` block.

Example of nested `where`:
```haskell
f x = y + 1
    where y = x' * 2
        where x' = x + 10
```

Visibility levels (scoping):
- The function body "y + 1" sees only "y". It does not have direct access to "x'".
- The variable "y" sees the parameter "x" as well as its own local variable "x'".
- The variable "x'" sees the parameter "x", as the latter resides in the function's broader scope.

---

# 6. The Rewrite: Nested structures with `let`
Since `let ... in` is an expression, we must ensure the scoping hierarchy is accurately reflected when translating the nested structure.

Rewrite using `let ... in` (bottom-up):
```haskell
f x =
    let y = (let x' = x + 10 in x' * 2)
    in y + 1
```

difference in reading:

- With `where`, we read: "The result is y + 1, where y is twice x', and x' is x + 10." - With `let`, we read: "Calculate `x'`, use it for `y`, and return `y + 1` at the end."

---

# 7. Advantages of `where`: Shared Scope in Guards
The greatest advantage of `where` becomes apparent when using conditions (guards).

Elegant with `where` (no duplication):

```haskell
f x
| x < 3     = y + 1
| x > 3     = y - 1
| otherwise = y
    where
    y = x * 2
```

Here, `y` is visible across all guards.

Clunky with `let ... in` (code duplication):

```haskell
f x
    | x < 3     = let y = x * 2 in y + 1
    | x > 3     = let y = x * 2 in y - 1
    | otherwise = let y = x * 2 in y
```

Since `let` is bound to the individual expression, the calculation must be repeated for each guard.

---

# 8. GHC Core and a complex example 
The Glasgow Haskell Compiler (GHC) reduces the code to a highly simplified intermediate language called GHC Core. At this level, the `where` construct does not exist at all.

GHC Core translation (simplified):

Consider follwing code:
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
Following Code will be shown just for a short amount
https://godbolt.org/z/38qaWE8eq

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

This more readable version will be shown:
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


It will be turned into

Every `where` is internally transformed into a `let` construct. It is pure "syntactic sugar" for better, more declarative readability.

---

# 9. Decision Matrix: When to use which?

`where` clause:
- Is a syntactic declaration construct.
- Is read from top to bottom (result first).
- Is visible across all guards of an equation.
- Is best suited for structural clarity at the function level.

`let ... in` expression:
- Is a standalone expression.
- Is read from bottom to top (derivation first).
- Is visible only within the specific branch. - Best suited for small, local expressions within the control flow (e.g., lambdas).

---