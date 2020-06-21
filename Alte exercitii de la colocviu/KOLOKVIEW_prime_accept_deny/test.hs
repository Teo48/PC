data Pereche = P {
    firstValue :: Int,
    secondValue :: Int
}

getFirst pair@(P firstValue secondeValue) = firstValue pair