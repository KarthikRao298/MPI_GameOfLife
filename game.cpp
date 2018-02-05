/*
 * File Name : game.cpp
 * ver: 0.1
 * date : Nov 29 2017
 *
 * To compile :
 * Make game
 *
 * To compile :
 * mpicxx -std=c++11 game.cpp -o game
 * 
 * Sample command line execution :
 * 
 * mpirun -n 2 ./game  6
 * mpirun -n 3 ./game  11200
 *
 */

/* Debug prints will be enabled if set to 1 */
//#define DEBUG 0
#define NODE_0 0
#define GAME_MAT_SENDER_FIRST_ROW 0
#define GAME_MAT_SENDER_LAST_ROW 11

#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <string.h>
#include <cmath>

#include "CommonHeader.h"

/* function to initialise the matrix */
void InitGameMatrix (int *** gameSubMatCur, int gameSubMatRowSize, int gameSubMatColmSize, int indexValue);

/* function for the transition rules */
int ApplyTransitionRule (int totalNeighborValue, int pastCellValue);

/*==============================================================================
 *  main
 *=============================================================================*/

int main (int argc, char* argv[]) {


    if (argc < 2) {
        std::cerr<<"Usage: "<<argv[0]<<" <gameMatSize>"<<std::endl;

        return -1;
    }


    MPI_Init(NULL, NULL);

    /* measure time taken for integration */
    std::chrono::time_point<std::chrono::system_clock> StartTime;
    std::chrono::time_point<std::chrono::system_clock>  EndTime;
    std::chrono::duration<double> ElapsedTime;

    int countLiveCell = 0;
    int totalLiveCell = 0;
    int i,j;
    int gameMatSize  = atoi (argv[1]);

    int gameSubMatColmSize, gameSubMatRowSize;
    int firstRow, lastRow;
    int commSize;
    int procRank;
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);
    int firstNode = NODE_0;
    int lastNode = commSize - 1;

    MPI_Request SendReq[2];
    MPI_Status Status[2];

    int ** gameSubMatPast;
    int ** gameSubMatCur;
    gameSubMatColmSize = gameMatSize;
    gameSubMatRowSize = gameMatSize / commSize;


    InitGameMatrix (&gameSubMatPast, gameSubMatRowSize, gameSubMatColmSize, 1);
    InitGameMatrix (&gameSubMatCur, gameSubMatRowSize, gameSubMatColmSize, 0);

    MPI_Barrier( MPI_COMM_WORLD ) ;
    if (procRank == NODE_0){
        StartTime = std::chrono::system_clock::now();
    }
    MPI_Barrier( MPI_COMM_WORLD ) ;


    firstRow = 0 ;
    lastRow  = gameSubMatRowSize - 1;

    DLOG (C_VERBOSE, "Node[%d] firstRow  = %d. lastRow  = %d\n",procRank, firstRow, lastRow);
    DLOG (C_VERBOSE, "Node[%d] gameSubMatRowSize  = %d. gameSubMatColmSize  = %d\n",procRank, gameSubMatRowSize, gameSubMatColmSize);
    DLOG (C_VERBOSE, "Node[%d] Creating firstRow subarray datatype\n",procRank);
    /* create a datatype to describe the first row of the subarray */

    int sizes[2]    = {gameSubMatRowSize, gameSubMatColmSize};
    int subsizes[2] = {1, gameSubMatColmSize};
    int starts[2]   = {0,0};
    MPI_Datatype firstRowSubArray;
    MPI_Type_create_subarray(2, sizes, subsizes, starts, MPI_ORDER_C, MPI_INT, &firstRowSubArray);
    MPI_Type_commit(&firstRowSubArray);

    /* end of first row subarray data type creation */


    DLOG (C_VERBOSE, "Node[%d] Creating lastRow subarray datatype\n",procRank);
    /* create a datatype to describe the last row of the subarray */

    int sizes_1[2]    = {gameSubMatRowSize, gameSubMatColmSize};
    int subsizes_1[2] = {1, gameSubMatColmSize};
    int starts_1[2]   = {0,0};
    MPI_Datatype lastRowSubArray;
    MPI_Type_create_subarray(2, sizes_1, subsizes_1, starts_1, MPI_ORDER_C, MPI_INT, &lastRowSubArray);
    MPI_Type_commit(&lastRowSubArray);

    /* end of first row subarray data type creation */

    /* allocate memory for the received rows */
    int * receivedRow = new int [gameSubMatColmSize];

    for (int k = 0; k < 10 ; k++) {

        if (procRank == firstNode) {
            /*To do :  create the last row of sub matrix */
            DLOG (C_VERBOSE, "Node[%d] procRank == firstNode so, sending lastRow\n",procRank);

            MPI_Isend (&gameSubMatPast[lastRow][0], 1, lastRowSubArray, procRank + 1,
                    GAME_MAT_SENDER_LAST_ROW, MPI_COMM_WORLD, &SendReq[1]); 

        } else if (procRank == lastNode) {
            DLOG (C_VERBOSE, "Node[%d] procRank == lastNode so, sending firstRow\n",procRank);
            /*To do :  create the first row of sub matrix */
            MPI_Isend (&gameSubMatPast[firstRow][0], 1, firstRowSubArray, procRank - 1,
                    GAME_MAT_SENDER_FIRST_ROW, MPI_COMM_WORLD, &SendReq[0]); 


        } else {
            DLOG (C_VERBOSE, "Node[%d] procRank is neither firstNode nor lastNode so, sending both firstRow & lastRow\n",procRank);
            /*To do :  create the last row of sub matrix */
            MPI_Isend (&gameSubMatPast[lastRow][0], 1, lastRowSubArray, procRank + 1,
                    GAME_MAT_SENDER_LAST_ROW, MPI_COMM_WORLD, &SendReq[0]); 


            /*To do :  create the first row of sub matrix */
            MPI_Isend (&gameSubMatPast[firstRow][0], 1, firstRowSubArray, procRank - 1,
                    GAME_MAT_SENDER_FIRST_ROW, MPI_COMM_WORLD, &SendReq[1]); 

        }

        DLOG (C_VERBOSE, "Node[%d] computing inner most submatrix \n",procRank);
        i = firstRow + 1 ; j=0;

        /* compute the inner most submatrix */
        for (i = firstRow + 1 ; i < lastRow ; i++) {

            for (j = 0; j < gameSubMatColmSize; j++){

                if (j == 0 ){
                    DLOG (C_VERBOSE, "Node[%d] j==0 first column \n",procRank);
                    /* for first column the  undefined values of H(t-1) are assumed to be 0 */
                    gameSubMatCur[i][j] = ( 0 + gameSubMatPast[i-1][j] + gameSubMatPast[i-1][j+1] + 
                            0 + 0  + gameSubMatPast[i][j+1]   + 
                            0 + gameSubMatPast[i+1][j] + gameSubMatPast[i+1][j+1]   );



                } else if ( j == gameSubMatColmSize-1) {
                    DLOG (C_VERBOSE, "Node[%d] j==0 last column \n",procRank);

                    /* for last column the  undefined values of H(t-1) are assumed to be 0 */
                    gameSubMatCur[i][j] = ( gameSubMatPast[i-1][j-1] + gameSubMatPast[i-1][j] + 0 + 
                            gameSubMatPast[i][j-1]   + 0  + 0 +  
                            gameSubMatPast[i+1][j-1] + gameSubMatPast[i+1][j] + 0   );



                } else {
                    DLOG (C_VERBOSE, "Node[%d] inner column \n",procRank);

                    gameSubMatCur[i][j] = ( gameSubMatPast[i-1][j-1] + gameSubMatPast[i-1][j] + gameSubMatPast[i-1][j+1] + 
                            gameSubMatPast[i][j-1]   + 0  + gameSubMatPast[i][j+1]   + 
                            gameSubMatPast[i+1][j-1] + gameSubMatPast[i+1][j] + gameSubMatPast[i+1][j+1]  );


                }
                gameSubMatCur[i][j] = ApplyTransitionRule(gameSubMatCur[i][j], gameSubMatPast[i][j]);

                countLiveCell = countLiveCell + gameSubMatCur[i][j];

            } /* end of colm for loop */

        }/* end of row for loop */




        /* compute first row . 
         * if its a first node then the undefined values of H(t-1) are assumed to be 0 
         * so the game equation is modified accordingly */

        if (procRank == firstNode) {;
            DLOG (C_VERBOSE, "Node[%d] procRank == firstNode , so assuming 0 for ghost rows\n",procRank);

            i = 0;   
            for (j = 0; j < gameSubMatColmSize; j++){

                if (j == 0 ){
                    /* for first column the  undefined values of H(t-1) are assumed to be 0 */
                    gameSubMatCur[i][j] = ( 0 + 0 + 0 + 
                            0 + 0   + gameSubMatPast[i][j+1]   + 
                            0 + gameSubMatPast[i+1][j] + gameSubMatPast[i+1][j+1]   );



                } else if ( j == gameSubMatColmSize-1) {

                    /* for last column the  undefined values of H(t-1) are assumed to be 0 */
                    gameSubMatCur[i][j] = ( 0 + 0 + 0 + 
                            gameSubMatPast[i][j-1]   + 0  + 0 +  
                            gameSubMatPast[i+1][j-1] + gameSubMatPast[i+1][j] + 0   );




                } else {

                    gameSubMatCur[i][j] = ( 0 + 0 + 0 + 
                            gameSubMatPast[i][j-1]   + 0  + gameSubMatPast[i][j+1]   + 
                            gameSubMatPast[i+1][j-1] + gameSubMatPast[i+1][j] + gameSubMatPast[i+1][j+1]  );


                }



                gameSubMatCur[i][j] = ApplyTransitionRule(gameSubMatCur[i][j], gameSubMatPast[i][j]);
                countLiveCell = countLiveCell + gameSubMatCur[i][j];


            }/* end of colm for loop */

        } else {
            DLOG (C_VERBOSE, "Node[%d] procRank != firstNode, so receiving upper row from the predessor node\n",procRank);

            /* receive the upper row from the predessor node and compute  */
            MPI_Recv (&receivedRow[0], gameSubMatColmSize, MPI_INT, procRank-1, GAME_MAT_SENDER_LAST_ROW, MPI_COMM_WORLD, &Status[0]);
            i = firstRow;


            for (j = 0; j < gameSubMatColmSize; j++){

                if (j == 0 ){
                    /* for first column the  undefined values of H(t-1) are assumed to be 0 */
                    gameSubMatCur[i][j] = ( 0 + receivedRow[j] + receivedRow[j+1] + 
                            0 + 0   + gameSubMatPast[i][j+1]   + 
                            0 + gameSubMatPast[i+1][j] + gameSubMatPast[i+1][j+1]   );



                } else if ( j == gameSubMatColmSize-1) {

                    /* for last column the  undefined values of H(t-1) are assumed to be 0 */
                    gameSubMatCur[i][j] = ( receivedRow[j-1] + receivedRow[j] + 0 + 
                            gameSubMatPast[i][j-1]   + 0  + 0 +  
                            gameSubMatPast[i+1][j-1] + gameSubMatPast[i+1][j] + 0   );




                } else {

                    gameSubMatCur[i][j] = ( receivedRow[j-1] + receivedRow[j] + receivedRow[j+1] + 
                            gameSubMatPast[i][j-1]   + 0  + gameSubMatPast[i][j+1]   + 
                            gameSubMatPast[i+1][j-1] + gameSubMatPast[i+1][j] + gameSubMatPast[i+1][j+1]  );


                }
                gameSubMatCur[i][j] = ApplyTransitionRule(gameSubMatCur[i][j], gameSubMatPast[i][j]);
                countLiveCell = countLiveCell + gameSubMatCur[i][j];
            }/* end of colm for loop */

        }

        /* compute last row . if its a first node then the undefined values of H(t-1) are assumed to be 0
         * if its a first node then the undefined values of H(t-1) are assumed to be 0 
         * so the game equation is modified accordingly */

        if (procRank == lastNode) {
            DLOG (C_VERBOSE, "procRank is lastNode. computing last row\n");

            i = lastRow;   
            for (j = 0; j < gameSubMatColmSize; j++){


                if (j == 0 ){
                    /* for first column the  undefined values of H(t-1) are assumed to be 0 */
                    gameSubMatCur[i][j] = ( 0 + gameSubMatPast[i-1][j] + gameSubMatPast[i-1][j+1] + 
                            0 + 0  + gameSubMatPast[i][j+1]   + 
                            0 + 0 + 0   );

                } else if ( j == gameSubMatColmSize-1) {

                    /* for last column the  undefined values of H(t-1) are assumed to be 0 */
                    gameSubMatCur[i][j] = ( gameSubMatPast[i-1][j-1] + gameSubMatPast[i-1][j] + 0 + 
                            gameSubMatPast[i][j-1]   + 0  + 0 +  
                            0 + 0 + 0   );


                } else {

                    gameSubMatCur[i][j] = ( gameSubMatPast[i-1][j-1] + gameSubMatPast[i-1][j] + gameSubMatPast[i-1][j+1] + 
                            gameSubMatPast[i][j-1]   + 0   + gameSubMatPast[i][j+1]   +
                            0 + 0 + 0  );

                }


                gameSubMatCur[i][j] = ApplyTransitionRule(gameSubMatCur[i][j], gameSubMatPast[i][j]);
                countLiveCell = countLiveCell + gameSubMatCur[i][j];

            }/* end of colm for loop */

        } else {
            DLOG (C_VERBOSE, "procRank != lastNode. Receiving last row from the successor node\n");

            /* receive the lower row from the successor node and compute  */
            MPI_Recv (&receivedRow[0], gameSubMatColmSize, MPI_INT, procRank+1, GAME_MAT_SENDER_FIRST_ROW, MPI_COMM_WORLD, &Status[1]);

            i = lastRow;


            for (j = 0; j < gameSubMatColmSize; j++){

                if (j == 0 ){
                    /* for first column the  undefined values of H(t-1) are assumed to be 0 */
                    gameSubMatCur[i][j] = ( 0 + gameSubMatPast[i-1][j] + gameSubMatPast[i-1][j+1] + 
                            0 + 0  + gameSubMatPast[i][j+1]   + 
                            0 + receivedRow[j] + receivedRow[j+1]   );

                } else if ( j == gameSubMatColmSize-1) {

                    /* for last column the  undefined values of H(t-1) are assumed to be 0 */
                    gameSubMatCur[i][j] = ( gameSubMatPast[i-1][j-1] + gameSubMatPast[i-1][j] + 0 + 
                            gameSubMatPast[i][j-1]   + 0  + 0 +  
                            receivedRow[j-1] + receivedRow[j] + 0   );


                } else {

                    gameSubMatCur[i][j] = ( gameSubMatPast[i-1][j-1] + gameSubMatPast[i-1][j] + gameSubMatPast[i-1][j+1] + 
                            gameSubMatPast[i][j-1]   + 0  + gameSubMatPast[i][j+1]   + 
                            receivedRow[j-1] + receivedRow[j] + receivedRow[j+1]  );
                }


                gameSubMatCur[i][j] = ApplyTransitionRule(gameSubMatCur[i][j], gameSubMatPast[i][j]);
                countLiveCell = countLiveCell + gameSubMatCur[i][j];

            }/* end of colm for loop */

        }


        MPI_Reduce(&countLiveCell, &totalLiveCell, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        if (procRank == NODE_0){

            std::cout<<"rank = "<<procRank<<"totalLiveCell = "<<totalLiveCell<<std::endl;
        }



        if (procRank == firstNode) {
            DLOG (C_VERBOSE, "Node[%d] waiting for the MPI_Isend to complete\n", procRank);
            MPI_Wait (&SendReq[1], MPI_STATUS_IGNORE);

        } else if (procRank == lastNode) {
            DLOG (C_VERBOSE, "Node[%d] waiting for the MPI_Isend to complete\n", procRank);
            MPI_Wait (&SendReq[0], MPI_STATUS_IGNORE);

        } else {
            DLOG (C_VERBOSE, "Node[%d] waiting for the MPI_Isend to complete\n", procRank);
            MPI_Waitall (2, &SendReq[0], MPI_STATUS_IGNORE);
        }

        DLOG (C_VERBOSE, "Node[%d] Copying heatSubMatCur to heatSubMatPast\n",procRank);
        for (i = 0; i < gameSubMatRowSize ; i++){
            memcpy ( &gameSubMatPast[i][0], &gameSubMatCur[i][0], gameSubMatColmSize * sizeof(int));
        }


    }


    MPI_Barrier( MPI_COMM_WORLD ) ;
    /* compute the time taken to compute the sum and display the same */
    if (procRank == NODE_0){
        EndTime = std::chrono::system_clock::now();
        ElapsedTime = EndTime - StartTime;

        std::cout<<"countLiveCell = "<<countLiveCell<<std::endl;
        std::cout<<"totalLiveCell = "<<totalLiveCell<<std::endl;
        std::cerr<<ElapsedTime.count()<<std::endl;
    }
    MPI_Barrier( MPI_COMM_WORLD ) ;

    for (i = 0; i < gameSubMatRowSize; i++){
        delete [] gameSubMatCur[i];
    }
    delete [] gameSubMatCur;

    for (i = 0; i < gameSubMatRowSize; i++){
        delete [] gameSubMatPast[i];
    }
    delete [] gameSubMatPast;
    delete [] receivedRow;


    MPI_Type_free(&lastRowSubArray);
    MPI_Type_free(&firstRowSubArray);
    MPI_Finalize();

    return 0;
}


/*==============================================================================
 *  ApplyTransitionRule
 *=============================================================================*/

int ApplyTransitionRule (int totalNeighborValue, int pastCellValue) {

    int cellValue = 0;

    int procRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);


    if (totalNeighborValue < 2){
        /* any live cell with fewer than two live neighbors dies */
        cellValue = 0;
    } else if ((totalNeighborValue < 3) && (pastCellValue == 1)) {
        /* any live cell with two or three live neighbors lives on */
        cellValue = 1;
    } else if ((totalNeighborValue < 3) && (pastCellValue == 0)) {
        /* any live cell with two or three live neighbors lives on */
        cellValue = 0;
    } else if (totalNeighborValue == 3) {
        /* any dead cell with exactly three live neighbors becomes a live cell */
        cellValue = 1;
    } else if (totalNeighborValue > 3) {
        /* any live cell with more than three live neighbors dies */
        cellValue = 0;

    }

    return cellValue;
}

/*==============================================================================
 *  InitGameMatrix
 *=============================================================================*/

void InitGameMatrix (int *** gameSubMatCur, int gameSubMatRowSize, int gameSubMatColmSize, int indexValue) {

    int i,j;
    int ** gameMatrix = new int * [gameSubMatRowSize];

    (*gameSubMatCur) = gameMatrix;
    int procRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    DLOG (C_VERBOSE, "Node[%d] Enter\n",procRank);

    for (i = 0; i < gameSubMatRowSize; i++){
        gameMatrix[i] = new int [gameSubMatColmSize];
        (*gameSubMatCur)[i] = gameMatrix[i];
    }

    if (indexValue == 0) {
        /* set the array elements to 0, use memset */
        for (i = 0; i < gameSubMatRowSize ; i++ ) {
            for ( j = 0; j < gameSubMatColmSize ; j++ ) {
                gameMatrix[i][j] = 0; 
            }/* end of colm for loop */
        }/* end of row for loop */

    } else {
        /* randomly set the array elements or set elements in a particular order */

        for (i = 0; i < gameSubMatRowSize ; i++ ) {
            for ( j = 0; j < gameSubMatColmSize ; j++ ) {


                if ( (i+j) % 2 == 0) {
                    gameMatrix[i][j] = 1; 
                } else {
                    gameMatrix[i][j] = 0; 
                }


            }/* end of colm for loop */
        }/* end of row for loop */
    }


    DLOG (C_VERBOSE, "Node[%d] Exit\n",procRank);
}
