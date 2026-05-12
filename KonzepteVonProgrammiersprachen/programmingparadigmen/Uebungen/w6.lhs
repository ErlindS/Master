
{-# LANGUAGE GADTs #-}


data E = N Int | Plus E E | Mult E E
       | B Bool | LessThan E E | Or E E deriving (Show, Eq)

data E2 where 
   N2 :: Int -> E2
   Plus2 :: E2 -> E2 -> E2
   
   
   

-- short-hands
zero = N 0
one = N 1
true = B True
false = B False

-- Examples

ex = Plus (Mult one zero) one

ex1 = Mult zero (Plus zero one)

ex2 = Or (LessThan zero one) false

---------------------------------------
-- Simple top-down parser written by hand

-- Precedence rules.
-- * binds tighter than +
-- < binds tighter than |
-- Best to use parentheses.

data Token = NUM Int
           | BOOL Bool
           | OPEN | CLOSE
           | PLUS | MULT | LESSTHAN | OR deriving Show

type Tokens = [Token]

tokenize :: String -> Maybe [Token]
tokenize [] = Just []
tokenize (x:xs)
 | x == ' '  = tokenize xs
 | isDigit x = let (ds,ys) = collectDigits (x:xs)
               in case tokenize ys of
                    Just ts -> Just (NUM (read ds :: Int) : ts)
                    f       -> f
 | otherwise = case lookup x prims of
                Just t -> case tokenize xs of
                           Just ts -> Just (t:ts)
                           f       -> f
                Nothing -> Nothing


collectDigits xs = (takeWhile isDigit xs, dropWhile isDigit xs)
isDigit x = elem x ['0'..'9']
prims = [('(',OPEN), (')',CLOSE),
         ('+',PLUS), ('*',MULT),
         ('<', LESSTHAN), ('|', OR),
         ('t', BOOL True), ('f', BOOL False)]


parse :: String -> Maybe E
parse xs = do
  ys <- tokenize xs
  (zs,e) <- parseE ys
  case zs of
    [] -> return e

parseE :: Tokens -> Maybe (Tokens, E)
parseE xs = do (ys,e1) <- parseT xs
               parseE2 ys e1

parseT :: Tokens -> Maybe (Tokens, E)
parseT xs = do (ys,e) <- parseF xs
               parseT2 ys e

parseE2 :: Tokens -> E -> Maybe (Tokens, E)
parseE2 [] e1 = return ([],e1)
parseE2 (PLUS:xs) e1 = do (ys,e2) <- parseT xs
                          parseE2 ys (Plus e1 e2)
parseE2 (OR:xs) e1 = do (ys,e2) <- parseT xs
                        parseE2 ys (Or e1 e2)
parseE2 xs e          = return (xs,e)

parseT2 :: Tokens -> E -> Maybe (Tokens, E)
parseT2 [] e1 = return ([],e1)
parseT2 (MULT:xs) e1 = do (ys, e2) <- parseF xs
                          parseT2 ys (Mult e1 e2)
parseT2 (LESSTHAN:xs) e1 = do (ys, e2) <- parseF xs
                              parseT2 ys (LessThan e1 e2)
parseT2 xs e          =  return (xs,e)

parseF :: Tokens -> Maybe (Tokens, E)
parseF [] = fail "Syntax error"
parseF (x:xs) =
  case x of
   BOOL b -> return (xs, B b)
   NUM i -> return (xs, N i)
   OPEN -> do (ys,e) <- parseE xs
              case ys of
                (CLOSE:zs) -> return (zs,e)
                _          -> fail "Syntax error"


-----------------------
-- Interpreter

{-

Conventions:

We write i to refer to integer values and b to refer boolean values.

(O1)  (N i) => i

(O2)  (B b) => b

      e1 => i1
      e2 => i2
      i=i1+i2
(O3)  ------------------
      (Plus e1 e2) => i

      e1 => i1
      e2 => i2
      i=i1*i2
(O4)  ------------------
      (Mult e1 e2) => i

      e1 => i1
      e2 => i2
      i1 < i2
(O5)  ------------------
      (LessThan e1 e2) => True

      e1 => i1
      e2 => i2
      i1 >= i2
(O6)  ------------------
      (LessThan e1 e2) => False

      e1 => True
(O7)  ------------------
      (Or e1 e2) => True

      e1 => False
      e2 => True
(O8)  ------------------
      (Or e1 e2) => True

      e1 => False
      e2 => False
(O9)  ------------------
      (Or e1 e2) => False

-}


eval :: E -> Maybe (Either Int Bool)
eval (N i) = Just (Left i)                                               -- (O1)
eval (B b) = Just (Right b)                                              -- (O2)
eval (Plus e1 e2) =                                                      -- (O3)
     let r1 = eval e1
         r2 = eval e2
     in case (r1, r2) of
         (Just (Left i1), Just (Left i2)) -> Just (Left (i1 + i2))
         (_,_)  -> Nothing
eval (Mult e1 e2) =                                                      -- (O4)
     let r1 = eval e1
         r2 = eval e2
     in case (r1, r2) of
         (Just (Left i1), Just (Left i2)) -> Just (Left (i1 * i2))
         (_,_)  -> Nothing
eval (LessThan e1 e2) =                                                  -- (O5-6)
     let r1 = eval e1
         r2 = eval e2
     in case (r1, r2) of
         (Just (Left i1), Just (Left i2)) -> if i1 < i2
                                             then Just (Right True)
                                             else Just (Right False)
         (_,_)  -> Nothing
eval (Or e1 e2) =                                                        -- (O7-9)
     case (eval e1) of
        Nothing -> Nothing
        (Just Left{}) -> Nothing
        (Just (Right True)) -> Just (Right True)
        _                   -> case (eval e2) of
                                 Nothing -> Nothing
                                 (Just Left{}) -> Nothing
                                 (Just (Right True)) -> Just (Right True)
                                 _                   -> Just (Right False)

-- Examples
-- f | t
ex3 = Or false true

-- f | 1
ex4 = Or false one

-- t | 1
ex5 = Or true one



runEval = do putStrLn "Key in some expression:"
             s <- getLine
             case parse s of
               Nothing -> putStrLn "Invalid syntax"
               Just e -> case eval e of
                           Nothing -> putStrLn "Evaluation failure"
                           Just (Left i) -> putStrLn ("Integer result = " ++ show i)
                           Just (Right b) -> putStrLn ("Boolean result = " ++ show b)


parseAndEval s = case parse s of
                    Nothing -> putStrLn "Invalid syntax"
                    Just e -> case eval e of
                               Nothing ->  putStrLn "Evaluation failure"
                               Just (Left i) -> putStrLn ("Integer result = " ++ show i)
                               Just (Right b) -> putStrLn ("Boolean result = " ++ show b)

{-

Examples.

parseAndEval  "(1+2) * 3"
Integer result = 9

parseAndEval  "(1+3) * t"
Evaluation failure

parseAndEval  "(1<2) | 3"
Boolean result = True


-}


-----------------------
-- Simplifyer
-- We apply the algebraic law 0 * x = x

simp :: E -> E
simp e@N{} = e
simp e@B{} = e
simp (Plus e1 e2) = Plus (simp e1) (simp e2)
simp (Mult (N 0) _) = N 0
simp (Mult e1 e2) = Mult (simp e1) (simp e2)
simp (LessThan e1 e2) = LessThan (simp e1) (simp e2)
simp (Or e1 e2) = Or (simp e1) (simp e2)


-- Examples

-- (0 * 1) * 1
s1 = Mult (Mult zero one) one

{-

 eval s1 => Just (Left 0)

 simp s1 => Mult (N 0) (N 1)

 simp (simp s1) => N 0

 eval (simp s1) => Just (Left 0)

-}


-- 0 * (0 * False)
s2 = Mult zero (Or zero false)

{-

 eval s2 => Nothing

 simp s2 => N 0

 eval (simp s2) => Just (Left 0)

-}


{-

Observations:

We have to exhaustively apply simp until no further simplifications are possible.
See example s1.
We apply a fixpoint construction to exhaustively apply simplifications.


Evaluation of the original and simplified expression ought to yield the same result.
This is not necessarily the case. See example s2.
We implement a type checker to rule out ill-typed expressions.

-}

------------------------------------
-- Exhaustive application of simp

simpFix :: E -> E
simpFix e = let e2 = simp e
            in if e2 == e then e
               else simpFix e2


{-

simp s1 => Mult (N 0) (N 1)

simpFix s1 => N 0

-}


-----------------------
-- Type checker


{-

Conventions:

We write

E |- T

to denote that expression E has type T where T can be either Int or Bool.


(T1)  (N i) |- Int

(T2)  (B b) |- Bool

      e1 |- Int
      e2 |- Int
(T3)  ------------------
      (Plus e1 e2) |- Int

      e1 |- Int
      e2 |- Int
(T4)  ------------------
      (Mult e1 e2) |- Int

      e1 |- Int
      e2 |- Int
(T5)  ------------------
      (LessThan e1 e2) |- Bool

      e1 |- Bool
      e2 |- Bool
(T6)  ------------------
      (Or e1 e2) |- Bool

-}

data Type = TInt | TBool deriving Show

typecheck :: E -> Maybe Type
typecheck (N _) = Just TInt
typecheck (B _) = Just TBool
typecheck (Plus e1 e2) =
     case (typecheck e1, typecheck e2) of
       (Just TInt, Just TInt) -> Just TInt
       (_, _) -> Nothing
typecheck (Mult e1 e2) =
     case (typecheck e1, typecheck e2) of
       (Just TInt, Just TInt) -> Just TInt
       (_, _) -> Nothing
typecheck (LessThan e1 e2) =
     case (typecheck e1, typecheck e2) of
       (Just TInt, Just TInt) -> Just TBool
       (_, _) -> Nothing
typecheck (Or e1 e2) =
     case (typecheck e1, typecheck e2) of
       (Just TBool, Just TBool) -> Just TBool
       (_, _) -> Nothing



{-

typecheck s1 => Just TInt

typecheck s2 => Nothing

-}

runTypecheck =
  do putStrLn "Key in some expression:"
     s <- getLine
     case parse s of
       Nothing -> putStrLn "Invalid syntax"
       Just e -> case typecheck e of
                   Nothing ->  putStrLn "Type checking failure"
                   Just TInt -> putStrLn "Expression has type Int"
                   Just TBool -> putStrLn "Expression has type Bool"


parseTypeCheckEval s =
  case parse s of
    Nothing -> putStrLn "Invalid syntax"
    Just e -> case typecheck e of
                Nothing -> putStrLn "Type checking failure"
                Just _ ->  case eval e of
                               Nothing ->  putStrLn "Evaluation failure" -- Deadcode!
                               Just (Left i) -> putStrLn ("Integer result = " ++ show i)
                               Just (Right b) -> putStrLn ("Boolean result = " ++ show b)


runTypeCheckEval =
  do putStrLn "Key in some expression:"
     s <- getLine
     parseTypeCheckEval s

parseTypeCheckEvalM s = do
             -- Maybe monad
   let res = do e <- parse s
                _ <- typecheck e
                v <- eval e
                return v
   case res of
                -- IO monad, can't mix Maybe and IO!
     Nothing -> putStrLn "failure"
     Just (Left i) -> putStrLn ("Integer result = " ++ show i)
     Just (Right b) -> putStrLn ("Boolean result = " ++ show b)



-----------------------------------------
-- Evaluation of well-typed expressions


evalT :: E -> Either Int Bool
evalT (N i) = Left i
evalT (B b) = Right b
evalT (Plus e1 e2) =
    case (evalT e1, evalT e2) of
       (Left i1, Left i2) -> Left (i1 + i2)
evalT (Mult e1 e2) =
    case (evalT e1, evalT e2) of
       (Left i1, Left i2) -> Left (i1 * i2)
evalT (LessThan e1 e2) =
    case (evalT e1, evalT e2) of
       (Left i1, Left i2) -> Right (i1 < i2)
evalT (Or e1 e2) =
     case (evalT e1) of
       Right True -> Right True
       _          -> evalT e2


---------------------------------
-- More general data types

-- With generalized algebraic data types (GADTs),
-- we can encode the typing rules in the construction of values.
-- So by construction, we can only ever generate well-typed expressions!


data EE a where
  N_EE :: Int -> EE Int
  B_EE :: Bool -> EE Bool
  Plus_EE :: EE Int -> EE Int -> EE Int
  Mult_EE :: EE Int -> EE Int -> EE Int
  LessThan_EE :: EE Int -> EE Int -> EE Bool
  Or_EE :: EE Bool -> EE Bool -> EE Bool


evalEE :: EE a -> a
evalEE (N_EE i) = i
evalEE (B_EE b) = b
evalEE (Plus_EE e1 e2) = evalEE e1 + evalEE e2
evalEE (Mult_EE e1 e2) = evalEE e1 * evalEE e2
evalEE (LessThan_EE e1 e2)
  | evalEE e1 < evalEE e2 = True
  | otherwise             = False
evalEE (Or_EE e1 e2) = evalEE e1 || evalEE e2


----------------------
-- main functions

main = do putStrLn "Some of our own examples"
          let e = LessThan (B True) (N 0)
          let e2 = Or (B True) (N 0)
          -- putStrLn $ show $ eval e 
          putStrLn $ show $ eval e
          putStrLn $ show $ typecheck e2
          firstTypeCheckThenEval e2

firstTypeCheckThenEval e = 
     case typecheck e of
       Nothing -> putStrLn "Type error"
       _       -> putStrLn ("The result is = " ++ (show $ evalT e))

main4 = runEval
main2 = runTypecheck
main3 = runTypeCheckEval
