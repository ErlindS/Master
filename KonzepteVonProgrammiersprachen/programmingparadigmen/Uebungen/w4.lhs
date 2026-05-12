main :: IO ()
main = do putStrLn "We are now playing with lists"

data List a = Nil | Cons a (List a)


l1 = Nil
l1b = []

l2 = Cons 1 Nil
l2b = 1 : []
l2c = (:) 1 []


l3 = Cons 1 (Cons 2 Nil)
l3b = 1 : 2 : []
l3c = [1,2]

hd :: List a -> a
hd (Cons x _) = x 

tl :: List a -> List a
tl (Cons _ xs) = xs

-- Future:
-- len :: List a n -> Int n
len :: List a -> Int
len Nil = 0
len (Cons _ xs) = len xs + 1


mapL f Nil = Nil
mapL f (Cons x xs) = Cons (f x) (mapL f xs)

{-

class List<a> { ... }

class Nil derived List<a>  { ... }

class Cons<a> derived List<a> { ... }

-}


vs = [ 1, 2, 3]

vs2 = map (1+) vs

vs3 = map (\x -> x * x) vs2

vs4 = map (\x -> x * x) (map (1+) vs)

-- "map law"
-- map f . map g = map (f. g)

-- member applies to any list as long
-- as the elements are comparable
member :: Eq a => a -> List a -> Bool
member x Nil = False
member x (Cons y ys)
    | x == y    = True
    | otherwise = member x ys
    
    
data Funny = MkFunny Int

eqFunny :: Funny -> Funny -> Bool
eqFunny (MkFunny x) (MkFunny y) = x == y

-- Need to teach Haskell that "Eq Funny" holds.

instance Eq Funny where
  (==) = eqFunny


example = member (MkFunny 1) Nil  

