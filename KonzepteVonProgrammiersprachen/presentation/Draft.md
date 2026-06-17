# Outline 

## Introduction (1 min)
- Welcome to the presentation on bindings in Haskell.
- In functional programming, we often need to define helper functions.
- Haskell offers very elegant solutions for this. Today we’ll focus on the `where` clause.

## Show the example of local bindings in Haskell (Where) (1 min)
```haskell 
f x = y + 1
  where y = x * 2

main = print (f 3)
```

What is the output?
-> 7
(Explanation: `x` is 3, `y` is calculated as `3 * 2 = 6`, so `f 3` returns the result `6 + 1 = 7`.)

## Explain where (1 min)
- Syntax: The `where` clause is appended after the body of a function or equation.
- Top-down approach: It allows you to formulate the main logic of the function first and move the details (variables, helper functions) below it.
- Scope: The identifiers defined in `where` are only visible within the associated function/equation.
- Important: `where` is a syntactic construct for declarations, not a standalone expression.

## Showing the inline variant (1 min)
- As an alternative to `where`, there is the `let ... in ...` expression.
- The same example with `let` would look like this:
```haskell
f x = let y = x * 2
      in y + 1
```
- Here, we first define (`let`) and then use (`in`).
- Difference: `let` is an expression and can appear anywhere a value is expected.

## Advantages of `where` (1 min)
- High readability: The main result is at the very top. When someone reads the function, they immediately see the “what” and can look at the “how” in the `where` section if needed.
- Sharing via guards: This is the superpower of `where`. A variable defined in the `where` block can be used across multiple pattern-matching guards (`|`) without duplicating the code.

## Disadvantages of `where` (1 min)
- Not an expression: Since `where` is not an expression, you cannot use it spontaneously “inline” in the middle of another calculation or in a lambda expression.
- Lack of clarity in very long functions: If the `where` clause is at the very end of a long function, the definition is spatially far removed from its usage.

## When should `where` be used instead of `let`? (2 min)
- With multiple guards: As soon as multiple conditions require the same local variable, `where` is the idiomatic and clean choice.
- For complex helper functions: Idiomatic Haskell often states: “Do this, *where* helper function A does this...”. This promotes structured, understandable code.
- Rule of thumb: Use `where` for structural definitions and top-down readability at the function level. Use `let` for small, temporary expressions directly “inline”.

## Conclusion (1 min)
- Local bindings structure the code and avoid code duplication.
- `where` and `let` are both powerful tools that complement each other perfectly.
- The choice depends on whether you want to focus on the result (-> `where`) or on the step-by-step derivation (-> `let`). However, `where` really shines, especially with guards.