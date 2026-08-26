module Renderer

import Data.List
import Data.String
import System
import System.File

%default total

Key : Type
Key = List String

Rows : Type
Rows = List (List Key)

record Board where
  constructor MkBoard
  fileStem : String
  keyRows : Rows

one : String -> Key
one a = [a]

two : String -> String -> Key
two a b = [a, b]

three : String -> String -> String -> Key
three a b c = [a, b, c]

movement : Board
movement = MkBoard "cursor and page movement"
  [ [ one "MOVEMENT", one "TYPE", two "PREVIOUS" "LINE", two "NEXT" "LINE" ]
  , [ one "LONG LEFT", one "LONG RIGHT", two "START" "LINE", two "END" "LINE" ]
  , [ two "NEXT" "WORD", three "NEXT" "SPACE-SEPARATED" "WORD", two "MATCH" "DELIMITER" ]
  , [ three "ONE THIRD" "PAGE" "UP", three "ONE THIRD" "PAGE" "DOWN"
    , two "7" "TIMES", two "13" "TIMES", two "17" "TIMES", two "29" "TIMES" ]
  , [ two "REPEAT" "FORWARD", two "REPEAT" "BACKWARD"
    , two "DELETE" "CHARACTER", two "DELETE" "WORD", one "UNDO" ]
  ]

signals : Board
signals = MkBoard "signals (quit program, sleep program, shutdown)"
  [ [ two "QUIT" "PROGRAM", two "SLEEP" "PROGRAM", two "WAKE" "PROGRAM" ]
  , [ one "SHUTDOWN", three "QUIT" "AND" "DUMP", two "KILL" "PROGRAM" ]
  ]

mathKeys : Board
mathKeys = MkBoard "math ÷ ≤ × = –"
  [ [ one "ADD", one "SUBTRACT", one "MULTIPLY", one "DIVIDE" ]
  , [ one "EQUAL", two "NOT" "EQUAL", three "LESS" "OR" "EQUAL", three "GREATER" "OR" "EQUAL" ]
  ]

programming : Board
programming = MkBoard "classic programming symbols ([{ lambda dereference_pointer address_of comment assignment heredoc enclose_a_single_idea function_composition"
  [ [ two "LEFT" "DELIMITERS", two "PAIRED" "DELIMITER", one "LAMBDA"
    , one "FUNCTION", one "DEFINITION", two "⟦ ⟧" "EVALUATE", two "RIGHT" "DELIMITERS" ]
  , [ two "⟵" "ASSIGN", two "ASSIGN" "RIGHT", one "COMPOSE", one "ADDRESS"
    , one "DEREFERENCE", two "POINTER" "TYPE", one "SPLAT" ]
  , [ two "PERL" "SCALAR", two "PERL" "ARRAY", two "PERL" "HASH", one "DOT"
    , one "COLON", one "SEMICOLON", two "SINGLE" "QUOTE", one "BACKTICK", two "DOUBLE" "QUOTE" ]
  , [ one "SLASH", one "BACKSLASH", one "PIPE", one "COMMENT"
    , two "PAIRED" "QUOTES", two "ANGLE" "QUOTES" ]
  , [ one "FLOAT", one "DOUBLE", one "CHAR", one "STRING", one "DO", one "LOOP", two "START" "TEXT" ]
  ]

regex : Board
regex = MkBoard "regular expressions start end all any"
  [ [ three "START" "OF" "LINE", three "END" "OF" "LINE", three "END" "OF" "SLURP", two "MAYBE" "OK IF NOT?" ]
  , [ two "BIG CAPTURE" "GREEDY", two "SMALL CAPTURE" "NON GREEDY"
    , three "EXACT ORDER" "GROUP" "()", three "ANY FROM" "LIST" "[]" ]
  , [ one "LETTER", one "NUMBER", two "NON WEIRD" "CHARACTER", two "WEIRD" "CHARACTER" ]
  , [ two "INCANTATION RUNES" "CONTROL CHARACTERS" ]
  ]

separation : Board
separation = MkBoard "concept separation _"
  [ [ two "QUAD" "SPACE", two "FOUR" "SPACES", two "DOUBLE" "SPACE", two "SINGLE" "SPACE" ]
  , [ one "INDENT", one "UNDERSCORE" ]
  ]

incantations : Board
incantations = MkBoard "incantation assistance (search incantation history, complete the incantation I've started typing)"
  [ [ two "COMPLETE" "INCANTATION", two "FINISH" "INCANTATION", two "CHANT" "HISTORY" ] ]

pastebins : Board
pastebins = MkBoard "several pastebins push pop see"
  [ [ two "VIEW" "1", two "VIEW" "2", two "VIEW" "3" ]
  , [ two "REMEMBER" "1", two "REMEMBER" "2", two "REMEMBER" "3" ]
  , [ two "WRITE" "1", two "WRITE" "2", two "WRITE" "3" ]
  ]

boards : List Board
boards = [movement, signals, mathKeys, programming, regex, separation, incantations, pastebins]

margin : Nat
margin = 20

titleHeight : Nat
titleHeight = 62

keyWidth : Nat
keyWidth = 190

keyHeight : Nat
keyHeight = 76

gap : Nat
gap = 10

cellWidth : Nat
cellWidth = keyWidth + gap

cellHeight : Nat
cellHeight = keyHeight + gap

halfKeyWidth : Nat
halfKeyWidth = 95

listLength : List a -> Nat
listLength [] = 0
listLength (_ :: rest) = S (listLength rest)

maximumNat : Nat -> Nat -> Nat
maximumNat left right = if left > right then left else right

maximumColumns : Rows -> Nat
maximumColumns [] = 0
maximumColumns (row :: rest) = maximumNat (listLength row) (maximumColumns rest)

spanSize : Nat -> Nat -> Nat -> Nat
spanSize 0 _ _ = 0
spanSize count@(S previous) itemSize gapSize = count * itemSize + previous * gapSize

contentWidth : Board -> Nat
contentWidth board = margin * 2 + spanSize (maximumColumns (keyRows board)) keyWidth gap

titleWidth : Board -> Nat
titleWidth board = margin * 2 + listLength (unpack (fileStem board)) * 13

boardWidth : Board -> Nat
boardWidth board = maximumNat (contentWidth board) (titleWidth board)

boardHeight : Board -> Nat
boardHeight board = titleHeight + margin * 2 + spanSize (listLength (keyRows board)) keyHeight gap

escapeChar : Char -> String
escapeChar '&' = "&amp;"
escapeChar '<' = "&lt;"
escapeChar '>' = "&gt;"
escapeChar '"' = "&quot;"
escapeChar character = singleton character

escapeXml : String -> String
escapeXml value = fastConcat (map escapeChar (unpack value))

textElement : Nat -> Nat -> Nat -> String -> String
textElement x y size value =
  "<text x=\"" ++ show x ++ "\" y=\"" ++ show y ++
  "\" text-anchor=\"middle\" font-family=\"DejaVu Sans, sans-serif\"" ++
  " font-size=\"" ++ show size ++ "\" fill=\"#111\">" ++
  escapeXml value ++ "</text>\n"

titleElement : Nat -> Nat -> Nat -> String -> String
titleElement x y size value =
  "<text x=\"" ++ show x ++ "\" y=\"" ++ show y ++
  "\" text-anchor=\"start\" font-family=\"DejaVu Sans, sans-serif\"" ++
  " font-size=\"" ++ show size ++ "\" fill=\"#111\">" ++
  escapeXml value ++ "</text>\n"

renderLines : Nat -> Nat -> List String -> String
renderLines _ _ [] = ""
renderLines centreX y (line :: rest) =
  textElement centreX y 16 line ++ renderLines centreX (y + 20) rest

renderKey : Nat -> Nat -> Key -> String
renderKey rowNumber column key =
  let x = margin + column * cellWidth
      y = titleHeight + margin + rowNumber * cellHeight
      centreX = x + halfKeyWidth in
    "<rect x=\"" ++ show x ++ "\" y=\"" ++ show y ++
    "\" width=\"" ++ show keyWidth ++ "\" height=\"" ++ show keyHeight ++
    "\" rx=\"9\" fill=\"white\" stroke=\"#111\" stroke-width=\"2\"/>\n" ++
    renderLines centreX (y + 26) key

renderKeys : Nat -> Nat -> List Key -> String
renderKeys _ _ [] = ""
renderKeys rowNumber column (key :: rest) =
  renderKey rowNumber column key ++ renderKeys rowNumber (S column) rest

renderRows : Nat -> Rows -> String
renderRows _ [] = ""
renderRows rowNumber (row :: rest) =
  renderKeys rowNumber 0 row ++ renderRows (S rowNumber) rest

renderSvg : Board -> String
renderSvg board =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" ++
  "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" ++ show (boardWidth board) ++
  "\" height=\"" ++ show (boardHeight board) ++ "\" viewBox=\"0 0 " ++
  show (boardWidth board) ++ " " ++ show (boardHeight board) ++ "\">\n" ++
  "<rect width=\"100%\" height=\"100%\" fill=\"#f7f7f4\"/>\n" ++
  titleElement margin 38 24 (fileStem board) ++
  renderRows 0 (keyRows board) ++
  "</svg>\n"

outputDirectory : String
outputDirectory = "pictures of the keypads"

covering
writeBoard : Board -> IO Bool
writeBoard board = do
  let path = outputDirectory ++ "/" ++ fileStem board ++ ".svg"
  result <- writeFile path (renderSvg board)
  case result of
    Left fileError => do
      putStrLn ("could not write " ++ path ++ ": " ++ show fileError)
      pure False
    Right () => do
      putStrLn ("wrote " ++ path)
      pure True

covering
writeBoards : List Board -> IO Bool
writeBoards [] = pure True
writeBoards (board :: rest) = do
  thisOne <- writeBoard board
  remaining <- writeBoards rest
  pure (thisOne && remaining)

covering
main : IO ()
main = do
  success <- writeBoards boards
  if success then pure () else exitFailure
