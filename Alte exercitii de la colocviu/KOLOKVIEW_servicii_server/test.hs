import Data.List;
data T = T Int deriving (Eq, Show)
f (T x) = sum [ 1 | a <- [1 .. x], mod x a < 1 ]
instance Ord T where
    x <= y = f x <= f y

g x y z = filter z . y $ x