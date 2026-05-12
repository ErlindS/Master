main :: IO ()
main = do putStr "\nHello Class\n"

          let f1 = MkFunny 1
          let f2 = MkFunny 2
          putStrLn $ show f1
          putStrLn $ show $ f1 == f2

          let l1 = Cons "bb" (Cons "aa" Nil)
          putStrLn $ show $ member "bc" l1
          -- l2 has the type List (List String)
          let l2 = Cons (Cons "b" Nil) Nil
          
          putStr $ show $ member Nil l2
          
          -- Making type classes explicit
          putStr $ show $ memberI eqList Nil l2
          
          putStrLn "Done"

data List t = Nil | Cons t (List t) 


-- "Eq a" is a type constraint and tells the
-- compiler the equality will be provided for
-- the underlying values of type a.
--
-- The "=>" can be interpreted as logical implication.
eqList :: Eq a => List a -> List a -> Bool
eqList Nil Nil = True
eqList (Cons x xs) (Cons y ys) = 
     (x == y) && (eqList xs ys)
eqList _ _ = False
-- Below two cases are covered by the above case
{-
eqList Nil (Cons _ _) = False
eqList (Cons _ _) Nil = False
-}     

instance Eq a => Eq (List a) where
  (==) = eqList


member :: Eq a => a -> List a -> Bool
member x Nil = False
member x (Cons y ys)
    | x == y    = True
    | otherwise = member x ys
    
-- We replace "Eq a" by the explicit parameter
-- eq of type a -> a -> Bool
memberI :: (a -> a -> Bool) -> a -> List a -> Bool
memberI eq x Nil = False
memberI eq x (Cons y ys)
    | x `eq` y    = True
    | otherwise = memberI eq x ys
    
    
    
    
data Funny = MkFunny Int deriving (Eq, Show)

-- "deriving Eq" will automatically generate the code below
{-
instance Eq Funny where
   (==) (MkFunny x) (MkFunny y) = x == y
-}  

{-
instance Show Funny where
  show (MkFunny x) = "This is funny " ++ show x
-}

    
    