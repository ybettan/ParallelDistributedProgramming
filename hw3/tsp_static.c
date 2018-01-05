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
//                                heuristic
//-----------------------------------------------------------------------------

/* for a given cost of a node, caculate the lower bound of the path using the 
 * lightest edge of each vertex that wehere not visited. 
 * (the path may not exist but this will be out lower bound) */
int heuristic(const bool *visitedNodesUntilNow, int currCost) {
    
    int res = currCost;

    /* go over all not-visited vertex */
    for (int i=0 ; i<citiesNum ; i++) {

        if (visitedNodesUntilNow[i])
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
//                               State
//-----------------------------------------------------------------------------

typedef struct {
    
    int vertex;
    int cost;
    /* for example {3, 5, 1, NOT_SET, NOT_SET} for citiesNum=5 */
    int *shortestPathUntilNow;
    /* visitedNodesUntilNow[i] == true if and only if node i was visited */
    bool *visitedNodesUntilNow;

} State;


/* create a state with vertex=vertex, cost according to heuristic function
 * starting with currCost=0, shortestPathUntilNow=shortestPathUntilNow and 
 * visitedNodesUntilNow=visitedNodesUntilNow 
 * NOTE:
 * the array are copy so the state is the owner of the data and user should
 * free the memory of his array after send it to this function */
State* create_state(int vertex, int *shortestPathUntilNow, 
        bool *visitedNodesUntilNow) {

    State *state = malloc(sizeof(*state));
    if ( !state )
        return NULL;

    state->vertex = vertex;  

    /* compute the state's cost */
    int cost = 0;
    for (int i=1 ; i<citiesNum ; i++) {
        if (shortestPathUntilNow[i] == NOT_SET)
            break;
        cost += agencyMatrix[shortestPathUntilNow[i]][shortestPathUntilNow[i-1]];
    }
    state->cost = cost;

    /* create a copy of shortestPathUntilNow and visitedNodesUntilNow to be owner of 
     * the data */

    /* shortestPathUntilNow */
    state->shortestPathUntilNow = malloc(citiesNum * sizeof(int));
    if ( !state->shortestPathUntilNow )
        return NULL;
    for (int i=0 ; i<citiesNum ; i++) {
        state->shortestPathUntilNow[i] = shortestPathUntilNow[i];
    }

    /* visitedNodesUntilNow */
    state->visitedNodesUntilNow = malloc(citiesNum * sizeof(bool));
    if ( !state->visitedNodesUntilNow ) {
        free(state->shortestPathUntilNow);
        return NULL;
    }
    for (int i=0 ; i<citiesNum ; i++) {
        state->visitedNodesUntilNow[i] = visitedNodesUntilNow[i];
    }

    return state;
}

/* free the allocated memory for State struct */
void free_state(State *state) {
    free(state->shortestPathUntilNow);
    free(state->visitedNodesUntilNow);
    free(state);
}

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
//                              CPU main
//-----------------------------------------------------------------------------

/* the main function runned by a single CPU 
 * returns:
 * the minimum lenght as a return value
 * the shortest path in an input ptr */
//int cpu_main(State *state, int *shortestPathUntilNow) {
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
    bool visitedNodesUntilNow[3] = {false, false, false};
    assert(heuristic(visitedNodesUntilNow, 0) == 11);
    visitedNodesUntilNow[0] = true;
    assert(heuristic(visitedNodesUntilNow, 0) == 8);
    assert(heuristic(visitedNodesUntilNow, 10) == 18);
    visitedNodesUntilNow[1] = true;
    visitedNodesUntilNow[2] = true;
    assert(heuristic(visitedNodesUntilNow, 0) == 0);
    assert(heuristic(visitedNodesUntilNow, 20) == 20);

    /* create_test test */
    int shortestPathUntilNow[] = {1, NOT_SET, NOT_SET};
    visitedNodesUntilNow[0] = false;
    visitedNodesUntilNow[1] = true;
    visitedNodesUntilNow[2] = false;
    /* now visitedNodesUntilNow[] = {false, true, false} */
    State *state = create_state(1, shortestPathUntilNow, visitedNodesUntilNow);
    assert(state->vertex == 1);
    assert(state->cost == 0);
    shortestPathUntilNow[1] = 2;
    /* now shortestPathUntilNow[] = {1, 2, NOT_SET} */
    visitedNodesUntilNow[2] = true;
    /* now visitedNodesUntilNow[] = {false, true, true} */
    assert(state->shortestPathUntilNow[0] == 1);
    assert(state->shortestPathUntilNow[1] == NOT_SET);
    assert(state->shortestPathUntilNow[2] == NOT_SET);
    assert(state->visitedNodesUntilNow[0] == false);
    assert(state->visitedNodesUntilNow[1] == true);
    assert(state->visitedNodesUntilNow[2] == false);
    state = create_state(2, shortestPathUntilNow, visitedNodesUntilNow);
    assert(state->vertex == 2);
    assert(state->cost == 5);
    shortestPathUntilNow[2] = 0;
    /* now shortestPathUntilNow[] = {1, 2, 0} */
    visitedNodesUntilNow[0] = true;
    /* now visitedNodesUntilNow[] = {true, true, true} */
    state = create_state(0, shortestPathUntilNow, visitedNodesUntilNow);
    assert(state->vertex == 0);
    assert(state->cost == 12);





    
    
    
    free_agency_matrix(agencyMatrix, 3);
}








