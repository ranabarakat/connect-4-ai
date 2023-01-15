# connect-4-ai
Connect four is a strategy board game where the player who aligns four disks wins. However, there is a slight modification in our implementation.

*The two players keep playing until the board is full.  The winner is the player having greater number of connected-fours.*

The program has three main components:
1. Board representation
2. Search Algorithm
3. Heuristic function

## Prerequisites

  *MINGW:*
  
  Mingw is the C++ compiler used for this project. To compile this
  project with the given libraries, you will need [mingw](https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/7.3.0/threads-posix/seh/x86_64-7.3.0-release-posix-seh-rt_v5-rev0.7z/download) version 7.3.0.

  *SFML:*
  
  SFML is the GUI library used for this application. You need to
  download version 2.5.1 from this [link](https://www.sfml-dev.org/download/sfml/2.5.1/).
  
 *Make:*
 
  Since compiling the library files is a long command, we have provided
  a makefile with the project to allow for easy builds. Make sure you
  have make installed on your machine especially if you are running
  windows then edit the library path in the makefile here:
  
  ![makefile-sfml-edit-path](https://user-images.githubusercontent.com/76884362/212539759-b83c8d12-ec31-4f0a-8942-13ddcbcfe1e1.png)
  
  Then just run make and you will have an executable "bitboard.exe".

## Board representation

  Each player gets a 64-bit integer to
  represent his pieces. We only need 42 bits to show the full 6x7 board, but
  we made use of an additional row to check whether a column is full or not, for a total of 49 bits
  utilized per player. Bit 0 represents the bottom left corner, bit 1 represents
  the one directly above it and so on moving from bottom left to top right.

## The search algorithm

  **Mini-max algorithm** is a recursive or backtracking algorithm which is used in decision-making and game theory. It provides an optimal move for the player assuming that opponent is also playing optimally. 
The algorithm includes a maximizer function and a minimizer function that do similar working with a difference in comparison between elements. A master function first calls maximizer, providing it with the current state,
and the maximizing player. Then both functions pass the objects between
them until either a terminal state or the maximum depth is reached. The best score iterates back up the tree all the way back to the master
function that can now play the column that leads to its “best state”. 

  **Alpha-beta pruning** is a search algorithm that seeks to decrease the number of nodes that are evaluated by the minimax algorithm in its search tree.
It stops evaluating a move when at least one possibility has been found that proves the move to be worse than a previously examined move. Such moves need not be evaluated further. When applied to a standard minimax tree, it returns the same move as minimax would, but prunes away branches that cannot possibly influence the final decision.
https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning

## The heuristic function
 
  The heuristic function quantifies how desired or undesired a state of the board is for the agent. The search algorithm helps the agent decided on the next move by efficiently exploring what might be the consequences of each move.
    
  Our heuristics where derived from a [research paper](https://www.researchgate.net/publication/331552609_Research_on_Different_Heuristics_for_Minimax_Algorithm_Insight_from_Connect-4_Game) that looked at
different options, but because our version doesn’t end when
any player connects 4 in a row, we had to make some modifications.

  **H1:**

  The first heuristic focuses on rewarding different parts of the board
  different scores favoring the spots that could potentially connect to more fours. This 
  encourages the AI agent to play more towards the center, which increases
  the likelihood of connecting more fours. We implemented this heuristic
  by traversing over the board and adding the cost of each spot to its
  respective player and then subtracting the cost of the player from
  the cost of the AI to arrive at the final evaluation of a given board. The time
  complexity of this heuristic is O(n) where n = ROWS*COLS.

  **H2:**

  The second heuristic counts the number of connected 2s, 3s, and 4s
  for each player and adds that to the players’ respective score value
  then, like H1, subtracts the player’s score from the AI’s score to reach
  the evaluation of a given board. We implemented this heuristic by a
  series of bitwise operations on a 64-bit board representation.
  Counting the number of connected 4s, 3s, and 2s is usually an O(n)
  operation at best. But because the boards are represented in binary
  form, we were able to achieve O(1) complexity through bit
  manipulation. For demonstration, let’s look at the count fours
  algorithm: 
  
  ```python static int count_fours(uint64_t bitboard)
  {
      int count = 0;
      count += divide(bitboard & (bitboard >> 7) & (bitboard >> 14) & (bitboard >> 21));
      count += divide(bitboard & (bitboard >> 8) & (bitboard >> 16) & (bitboard >> 24));
      count += divide(bitboard & (bitboard >> 6) & (bitboard >> 12) & (bitboard >> 18));
      count += divide(bitboard & (bitboard >> 1) & (bitboard >> 2) & (bitboard >> 3));
      return count;
  } 
  ```
  To get the number of connected fours, we must shift rows, cols,
  positive diagonals, and negative diagonals to the right by four times.
  Doing this in a board with random pieces that aren’t connected 4s
  will always result in 0. Otherwise, it will result in a binary number
  with several 1s equal to the number of connected 4s. From there, we
  can simply count the number of binary 1s in our result to get the
  number of connected 4s.
    
  We can do this with the following 2 functions:
    
  ```python 
  int BitCount(unsigned int u)
  {
      unsigned int uCount;
      uCount = u - ((u >> 1) & 033333333333) - ((u >> 2) & 011111111111);
      return ((uCount + (uCount >> 3)) & 030707070707) % 63;
  }

  int divide(uint64_t bitboard)
  {
      return BitCount(bitboard & 0xFFFFFFFF) + BitCount(bitboard >> 32);
  }
  ```

  The [bit count](https://stackoverflow.com/questions/109023/count-the-number-of-set-bits-in-a-32-bit-integer) function takes a 32-bit integer and
  counts the number of binary 1s. It does this by utilizing the
  properties of any binary number. A binary number n can be
  represented as n = a31 * 2^31 + a30 * 2^30 +.....+ ak * 2^k +....+ a1* 2 + a0. 
  By finding the right number we can run a series of shifts
  to get the number of 1s without having to iterate linearly over the
  entire number. Since we have a 64-bit representation, we used bitCount once on one half of the board, then shifted the result by
  32 bits to the right and repeated the process on the other half, and finally added the results. The divide function returns the final result.

  **H1 vs H2:**

  Because H1 is static and doesn’t consider the number of
  connected pieces for either player, it was easily beatable by a human
  and could also be beaten by another AI. H2 on the other hand, was
  much more dynamic and could be tuned to play offensively,
  defensively, or balanced. This proved to be a very important factor in
  choosing our heuristic. However, H2 had a massive flaw. In the first few turns, H2 wouldn’t get a chance to reach a position where
  there are any connected 3s or 4s on the board for either player. So, it
  wouldn’t naturally favor playing in the center as it would
  have the same cost as any connected 2s in the corner. To overcome
  this, we implemented a heuristic that combines both H1 and H2. H1 is used
  for evaluation of boards with a number of turns played less than a tunable
  arbitrary number, while H2 is used for higher turns.
  
 ## Analysis
  *Comparing time and nodes visited per move, at depth k.*
  ![connect4-analysis](https://user-images.githubusercontent.com/76884362/212540030-d0901dce-e97d-4e77-a99f-76f630bb3338.png)

## Sample Run

  ![1](https://user-images.githubusercontent.com/76884362/212540531-6ef11984-fba9-4cad-8367-4be38ade2dc8.png)
  ![2](https://user-images.githubusercontent.com/76884362/212540537-2e1c1c08-032f-4e9e-a0fc-31cabf31a0c4.png)
  ![3](https://user-images.githubusercontent.com/76884362/212540542-8b47374f-7cac-4402-a9e4-f9a952da3dce.png)
  ![4](https://user-images.githubusercontent.com/76884362/212540546-b1c8316a-b4f1-483d-8e6d-32f3e430641f.png)
  ![5](https://user-images.githubusercontent.com/76884362/212540552-7dcf34d0-1319-4c7b-9921-e57642a3604a.png)
