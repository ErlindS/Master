---
# try also 'default' to start simple
theme: seriph
background: https://cover.sli.dev
title: Bindings in Haskell
info: |
  ## Bindings in Haskell
  Focusing on the `where` and `let` clauses.
class: text-center
highlighter: shiki
drawings:
  persist: false
transition: slide-left
---

# Bindings in Haskell

Focusing on the `where` clause

<div @click="$slidev.nav.next"> </div>

---
transition: fade-out
---

# Introduction

- Welcome to the presentation on bindings in Haskell.
- In functional programming, we often need to define helper functions.
- Haskell offers very elegant solutions for this. Today we'll focus on the `where` clause.

---
transition: slide-up
---

# Local Bindings in Haskell (Where)

Example of using a `where` clause:

```haskell
f x = y + 1
  where y = x * 2

main = print (f 3)
```

<div v-click>

What is the output?
<br>
<span class="text-2xl text-green-500 font-bold">-> 7</span>

</div>

<div v-click class="mt-4 p-4 bg-gray-100 dark:bg-gray-800 rounded-lg">

**Explanation:** <br>
`x` is 3, `y` is calculated as `3 * 2 = 6`, so `f 3` returns the result `6 + 1 = 7`.

</div>

---
transition: slide-up
---

# Explain `where`

- <span class="font-bold text-blue-500">Syntax:</span> The `where` clause is appended after the body of a function or equation.
- <span class="font-bold text-blue-500">Top-down approach:</span> It allows you to formulate the main logic of the function first and move the details (variables, helper functions) below it.
- <span class="font-bold text-blue-500">Scope:</span> The identifiers defined in `where` are only visible within the associated function/equation.
- <span class="font-bold text-red-500">Important:</span> `where` is a syntactic construct for declarations, not a standalone expression.

---
transition: slide-up
---

# Showing the inline variant

As an alternative to `where`, there is the `let ... in ...` expression.

<div class="grid grid-cols-2 gap-8 mt-8">

<div>

**The `where` approach:**
```haskell
f x = y + 1
  where y = x * 2
```

</div>

<div v-click>

**The same example with `let`:**
```haskell
f x = let y = x * 2
      in y + 1
```

</div>

</div>

<div v-click class="mt-8">

- Here, we first define (`let`) and then use (`in`).
- **Difference:** `let` is an expression and can appear anywhere a value is expected.

</div>

---
transition: slide-up
layout: two-cols
layoutClass: gap-4
---

# `where` vs `let` (Guards)

**Using `where` (Clean & Idiomatic)**

```haskell
f x 
  | x < 3     = y + 1
  | x > 3     = y - 1
  | otherwise = y
  where 
    y = x * 2

main = print (f 3)
```

::right::

<div v-click>

# <br>

**Using `let` (Code Duplication)**

```haskell
f x 
  | x < 3     = let y = x * 2 in y + 1
  | x > 3     = let y = x * 2 in y - 1
  | otherwise = let y = x * 2 in y

main = print (f 3)
```

</div>

<div v-click class="mt-4 p-4 bg-red-100 dark:bg-red-900 rounded-lg shadow-md border-l-4 border-red-500 text-sm">

**Why is `let` worse here?** <br>
Because `let` is an expression, we must repeat `let y = x * 2` for every single guard. `where` allows us to share the binding across all guards!

</div>

---
transition: slide-up
layout: two-cols
layoutClass: gap-4
---

# `where` vs `let`

**Using nested `where`**

```haskell
f x 
  | x < 3     = y + 1
  | x > 3     = y - 1
  | otherwise = y
  where 
    y = z + z
      where z = x * x

main = print (f 3)
```

::right::

<div v-click>

# <br>

**Using multiple `lets` (Harder to read)**

```haskell
f x = 
  -- Ein einziger let-Block,
  -- y und z sehen sich gegenseitig
  let y = z + z
      z = x * x
      
  in if x < 3 
     then y + 1
     else if x > 3 
          then y - 1
          else y

main = print (f 3)
```

</div>

<div v-click class="mt-4 p-4 bg-red-100 dark:bg-red-900 rounded-lg shadow-md border-l-4 border-red-500 text-sm">

**Why is `let` worse here?** <br>
We lose the clean, top-down structure of guards and have to use a nested `if-then-else` chain, making the main logic much harder to follow at a glance.

</div>

---
transition: slide-up
layout: two-cols
layoutClass: gap-4
---

# The Illusion of `where`

What we write (Syntactic Sugar):

```haskell
f :: Int -> Int
f x = y + y + 1  
  where y = x * 2
```

<div v-click class="mt-8 p-4 bg-yellow-100 dark:bg-yellow-900 rounded-lg shadow-md border-l-4 border-yellow-500 text-sm">

**The Revelation:** <br>
The `where` clause doesn't actually exist at the machine level! The Glasgow Haskell Compiler (GHC) completely strips it away.

</div>

::right::

<div v-click>

# <br>

What the compiler sees (GHC Core):

```haskell
-- 'where' is translated to a strict 'let'
f = \ x_awu ->
      let { y_awv = * $fNumInt x_awu (I# 2#) } in
      + $fNumInt (+ $fNumInt y_awv y_awv) (I# 1#)
```

</div>

<div v-click class="mt-4 text-sm">

- **No `where`:** It is replaced by an explicit `let ... in` expression.
- **Prefix Notation:** Math becomes prefix (`+ y 1` instead of `y + 1`).
- **Lambdas:** The function arguments are translated into an anonymous lambda `\ x ->`.

</div>

---
transition: slide-up
layout: two-cols
layoutClass: gap-2
---

# Compiling Nested `where`

A complex example with guards and nested `where`s:

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

::right::

<div v-click>

# <br>

**The Compiled Output (GHC Core):**
*(Click to animate through the translation)*

```haskell {all|2-5|6-13|all}
f5 = \ x_azb ->
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

</div>

---
transition: slide-up
layout: two-cols
layoutClass: gap-8
---

# Advantages of `where`

- **High readability:** The main result is at the very top. When someone reads the function, they immediately see the “what” and can look at the “how” in the `where` section if needed.
- **Sharing via guards:** This is the superpower of `where`. A variable defined in the `where` block can be used across multiple pattern-matching guards (`|`) without duplicating the code.

::right::

# Disadvantages of `where`

- **Not an expression:** Since `where` is not an expression, you cannot use it spontaneously “inline” in the middle of another calculation or in a lambda expression.
- **Lack of clarity in very long functions:** If the `where` clause is at the very end of a long function, the definition is spatially far removed from its usage.

---
transition: slide-up
---

# When should `where` be used instead of `let`?

- **With multiple guards:** As soon as multiple conditions require the same local variable, `where` is the idiomatic and clean choice.
- **For complex helper functions:** Idiomatic Haskell often states: “Do this, *where* helper function A does this...”. This promotes structured, understandable code.

<br>
<div v-click class="p-6 bg-blue-100 dark:bg-blue-900 rounded-lg shadow-md border-l-4 border-blue-500">

**Rule of thumb:** <br>
Use `where` for structural definitions and top-down readability at the function level. Use `let` for small, temporary expressions directly “inline”.

</div>

---
transition: slide-up
layout: center
class: text-center
---

# Conclusion

<div class="text-left inline-block">

- Local bindings structure the code and avoid code duplication.
- `where` and `let` are both powerful tools that complement each other perfectly.
- The choice depends on whether you want to focus on the result (-> `where`) or on the step-by-step derivation (-> `let`). 
- However, `where` really shines, especially with guards.

</div>