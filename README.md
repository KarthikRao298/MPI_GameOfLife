# Objective
#### Conway’s Game of Life
British mathematician John Horton Conway devised a cellular automaton, known by the name ‘Game of Life’, or more conventionally as ‘Life’. The evolution in the game proceeds based on just the initial state and requires no further input. More about this can be obtained from - https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life

Rules of the Game(algorithm)-
1. begin with an initial value of N. This value denotes your matrix size.
2. each cell in the matrix is either alive or dead, populated or unpopulated.
3. every cell in the matrix interacts with its immediately neighbors which vertically,
horizontally and diagonally adjacent to it.
4. At any given instant of time, the following rules apply
 - 4.1. Any live cell with fewer than two live neighbors dies, as if caused by underpopulation.
 - 4.2. Any live cell with two or three live neighbors lives on to the next generation.
 - 4.3. Any live cell with more than three live neighbors dies, as if by overpopulation.
 - 4.4. Any dead cell with exactly three live neighbors becomes a live cell, as if by reproduction.
-------
Question 1: Implement a distributed memory version of the game of life algorithm and perform computation on individual nodes. Run the game for 10 iterations. Choose initial game board as convenient (preferably using srand). Ensure the N value is large enough for distributed memory computing . Plot for N values in the order of 1000 and more.
Compute the total number of live cells at the end of each iteration and also output the same.

Question 2: Compute the time the program takes to run for cores P=2,4,8,16,32. Plot the speedup graphs for each and report the speedup observed.
