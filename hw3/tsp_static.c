//#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>


#define INF 999999
#define NOT_SET -1

/*
 * restruction:
 *
 * data distribution --> only point to point communication is allowed
 * results gathering --> only collective communication is allowed
 * we can't bound the number of citys
 */


//-----------------------------------------------------------------------------
//                               Globals
//-----------------------------------------------------------------------------

int **agencyMatrix;
int citiesNum;

//-----------------------------------------------------------------------------
//                               State
//-----------------------------------------------------------------------------
//typedef struct {
//    
//    int vertex;
//    int cost;
//    int *shortestPath;
//    /* visitedNodes[i] == true if and only if node i was visited */
//    bool *visitedNodes;
//
//} State;
//
//
//State* create_state(int vertex, int cost, int *shortestPath, bool *visitedNodes) {
//
//    State *state = malloc(sizeof(*state));
//    if ( !state )
//        return NULL;
//
//    //FIXME: change to copy ?
//    state->vertex = vertex;  
//    state->cost = cost;  
//    state->shortestPath = shortestPath;  
//    state->visitedNodes = visitedNodes;  
//}


//-----------------------------------------------------------------------------
//                             agency matrix
//-----------------------------------------------------------------------------

/* crate an agency matrix for the given graph for easier caculations */
void create_agency_matrix(int *xCoord, int *yCoord) {

    /* allocate  the matrix */
    agencyMatrix = malloc(citiesNum * citiesNum * sizeof(int*));
    if ( !agencyMatrix ) {
        printf("cannot allocate agencyMatrix\n");
        exit(1);
    }
    for (int i=0 ; i<citiesNum ; i++) {
        agencyMatrix[i] = malloc(citiesNum * sizeof(int));
    }

    /* insert reale values */
    for (int i=0 ; i<citiesNum ; i++) {
        for(int j=0 ; j<citiesNum ; j++) {
            if (i==j) {
                agencyMatrix[i][j] = INF;
            } else {
                agencyMatrix[i][j] = 
                    2*abs(xCoord[i]-xCoord[j]) + 2*abs(yCoord[i]-yCoord[j]) + 1; 
            }
        }
    } 
}


/* free the allocated memory for the agency maatrix */
void free_agency_matrix() {

    for (int i=0 ; i<citiesNum ; i++) {
        free(agencyMatrix[i]);
    }
    free(agencyMatrix);
}


//FIXME: for debugging
//void print_matrix(int **matrix, int size) {
//    
//    for (int i=0 ; i<size ; i++) {
//        for (int j=0 ; j<size; j++) {
//            printf("%d\t",matrix[i][j]);
//        }
//        printf("\n");
//    }
//}


//-----------------------------------------------------------------------------
//                                heuristic
//-----------------------------------------------------------------------------

/* for a given cost of a node, caculate the lower bound of the path using the 
 * lightest edge of each vertex that wehere not visited. 
 * (the path may not exist but this will be out lower bound) */
int heuristic(bool *visitedNodes, int currCost) {
    
    int res = currCost;

    /* go over all not-visited vertex */
    for (int i=0 ; i<citiesNum ; i++) {

        if (visitedNodes[i])
            continue;

        /* find the lightest edge of vertex i */
        int lightestEdge = INF;
        for (int j=0 ; j<citiesNum ; j++) {
            if (agencyMatrix[i][j] < lightestEdge) {
                lightestEdge = agencyMatrix[i][j];
            }
        }
        res += lightestEdge;
    }
    return res;
}



//-----------------------------------------------------------------------------
//                              CPU main
//-----------------------------------------------------------------------------

/* the main function runned by a single CPU 
 * returns:
 * the minimum lenght as a return value
 * the shortest path in an input ptr */
//int cpu_main(State *state, int *shortestPath) {
//    
//    sdf
//}












// The static parellel algorithm main function.
int tsp_main(int citiesNum, int xCoord[], int yCoord[], int shortestPath[])
{
	return -1;	//TODO
}




//FIXME: for testing
int main() {

    int xCoord[3] = {1, 1, 1};
    int yCoord[3] = {1, 2, 4};

    /* create matrix test */
    citiesNum = 3;
    create_agency_matrix(xCoord, yCoord);
    assert(agencyMatrix[0][0] == INF);
    assert(agencyMatrix[0][1] == 3);
    assert(agencyMatrix[0][2] == 7);
    assert(agencyMatrix[1][0] == 3);
    assert(agencyMatrix[1][1] == INF);
    assert(agencyMatrix[1][2] == 5);
    assert(agencyMatrix[2][0] == 7);
    assert(agencyMatrix[2][1] == 5);
    assert(agencyMatrix[2][2] == INF);
    //print_matrix(matrix, 3);
    
    /* heuristic test */
    bool visitedNodes[3] = {false, false, false};
    assert(heuristic(visitedNodes, 0) == 11);
    visitedNodes[0] = true;
    assert(heuristic(visitedNodes, 0) == 8);
    assert(heuristic(visitedNodes, 10) == 18);
    visitedNodes[1] = true;
    visitedNodes[2] = true;
    assert(heuristic(visitedNodes, 0) == 0);
    assert(heuristic(visitedNodes, 20) == 20);






    
    
    
    free_agency_matrix(agencyMatrix, 3);
}








