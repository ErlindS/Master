
main :: IO ()
main = do putStrLn "hello"
          processRes (divSafe 2 0)
          -- putStrLn $ show (myDiv2 2 0)

myDiv :: Int -> Int -> Int
myDiv x y =  ((+) x 3) `div` (y + 1)

myDiv2 :: Int -> Int -> Int
myDiv2 x y 
  | y == 0 = error "panic"
  | otherwise = x `div` y
  
data Pos = MkPos (Int, Int)  
  
data Res = Succ Int | Fail String

divSafe :: Int -> Int -> Res
divSafe x y
  | y == 0 = Fail "division by zero"
  | otherwise = Succ (x `div` y)
  
divSafe2 :: Int -> Int -> Res
divSafe2 x y = case y of
     0 -> Fail "division by zero"
     _ -> Succ (x `div` y)  

processRes :: Res -> IO ()
processRes r = 
    case r of
     Succ i -> putStrLn $ show i
     Fail e -> putStrLn e
     
processRes2 :: Res -> IO ()
processRes2 (Succ i) = putStrLn $ show i
processRes2 (Fail e) = putStrLn e  


data R a = SuccR a  | FailR String

divSafe3 :: Int -> Int -> R Int
divSafe3 x y
  | y == 0 = FailR "division by zero"
  | otherwise = SuccR (x `div` y)
  
isZero 0 = True
isZero _ = False
