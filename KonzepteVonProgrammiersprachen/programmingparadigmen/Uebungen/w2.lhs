main :: IO ()
main = do putStr "Hello"
          putStrLn "Whatever"
          let x = 1
          putStr (show x)
          let x_new = x + 1
          putStr (show x_new)
          let y = 5.3
          putStr $ show y
          -- y has the inferred type Float
          -- show is an overloaded function in its argument type
          putStr $ show (comp x (x+x))
          putStr $ show $ comp2 3
          putStr $ show $ increment 3
          putStr $ show $ plus2 1 2
          putStr $ show $ add2 (1,2)
          putStr $ show $ inc2 2
          putStr $ show $ 1 `plus` 2
          t <- inc4 5
          let z = 1 + t
          putStr $ show z

-- In Haskell side effects are visible via types
inc4 :: Int -> IO Int
inc4 x = do putStr "Fire missile"
            return (x+1)

plus :: Int -> (Int -> Int)
plus x y = x + y

add :: (Int, Int) -> Int
add (x,y) = x + y

add2 :: (Int, Int) -> Int
add2 = \(x,y) -> (+) x  y

increment :: Int -> Int
increment = \x -> x + 1

inc2 = (+1)

plus2 :: Int -> (Int -> Int)
plus2 = \x -> (\y -> x + y)

inc = plus2 1

comp x y = x * 5 + y     

comp2 = comp 5