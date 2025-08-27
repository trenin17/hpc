#include <stdlib.h>
#include <iostream>
#include "search.h"
#include "board.h"
#include "eval.h"
#include <omp.h>

class MinimaxStrategy : public SearchStrategy
{
public:
    // Defines the name of the strategy
    MinimaxStrategy() : SearchStrategy("Minimax") {}

    // Factory method: just return a new instance of this class
    SearchStrategy *clone() { return new MinimaxStrategy(); }

private:
    /**
     * Implementation of the strategy.
     */
    void searchBestMove();
    int runMinimax(int depth, Board &board);

    omp_lock_t depthLocks[15]; // Locks for each depth level
};

void MinimaxStrategy::searchBestMove()
{
    for (int i = 0; i < 15; ++i)
    {
        omp_init_lock(&depthLocks[i]);
    }
    int bestEvaluation = minEvaluation();
    int evaluation;
#pragma omp parallel
    {
// Start the minimax search with depth 0
#pragma omp single
        {
            evaluation = runMinimax(0, *_board);
        }
    }

    for (int i = 0; i < 15; ++i) {
        omp_destroy_lock(&depthLocks[i]);
    }
}

int MinimaxStrategy::runMinimax(int depth, Board &board)
{
    if (!board.isValid())
    {
        return -15000 + depth;
    }
    // When max depth is reached, return evaluation
    if (depth >= _maxDepth)
    {
        return -evaluateBoard(board);
    }

    int max = -15000;
    Move move;
    MoveList moveList;

    board.generateMoves(moveList);

    while (moveList.getNext(move))
    {
        if(depth +2 < _maxDepth){
#pragma omp task firstprivate(move) shared(max)
        {

            int evaluation;
            auto boardCopy = board; // Copy the board state
            boardCopy.playMove(move);

            // max(a, b) = -min(-a, -b) 
            evaluation = -runMinimax(depth + 1, boardCopy);

            // Reset the board
            boardCopy.takeBack();

            {
                omp_set_lock(&depthLocks[depth]);
                if (evaluation > max)
                {
                    max = evaluation;

                        foundBestMove(depth, move, evaluation);
                }
                omp_unset_lock(&depthLocks[depth]);
            }
        }
    } else {
        int evaluation;
        board.playMove(move);

        // max(a, b) = -min(-a, -b)
        evaluation = -runMinimax(depth + 1, board);

        // Reset the board
        board.takeBack();

        if (evaluation > max)
        {
            max = evaluation;
            foundBestMove(depth, move, evaluation);
        }
    }
    }
#pragma omp taskwait

    finishedNode(depth, 0);
    return max;
}

// register ourselves as a search strategy
MinimaxStrategy minimaxStrategy;