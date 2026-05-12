main2 :: IO ()
main2 = print $ show $ take 3 $ quicksort list
-- main = print $ show $ head ones

main = print $ show $ myHead $ myTail [1]

-- list =  [4,3,1,4,6,0]
-- list = [1,3,2,4,5,4,10]
list = [1..1000000]
 
ones = [1..]
 
quicksort :: Ord a =>  [a] -> [a] 
quicksort [] = []
quicksort (p:xs) = (quicksort lesser) ++ [p] ++ (quicksort greater)
    where
        lesser = [ x | x <- xs, x < p ]
                  -- filter (< p) xs
        greater = filter (>= p) xs
        
{-
Let's say we only need to two smallest numbers.

take 2 $ quicksort [3,2,5,1,4]

= take 2 $ (quicksort [2,1])  + [3] + (quicksort [5,4])

= take 2 $ [1,2] + [3] + (quicksort [5,4])

= [1,2]

-}


{-
Please never apply myHead on an empty list.
-}
myHead :: [a] -> a
myHead [] = error "empty list"
myHead (x:xs) = x

myTail :: [a] -> [a]
myTail [] = error "empty list"
myTail (x:xs) = xs 

{-

Types specify values.

Bool = { True, False }

Int = { -n, ..., 0, ... n }

Int_0 = { -n,...,-1,1,...,n }

Int_0 is a form of dependent type, cause it depends on a value.

[Int] = {[], [1], [2], ...., [1,2], .... }

[Int]_len where len > 0 means all lists are greater than len.



-}

