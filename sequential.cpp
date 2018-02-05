/*
 * File Name  :sequential.cpp
 * Ver        :0.1
 * Date       :Nov 29 2017
 * Author     :Karthik Rao
 *
 * To compile :
 * Make sequential
 *
 * To compile :
 * mpicxx -std=c++11 sequential.cpp -o sequential
 * 
 * Sample command line execution :
 * 
 * mpirun -n 2 ./sequential  6
 * mpirun -n 4 ./sequential  11200
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
    int procRank = 0;

    int firstNode = NODE_0;
    int lastNode = gameMatSize - 1;

    (void)firstNode;
    (void)lastNode;

    int ** gameSubMatPast;
    int ** gameSubMatCur;
    gameSubMatColmSize = gameMatSize;
    gameSubMatRowSize = gameMatSize;


    StartTime = std::chrono::system_clock::now();


    InitGameMatrix (&gameSubMatPast, gameSubMatRowSize, gameSubMatColmSize, 1);
    InitGameMatrix (&gameSubMatCur, gameSubMatRowSize, gameSubMatColmSize, 0);

    firstRow = 0 ;
    lastRow  = gameSubMatRowSize - 1;



    DLOG (C_VERBOSE, "Node[%d] firstRow  = %d. lastRow  = %d\n",procRank, firstRow, lastRow);
    DLOG (C_VERBOSE, "Node[%d] gameSubMatRowSize  = %d. gameSubMatColmSize  = %d\n",procRank, gameSubMatRowSize, gameSubMatColmSize);



    /* allocate memory for the received rows */
    int * receivedRow = new int [gameSubMatColmSize];

    for (int k = 0; k < 10 ; k++) {


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

        DLOG (C_VERBOSE, "Node[%d] procRank == firstNode , so assuming 0 for ghost rows\n",procRank);

        for (j = 0; j < gameSubMatColmSize; j++){
            i = 0;   

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

        /* compute last row .
         * if its a first node then the undefined values of H(t-1) are assumed to be 0 
         * so the game equation is modified accordingly */


        for (j = 0; j < gameSubMatColmSize; j++){
            i = lastRow;   

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



        totalLiveCell = countLiveCell;


        for (i = 0; i < gameSubMatRowSize ; i++){
            memcpy ( &gameSubMatPast[i][0], &gameSubMatCur[i][0], gameSubMatColmSize * sizeof(int));
        }



    }


    /* compute the time taken to compute the sum and display the same */
    EndTime = std::chrono::system_clock::now();
    ElapsedTime = EndTime - StartTime;

    std::cout<<"countLiveCell = "<<countLiveCell<<std::endl;
    std::cout<<"totalLiveCell = "<<totalLiveCell<<std::endl;
    std::cerr<<ElapsedTime.count()<<std::endl;

    for (i = 0; i < gameSubMatRowSize; i++){
        delete [] gameSubMatCur[i];
    }
    delete [] gameSubMatCur;

    for (i = 0; i < gameSubMatRowSize; i++){
        delete [] gameSubMatPast[i];
    }
    delete [] gameSubMatPast;
    delete [] receivedRow;



    return 0;
}


/*==============================================================================
 *  ApplyTransitionRule
 *=============================================================================*/

int ApplyTransitionRule (int totalNeighborValue, int pastCellValue) {

    int cellValue = 0;

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
    int procRank = 0;

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

