//#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>


#define INF 3000000
#define NOT_SET -1
#define ERROR -2

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
    /* hold the shortest path of the best leaf-son of the state */
    int *shortestPathRes;

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
    state->shortestPathUntilNow = shortestPathUntilNow;
    state->visitedNodesUntilNow = visitedNodesUntilNow;

    state->shortestPathRes = malloc(citiesNum * sizeof(int));
    if ( !state->shortestPathRes )
        return NULL;

    return state;
}

/* free the allocated memory for State struct */
void free_state(State *state) {
    if ( !state )
        return;
    free(state->shortestPathUntilNow);
    free(state->visitedNodesUntilNow);
    free(state->shortestPathRes);
    free(state);
}

//-----------------------------------------------------------------------------
//                             agency matrix
//-----------------------------------------------------------------------------

/* crate an agency matrix for the given graph for easier caculations */
void create_agency_matrix(int *xCoord, int *yCoord) {

    /* allocate  the matrix */
    agencyMatrix = malloc(citiesNum * sizeof(int*));
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


/* free the allocated memory for the agency matrix */
void free_agency_matrix(void) {

    for (int i=0 ; i<citiesNum ; i++) {
        free(agencyMatrix[i]);
    }
    free(agencyMatrix);
}


//-----------------------------------------------------------------------------
//                              CPU main
//-----------------------------------------------------------------------------

bool is_leaf(const State *state) {
    for (int i=0 ; i<citiesNum ; i++) {
        if ( !state->visitedNodesUntilNow[i] )
            return false;
    }
    return true;
}

/* copy state->shortestPathUntilNow into the result array */
void copy_shortest_path(const int *src, int *dst) {
    for (int i=0 ; i<citiesNum ; i++) {
        dst[i] = src[i];
    }
}

/* the main function runned by a single CPU
 * returns:
 * the minimum lenght as a return value
 * the shortest path in shortestPath variable */
//FIXME: add prunning according to heuristic function
int cpu_main(State *state, int *shortestPath) {

    if (is_leaf(state)) {
        int rootVertex = state->shortestPathUntilNow[0];
        int lastVertex = state->shortestPathUntilNow[citiesNum-1];
        int costBackToRoot = agencyMatrix[rootVertex][lastVertex];
        /* insert the shortest path result to the state */
        copy_shortest_path(state->shortestPathUntilNow, state->shortestPathRes);
        copy_shortest_path(state->shortestPathRes, shortestPath);
        return state->cost + costBackToRoot;
    }

    int minPathLen = INF;
    State *bestSon = NULL;

    /* check all the possible son's */
    for (int i=0 ; i<citiesNum ; i++) {

        if (state->visitedNodesUntilNow[i])
            continue;

        /* create shortestPathUntilNow array */
        int *shortestPathUntilNow = malloc(citiesNum * sizeof(int));
        if ( !shortestPathUntilNow ) {
            printf("cannot allocate shortestPathUntilNow\n");
            return ERROR;
        }
        bool updated = false;
        for (int k=0 ; k<citiesNum ; k++) {
            if(state->shortestPathUntilNow[k] == NOT_SET && !updated) {
                shortestPathUntilNow[k] = i;
                updated = true;
            } else {
                shortestPathUntilNow[k] = state->shortestPathUntilNow[k];
            }
        }

        /* create visitedNodesUntilNow array */
        bool *visitedNodesUntilNow = malloc(citiesNum * sizeof(bool));
        if ( !visitedNodesUntilNow ) {
            printf("cannot allocate visitedNodesUntilNow\n");
            return ERROR;
        }
        for (int k=0 ; k<citiesNum ; k++) {
            visitedNodesUntilNow[k] = state->visitedNodesUntilNow[k];
        }
        visitedNodesUntilNow[i] = true;

        /* create the Son state */
        State *son = create_state(i, shortestPathUntilNow, visitedNodesUntilNow);
        if ( !son ) {
            printf("cannot allocate son\n");
            return ERROR;
        }

        /* compute the result for this son */
        int res = cpu_main(son, shortestPath);

        if (res == ERROR) {
            printf("ERROR returned from cpu_main()\n");
            return ERROR;
        }

        if (res < minPathLen) {
            minPathLen = res;
            free_state(bestSon);
            bestSon = son;
        }

        if (son != bestSon)
            free_state(son);
    }
        
    copy_shortest_path(bestSon->shortestPathRes, state->shortestPathRes);
    copy_shortest_path(state->shortestPathRes, shortestPath);
    free_state(bestSon);
    return minPathLen;
}



//-----------------------------------------------------------------------------
//                                    MPI
//-----------------------------------------------------------------------------

// The static parellel algorithm main function.
int tsp_main(int citiesNum, int xCoord[], int yCoord[], int shortestPath[])
{
	return -1;	//TODO
}



//-----------------------------------------------------------------------------
//                                  tests
//                              FIXME:remove
//-----------------------------------------------------------------------------

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
    assert(state->shortestPathUntilNow[0] == 1);
    assert(state->shortestPathUntilNow[1] == NOT_SET);
    assert(state->shortestPathUntilNow[2] == NOT_SET);
    assert(state->visitedNodesUntilNow[0] == false);
    assert(state->visitedNodesUntilNow[1] == true);
    assert(state->visitedNodesUntilNow[2] == false);
    shortestPathUntilNow[1] = 2;
    /* now shortestPathUntilNow[] = {1, 2, NOT_SET} */
    visitedNodesUntilNow[2] = true;
    /* now visitedNodesUntilNow[] = {false, true, true} */
    free(state);
    state = create_state(2, shortestPathUntilNow, visitedNodesUntilNow);
    assert(state->vertex == 2);
    assert(state->cost == 5);
    shortestPathUntilNow[2] = 0;
    /* now shortestPathUntilNow[] = {1, 2, 0} */
    visitedNodesUntilNow[0] = true;
    /* now visitedNodesUntilNow[] = {true, true, true} */
    free(state);
    state = create_state(0, shortestPathUntilNow, visitedNodesUntilNow);
    assert(state->vertex == 0);
    assert(state->cost == 12);
    free(state);

    /* is_leaf test */
    int shortestPathUntilNow2[] = {1, NOT_SET, NOT_SET};
    bool visitedNodesUntilNow2[3] = {false, false, false};
    State *state2 = create_state(3, shortestPathUntilNow2, visitedNodesUntilNow2);
    assert(!is_leaf(state2));
    visitedNodesUntilNow2[1] = true;
    free(state2);
    state2 = create_state(3, shortestPathUntilNow2, visitedNodesUntilNow2);
    assert(!is_leaf(state2));
    visitedNodesUntilNow2[0] = true;
    visitedNodesUntilNow2[2] = true;
    free(state2);
    state2 = create_state(3, shortestPathUntilNow2, visitedNodesUntilNow2);
    assert(is_leaf(state2));
    free(state2);

    free_agency_matrix();


    /* cpu_main test */
    int xCoord2[] = {0, 1, 1, 0};
    int yCoord2[] = {0, 0, 1, 1};
    citiesNum = 4;
    int shortestPath[4];
    create_agency_matrix(xCoord2, yCoord2);
    
    //---------------------------------------------------------------- len 4
    int shortestPathUntilNow3[] = {0,1,2,3};
    bool visitedNodesUntilNow3[] = {true, true, true, true};
    State *state3 = create_state(0, shortestPathUntilNow3, visitedNodesUntilNow3);   
    assert(cpu_main(state3, shortestPath) == 12);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 1);
    assert(shortestPath[2] == 2);
    assert(shortestPath[3] == 3);
    free(state3->shortestPathRes);
    free(state3);

    int shortestPathUntilNow4[] = {0,1,3,2};
    bool visitedNodesUntilNow4[] = {true, true, true, true};
    State *state4 = create_state(0, shortestPathUntilNow4, visitedNodesUntilNow4);   
    assert(cpu_main(state4, shortestPath) == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 1);
    assert(shortestPath[2] == 3);
    assert(shortestPath[3] == 2);
    free(state4->shortestPathRes);
    free(state4);

    int shortestPathUntilNow5[] = {0,2,1,3};
    bool visitedNodesUntilNow5[] = {true, true, true, true};
    State *state5 = create_state(0, shortestPathUntilNow5, visitedNodesUntilNow5);   
    assert(cpu_main(state5, shortestPath) == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 2);
    assert(shortestPath[2] == 1);
    assert(shortestPath[3] == 3);
    free(state5->shortestPathRes);
    free(state5);

    int shortestPathUntilNow6[] = {0,2,3,1};
    bool visitedNodesUntilNow6[] = {true, true, true, true};
    State *state6 = create_state(0, shortestPathUntilNow6, visitedNodesUntilNow6);   
    assert(cpu_main(state6, shortestPath) == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 2);
    assert(shortestPath[2] == 3);
    assert(shortestPath[3] == 1);
    free(state6->shortestPathRes);
    free(state6);

    int shortestPathUntilNow7[] = {0,3,1,2};
    bool visitedNodesUntilNow7[] = {true, true, true, true};
    State *state7 = create_state(0, shortestPathUntilNow7, visitedNodesUntilNow7);   
    assert(cpu_main(state7, shortestPath) == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 3);
    assert(shortestPath[2] == 1);
    assert(shortestPath[3] == 2);
    free(state7->shortestPathRes);
    free(state7);

    int shortestPathUntilNow8[] = {0,3,2,1};
    bool visitedNodesUntilNow8[] = {true, true, true, true};
    State *state8 = create_state(0, shortestPathUntilNow8, visitedNodesUntilNow8);   
    assert(cpu_main(state8, shortestPath) == 12);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 3);
    assert(shortestPath[2] == 2);
    assert(shortestPath[3] == 1);
    free(state8->shortestPathRes);
    free(state8);

    //---------------------------------------------------------------- len 3
    int shortestPathUntilNow9[] = {0,1,2,NOT_SET};
    bool visitedNodesUntilNow9[] = {true, true, true, false};
    State *state9 = create_state(0, shortestPathUntilNow9, visitedNodesUntilNow9);   
    assert(cpu_main(state9, shortestPath) == 12);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 1);
    assert(shortestPath[2] == 2);
    assert(shortestPath[3] == 3);
    free(state9->shortestPathRes);
    free(state9);

    int shortestPathUntilNow10[] = {0,1,3,NOT_SET};
    bool visitedNodesUntilNow10[] = {true, true, false, true};
    State *state10=create_state(0, shortestPathUntilNow10, visitedNodesUntilNow10);   
    assert(cpu_main(state10, shortestPath) == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 1);
    assert(shortestPath[2] == 3);
    assert(shortestPath[3] == 2);
    free(state10->shortestPathRes);
    free(state10);

    int shortestPathUntilNow11[] = {0,2,1,NOT_SET};
    bool visitedNodesUntilNow11[] = {true, true, true, false};
    State *state11=create_state(0, shortestPathUntilNow11, visitedNodesUntilNow11);   
    assert(cpu_main(state11, shortestPath) == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 2);
    assert(shortestPath[2] == 1);
    assert(shortestPath[3] == 3);
    free(state11->shortestPathRes);
    free(state11);

    int shortestPathUntilNow12[] = {0,2,3,NOT_SET};
    bool visitedNodesUntilNow12[] = {true, false, true, true};
    State *state12=create_state(0, shortestPathUntilNow12, visitedNodesUntilNow12);   
    assert(cpu_main(state12, shortestPath) == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 2);
    assert(shortestPath[2] == 3);
    assert(shortestPath[3] == 1);
    free(state12->shortestPathRes);
    free(state12);

    int shortestPathUntilNow13[] = {0,3,1,NOT_SET};
    bool visitedNodesUntilNow13[] = {true, true, false, true};
    State *state13=create_state(0, shortestPathUntilNow13, visitedNodesUntilNow13);   
    assert(cpu_main(state13, shortestPath) == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 3);
    assert(shortestPath[2] == 1);
    assert(shortestPath[3] == 2);
    free(state13->shortestPathRes);
    free(state13);

    int shortestPathUntilNow14[] = {0,3,2,NOT_SET};
    bool visitedNodesUntilNow14[] = {true, false, true, true};
    State *state14=create_state(0, shortestPathUntilNow14, visitedNodesUntilNow14);   
    assert(cpu_main(state14, shortestPath) == 12);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 3);
    assert(shortestPath[2] == 2);
    assert(shortestPath[3] == 1);
    free(state14->shortestPathRes);
    free(state14);

    //---------------------------------------------------------------- len 2
    int shortestPathUntilNow15[] = {0,1,NOT_SET,NOT_SET};
    bool visitedNodesUntilNow15[] = {true, true, false, false};
    State *state15=create_state(0, shortestPathUntilNow15, visitedNodesUntilNow15);   
    assert(cpu_main(state15, shortestPath) == 12);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 1);
    assert(shortestPath[2] == 2);
    assert(shortestPath[3] == 3);
    free(state15->shortestPathRes);
    free(state15);

    int shortestPathUntilNow16[] = {0,2,NOT_SET,NOT_SET};
    bool visitedNodesUntilNow16[] = {true, false, true, false};
    State *state16=create_state(0, shortestPathUntilNow16, visitedNodesUntilNow16);   
    assert(cpu_main(state16, shortestPath) == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 2);
    assert((shortestPath[2] == 1 && shortestPath[3] == 3) ||
           (shortestPath[2] == 3 && shortestPath[3] == 1));
    free(state16->shortestPathRes);
    free(state16);

    int shortestPathUntilNow17[] = {0,3,NOT_SET,NOT_SET};
    bool visitedNodesUntilNow17[] = {true, false, false, true};
    State *state17=create_state(0, shortestPathUntilNow17, visitedNodesUntilNow17);   
    assert(cpu_main(state17, shortestPath) == 12);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 3);
    assert(shortestPath[2] == 2);
    assert(shortestPath[3] == 1);
    free(state17->shortestPathRes);
    free(state17);








    free_agency_matrix();
}








