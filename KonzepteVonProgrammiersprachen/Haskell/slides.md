---
theme: seriph
title: Bindings in Haskell
info: |
  ## Bindings in Haskell
  Focusing on the `where` and `let` clauses.
class: text-center
highlighter: shiki
transition: slide-left
colorSchema: light
---

# Bindings in Haskell

Focusing on the `where` clause

<div @click="$slidev.nav.next"> </div>

---
transition: fade-out
---

# 1. Introduction: Local Scope in Haskell

<div class="grid grid-cols-2 gap-6 mt-12">

<div class="p-5 bg-gray-50 rounded-lg border border-gray-200 opacity-60 shadow-sm">
  <span class="text-xs uppercase font-bold text-gray-400 tracking-wider">Expression-Based</span>
  <div class="font-mono font-bold text-base text-gray-500 mt-2">let ... in ...</div>
  <p class="text-xs mt-3 text-gray-500 leading-relaxed">
    Defines variables <strong>before</strong> their use. Behaves like a standalone value (expression) and can appear anywhere a value is expected.
  </p>
</div>

<div class="p-5 bg-blue-50 rounded-lg border border-blue-200 shadow-md">
  <span class="text-xs uppercase font-bold text-blue-500 tracking-wider">Declaration-Based</span>
  <div class="font-mono font-bold text-base text-blue-600 mt-2">where</div>
  <p class="text-xs mt-3 text-blue-800 leading-relaxed font-semibold">
    Defines variables <strong>after</strong> their use. Allows for a declarative, top-down reading flow and is bound to a specific equation.
  </p>
</div>

</div>

---
transition: slide-up
---

# 2. Exercise: The Initial Example

Consider the following Haskell code:

<div class="grid grid-cols-2 gap-8 mt-4">

<div>

```haskell
f x = y + 1
    where y = x * 2

main = print (f 3)
```

</div>

<div>

<v-click :at="1">
  <div class="p-4 bg-gray-50 rounded-lg border border-gray-150 text-xs">
    <strong>Sub-task 1:</strong> What is the output?
    <div v-click :at="5" class="mt-2 text-sm font-bold text-green-600">Answer: 7</div>
  </div>
</v-click>

<v-click :at="3">
  <div class="mt-4 p-4 bg-blue-50 border border-blue-200 rounded-lg text-xs leading-relaxed">
    <strong>Evaluation steps:</strong>
    <ol class="list-decimal list-inside space-y-1 mt-2 text-gray-700">
      <li><code>f 3</code> is called; formal parameter <code>x</code> is bound to 3.</li>
      <li v-click :at="3"><code>y</code> is calculated in the <code>where</code> clause: <code>3 * 2 = 6</code>.</li>
      <li v-click :at="4">The function body returns <code>y + 1</code>, so <code>6 + 1 = 7</code>.</li>
    </ol>
  </div>
</v-click>

</div>

</div>

---
transition: slide-up
---

# 3. Exercise: Explanation of `where`

<div class="grid grid-cols-3 gap-6 mt-12">

<div class="p-5 bg-blue-50 rounded-lg border border-blue-150 shadow-sm flex flex-col justify-between">
  <div>
    <div class="flex items-center gap-2 font-bold text-blue-700 text-base">
      Syntactic Construct
    </div>
    <p class="text-xs mt-3 text-blue-800 leading-relaxed">
      <br><br>
      A <code>where</code> clause is not a standalone expression but an part of a function or a case branch.
    </p>
  </div>
</div>

<div class="p-5 bg-blue-50 rounded-lg border border-blue-150 shadow-sm flex flex-col justify-between">
  <div>
    <div class="flex items-center gap-2 font-bold text-blue-700 text-base">
      Top-Down Readability
    </div>
    <p class="text-xs mt-3 text-blue-800 leading-relaxed">
      <br><br>
      It allows the main task to be formulated first.
    </p>
  </div>
</div>

<div class="p-5 bg-blue-50 rounded-lg border border-blue-150 shadow-sm flex flex-col justify-between">
  <div>
    <div class="flex items-center gap-2 font-bold text-blue-700 text-base">
      Lexical Scope
    </div>
    <p class="text-xs mt-3 text-blue-800 leading-relaxed">
      <br><br>
      Identifiers defined in a <code>where</code> block are visible only within the equation/declaration.
    </p>
  </div>
</div>

</div>

---
transition: slide-up
---

# 4. Exercise: Inlining the definition of y

<div class="grid grid-cols-2 gap-8 mt-12">

<div>
<strong>Original version using `where`:</strong>

```haskell {lines: true}
f x = y + 1
    where y = x * 2
```
</div>

<div v-click>
<strong>Inlined version:</strong>

```haskell {lines: true}
f x = (x * 2) + 1
```

<div class="mt-8 p-4 bg-gray-50 border border-gray-200 rounded-lg text-xs leading-relaxed text-gray-600">
  Inlining eliminates the local naming construct. It can decrease readability when the sub-expression is complex or reused.
</div>
</div>

</div>

---
transition: slide-up
---

# 5. Extension: Nested `where` clauses

Each definition within a `where` block can itself contain its own nested `where` block.

<div class="grid grid-cols-2 gap-8 mt-6">

<div>
<strong>Example of nested `where`:</strong>

```haskell {lines: true}
f x = y + 1
    where y = x' * 2
        where x' = x + 10
```
</div>

<div>
<strong>Visibility levels (scoping):</strong>

<ul class="space-y-4 text-xs mt-4">
  <v-click>
    <li class="flex items-start gap-2">
      <span class="text-blue-500 font-bold">▪</span>
      <span>The function body <code>y + 1</code> sees only <code>y</code>. It does <strong>not</strong> have direct access to <code>x'</code>.</span>
    </li>
  </v-click>
  <v-click>
    <li class="flex items-start gap-2">
      <span class="text-blue-500 font-bold">▪</span>
      <span>The variable <code>y</code> sees the parameter <code>x</code> as well as its own local variable <code>x'</code>.</span>
    </li>
  </v-click>
  <v-click>
    <li class="flex items-start gap-2">
      <span class="text-blue-500 font-bold">▪</span>
      <span>The variable <code>x'</code> sees the parameter <code>x</code>, as the latter resides in the function's broader scope.</span>
    </li>
  </v-click>
</ul>
</div>

</div>

---
transition: slide-up
---

# 6. The Rewrite: Nested structures with `let`

<div class="grid grid-cols-2 gap-8 mt-4">

<div>
Rewrite using `let ... in` (bottom-up):

```haskell {lines: true}
f x =
    let y = (let x' = x + 10 in x' * 2)
    in y + 1
```
</div>

<div>
Difference in reading flow:

<v-clicks>
<div class="p-4 bg-blue-50 border border-blue-200 rounded-lg text-xs mt-2">
  <strong>where (top-down):</strong>
  <p class="italic mt-1 text-blue-900">"The result is y + 1, where y is twice x', and x' is x + 10."</p>
</div>

<div class="p-4 bg-yellow-50 border border-yellow-250 rounded-lg text-xs mt-4">
  <strong>let (bottom-up):</strong>
  <p class="italic mt-1 text-yellow-900">"Calculate x', use it for y, and return y + 1 at the end."</p>
</div>
</v-clicks>
</div>

</div>

---
transition: slide-up
---

# 7. Advantages of `where`: Shared Scope in Guards

<div class="grid grid-cols-2 gap-8 mt-4">

<div>
Elegant with `where` (no duplication):

```haskell {lines: true}
f x
  | x < 3     = y + 1
  | x > 3     = y - 1
  | otherwise = y
    where
      y = x * 2
```

<div class="mt-4 text-xs text-gray-500 leading-relaxed">
  Here, <code>y</code> is visible across all guards. The binding is evaluated once and shared.
</div>
</div>

<div>
Clunky with `let ... in` (repetitive):

```haskell {lines: true}
f x
    | x < 3     = let y = x * 2 in y + 1
    | x > 3     = let y = x * 2 in y - 1
    | otherwise = let y = x * 2 in y
```

<div class="mt-4 text-xs text-red-500 leading-relaxed">
  Since <code>let</code> is bound to the individual expression, the calculation must be repeated for each guard.
</div>
</div>

</div>

---
transition: slide-up
layout: two-cols
layoutClass: gap-8
---

# 8. GHC Core: Complex Example

Consider the following nested structure:

```haskell {lines: true}
f5 :: Int -> Int
f5 x 
  | x < 3     = y + y + 1
  | x > 3     = y - 1
  | otherwise = y
  where 
    y = z + z  
      where z = x * x
```

<div class="mt-4 text-xs">
  Interactive Explorer: <a href="https://godbolt.org/z/38qaWE8eq" target="_blank" class="text-blue-500 font-mono underline">godbolt.org/z/38qaWE8eq</a>
</div>

::right::

### 1. Raw GHC Core <span class="text-xs bg-red-100 text-red-700 px-2 py-0.5 rounded ml-2 font-mono font-bold">obfuscated</span>

```haskell
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

<v-click>

### 2. Simplified GHC Core <span class="text-xs bg-green-100 text-green-700 px-2 py-0.5 rounded ml-2 font-mono font-bold">readable</span>

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

</v-click>

<v-click>
  <div class="mt-4 text-xs text-blue-800 bg-blue-50 p-2 rounded border border-blue-150">
    <strong>Syntactic Sugar:</strong> Every <code>where</code> is internally transformed into a <code>let</code> construct.
  </div>
</v-click>

---
transition: slide-up
---

# 9. Decision Matrix: When to use which?

<div class="grid grid-cols-2 gap-6 mt-12">

<div class="p-6 bg-blue-50 border border-blue-200 rounded-lg shadow-sm">
  <div class="font-bold text-blue-700 text-base mb-4"> where clause:</div>
  <ul class="list-disc list-inside text-xs space-y-3 text-blue-900 leading-relaxed">
    <li>Is a <strong>syntactic declaration</strong> construct.</li>
    <li>Is read from <strong>top to bottom</strong> (result first).</li>
    <li>Is visible <strong>across all guards</strong> of an equation.</li>
    <li>Is best suited for <strong>structural clarity</strong> at the function level.</li>
  </ul>
</div>

<div class="p-6 bg-yellow-50 border border-yellow-250 rounded-lg shadow-sm">
  <div class="font-bold text-yellow-700 text-base mb-4"> let ... in expression:</div>
  <ul class="list-disc list-inside text-xs space-y-3 text-yellow-900 leading-relaxed">
    <li>Is a <strong>standalone expression</strong> (produces a value).</li>
    <li>Is read from <strong>bottom to top</strong> (derivation first).</li>
    <li>Is visible <strong>only within</strong> the specific branch.</li>
    <li>Best suited for <strong>small, local expressions</strong> within control flows (e.g. lambdas).</li>
  </ul>
</div>

</div>