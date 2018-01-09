//#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>


#define INF 3000000
#define NOT_SET -1
#define ERROR -2
#define TAG 99




//-----------------------------------------------------------------------------
//                             agency matrix
//-----------------------------------------------------------------------------

/* crate an agency matrix for the given graph for easier caculations */
int** create_agency_matrix(int *xCoord, int *yCoord, int citiesNum) {

    /* allocate  the matrix */
    int **agencyMatrix = malloc(citiesNum * sizeof(int*));
    assert(agencyMatrix);

    for (int i=0 ; i<citiesNum ; i++) {
        agencyMatrix[i] = malloc(citiesNum * sizeof(int));
        assert(agencyMatrix[i]);
    }

    /* insert real values */
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

    return agencyMatrix;
}
//FIXME:remove
///* crate an agency matrix for the given graph for easier caculations */
//int** create_agency_matrix(int *xCoord, int *yCoord, int citiesNum) {
//
//    /* allocate  the matrix */
//    int **agencyMatrix = malloc(citiesNum * sizeof(int*));
//    if ( !agencyMatrix ) {
//        printf("cannot allocate agencyMatrix\n");
//        return NULL;
//    }
//    for (int i=0 ; i<citiesNum ; i++) {
//        agencyMatrix[i] = malloc(citiesNum * sizeof(int));
//        if ( !agencyMatrix[i] ) {
//            printf("cannot allocate agencyMatrix\n");
//            return NULL;
//        }
//    }
//
//    /* insert reale values */
//    for (int i=0 ; i<citiesNum ; i++) {
//        for(int j=0 ; j<citiesNum ; j++) {
//            if (i==j) {
//                agencyMatrix[i][j] = INF;
//            } else {
//                agencyMatrix[i][j] =
//                    2*abs(xCoord[i]-xCoord[j]) + 2*abs(yCoord[i]-yCoord[j]) + 1;
//            }
//        }
//    }
//    return agencyMatrix;
//}


/* free the allocated memory for the agency matrix 
 * MEM NOTE:
 * if agencyMatrix is NULL nothing is done */
void free_agency_matrix(int **agencyMatrix, int citiesNum) {

    if ( !agencyMatrix )
        return;

    for (int i=0 ; i<citiesNum ; i++) {
        free(agencyMatrix[i]);
    }
    free(agencyMatrix);
}

//-----------------------------------------------------------------------------
//                               State
//-----------------------------------------------------------------------------

typedef struct {

    int vertex;
    int cost;
    /* for example {3, 5, 1, NOT_SET, NOT_SET} for citiesNum=5 */
    int shortestPathUntilNow[1];

} State;

//FIXME:remove
//typedef struct {
//
//    int vertex;
//    int cost;
//    /* for example {3, 5, 1, NOT_SET, NOT_SET} for citiesNum=5 */
//    int *shortestPathUntilNow;
//    /* visitedNodesUntilNow[i] == 1 if and only if node i was visited */
//    int *visitedNodesUntilNow;
//    /* hold the shortest path of the best leaf-son of the state */
//    int *shortestPathRes;
//
//} State;

/* allocate memory for arrays of int with all initiallized to NOT_SET */
int* allocate_empty_array_of_int(int size) {

    int *res = malloc(size * sizeof(int));
    assert(res);

    for (int i=0 ; i<size ; i++) {
        res[i] = NOT_SET;
    }
    return res;
}

void copy_array_of_int(int *src, int *dst, int size) {
    for (int i=0 ; i<size ; i++) {
        dst[i] = src[i];
    }
}

void free_array_of_int(int *arr) {
    free(arr);
}

/* creates an array of states which each state get vertex, its cost and a copy
 * of shortestPathUntilNow input array.
 * MEM NOTE:
 * shortestPathUntilNow is freed inside the function */
State* create_array_of_state(int size, int citiesNum, int vertex,
        int *shortestPathUntilNow, int **agencyMatrix) {

    State *arr = malloc(size * (sizeof(State) + (citiesNum-1) * sizeof(int)));
    assert(arr);

    /* init each cell to obtain vertex, cost and shortestPathUntilNow */
    for (int i=0 ; i<size ; i++) {

        arr[i].vertex = vertex;

        /* compute the state's cost */
        int cost = 0;
        for (int k=1 ; k<citiesNum ; k++) {
            if (shortestPathUntilNow[k] == NOT_SET)
                break;
            cost += agencyMatrix[shortestPathUntilNow[k]][shortestPathUntilNow[k-1]];
        }
        arr[i].cost = cost;

        /* copy shortestPathUntilNow */
        copy_array_of_int(shortestPathUntilNow, arr[i].shortestPathUntilNow,
                citiesNum);
    }

    free(shortestPathUntilNow);
    return arr;
}

/* copy src array of states to dst array of states */
void copy_array_of_states(State *src, State *dst, int size, int citiesNum) {

    for (int i=0 ; i<size ; i++) {

        dst[i].vertex = src[i].vertex;
        dst[i].cost = src[i].cost;
        copy_array_of_int(src[i].shortestPathUntilNow,
                dst[i].shortestPathUntilNow, citiesNum);
    }
}

/* free the memory allocated for the array.
 * MEM NOTE:
 * do nothing in input is NULL */
void free_array_of_states(State *arr) {
    if ( !arr )
        return;
    free(arr);
}

//FIXME:remove
/* create a state with vertex=vertex, cost according to heuristic function
 * starting with currCost=0, shortestPathUntilNow=shortestPathUntilNow and
 * visitedNodesUntilNow=visitedNodesUntilNow */
//State* create_state(int vertex, int *shortestPathUntilNow,
//        int *visitedNodesUntilNow, int citiesNum, int **agencyMatrix) {
//
//    State *state = malloc(sizeof(*state));
//    if ( !state )
//        return NULL;
//
//    state->vertex = vertex;
//
//    /* compute the state's cost */
//    int cost = 0;
//    for (int i=1 ; i<citiesNum ; i++) {
//        if (shortestPathUntilNow[i] == NOT_SET)
//            break;
//        cost += agencyMatrix[shortestPathUntilNow[i]][shortestPathUntilNow[i-1]];
//    }
//    state->cost = cost;
//    state->shortestPathUntilNow = shortestPathUntilNow;
//    state->visitedNodesUntilNow = visitedNodesUntilNow;
//
//    state->shortestPathRes = malloc(citiesNum * sizeof(int));
//    if ( !state->shortestPathRes )
//        return NULL;
//
//    return state;
//}

///* copy src state to dst state */
//void copy_state(State *src, State *dst, int citiesNum) {
//    dst->vertex = src->vertex;
//    dst->cost = src->cost;
//    for (int i=0 ; i<citiesNum ; i++) {
//        dst->shortestPathUntilNow[i] = src->shortestPathUntilNow[i];
//        dst->visitedNodesUntilNow[i] = src->visitedNodesUntilNow[i];
//        dst->shortestPathRes[i] = src->shortestPathRes[i];
//    }
//}

///* free the allocated memory for State struct */
//void free_state(State *state) {
//    if ( !state )
//        return;
//    free(state->shortestPathUntilNow);
//    free(state->visitedNodesUntilNow);
//    free(state->shortestPathRes);
//    free(state);
//}

///* allocate memory for arrays of in with all initiallized to NOT_SET */
//int* allocate_empty_array_of_int(int citiesNum) {
//    int *res = malloc(citiesNum * sizeof(int));
//    assert(res);
//    for (int i=0 ; i<citiesNum ; i++) {
//        res[i] = NOT_SET;
//    }
//    return res;
//}
//
//void free_array_of_int(int *arr) {
//    free(arr);
//}

//-----------------------------------------------------------------------------
//                                heuristic
//-----------------------------------------------------------------------------

/* return true if num is in arr and false otherwise */
bool is_in(int num, int *arr, int size) {
    
    for (int i=0 ; i<size ; i++) {
        if (num == arr[i])
            return true;
    }
    return false;
}

/* for a given cost of a node, caculate the lower bound of the path using the
 * lightest edge of each vertex that wehere not visited.
 * (the path may not exist but this will be out lower bound) */
int heuristic(State *state, int citiesNum, int **agencyMatrix) {

    int res = state->cost;

    /* go over all not-visited vertex */
    for (int i=0 ; i<citiesNum ; i++) {

        if (is_in(i, state->shortestPathUntilNow, citiesNum) && 
            i != state->shortestPathUntilNow[0])
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

//FIXME:remove
//int heuristic(State *state, int citiesNum, int **agencyMatrix) {
//
//    int res = state->cost;
//
//    /* go over all not-visited vertex */
//    for (int i=0 ; i<citiesNum ; i++) {
//
//        if (state->visitedNodesUntilNow[i] && i!=state->shortestPathUntilNow[0])
//            continue;
//
//        /* find the lightest edge of vertex i */
//        int lightestEdge = INF;
//        for (int j=0 ; j<citiesNum ; j++) {
//            if (agencyMatrix[i][j] < lightestEdge) {
//                lightestEdge = agencyMatrix[i][j];
//            }
//        }
//        res += lightestEdge;
//    }
//    return res;
//}




//-----------------------------------------------------------------------------
//                              CPU main
//-----------------------------------------------------------------------------

/* return true if state is a leaf in the search tree and false otherwise */
bool is_leaf(State *state, int citiesNum) {

    for (int i=0 ; i<citiesNum ; i++) {
        if (state->shortestPathUntilNow[i] == NOT_SET)
            return false;
    }
    return true;
}

//FIXME:remove
//bool is_leaf(State *state, int citiesNum) {
//    for (int i=0 ; i<citiesNum ; i++) {
//        if ( !state->visitedNodesUntilNow[i] )
//            return false;
//    }
//    return true;
//}
//
//


/* the main function runned by a single CPU
 * returns:
 * the minimum lenght as a return value
 * the shortest path in shortestPath variable */
int cpu_main(State *state, int citiesNum, int **agencyMatrix, int *shortestPath) {

    if (is_leaf(state, citiesNum)) {
        int rootVertex = state->shortestPathUntilNow[0];
        int lastVertex = state->shortestPathUntilNow[citiesNum-1];
        int costBackToRoot = agencyMatrix[rootVertex][lastVertex];
        copy_array_of_int(state->shortestPathUntilNow, shortestPath, citiesNum);
        return state->cost + costBackToRoot;
    }

    int minPathLen = INF;
    int *bestShortestPath = allocate_empty_array_of_int(citiesNum);

    /* check all the possible son's */
    for (int i=0 ; i<citiesNum ; i++) {

        if (is_in(i, state->shortestPathUntilNow, citiesNum))
            continue;

        /* create shortestPathUntilNow array */
        int *shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
        assert(shortestPathUntilNow);
        copy_array_of_int(state->shortestPathUntilNow, shortestPathUntilNow,
                citiesNum);
        for (int k=0 ; k<citiesNum ; k++) {
            if(state->shortestPathUntilNow[k] == NOT_SET) {
                shortestPathUntilNow[k] = i;
                break;
            }
        }

        /* create the Son state */
        State *son = create_array_of_state(1, citiesNum, i, shortestPathUntilNow,
                agencyMatrix);
        assert(son);

        /* prune branches if there is no chance to obtain shortest path from 
         * this son */
        if (heuristic(son, citiesNum, agencyMatrix) > minPathLen) {
            free_array_of_states(son);
            continue;
        }

        /* compute the result for this son */
        int res = cpu_main(son, citiesNum, agencyMatrix, shortestPath);

        if (res < minPathLen) {
            minPathLen = res;
            copy_array_of_int(shortestPath, bestShortestPath, citiesNum);
        }

        free_array_of_states(son);
    }
        
    free_array_of_int(bestShortestPath);
    return minPathLen;
}
///* the main function runned by a single CPU
// * returns:
// * the minimum lenght as a return value
// * the shortest path in shortestPath variable */
//int cpu_main(State *state, int citiesNum, int **agencyMatrix,
//        int *shortestPath) {
//
//    if (is_leaf(state, citiesNum)) {
//        int rootVertex = state->shortestPathUntilNow[0];
//        int lastVertex = state->shortestPathUntilNow[citiesNum-1];
//        int costBackToRoot = agencyMatrix[rootVertex][lastVertex];
//        /* insert the shortest path result to the state */
//        copy_array_of_int(state->shortestPathUntilNow, state->shortestPathRes,
//                citiesNum);
//        copy_array_of_int(state->shortestPathRes, shortestPath, citiesNum);
//        return state->cost + costBackToRoot;
//    }
//
//    int minPathLen = INF;
//    State *bestSon = NULL;
//
//    /* check all the possible son's */
//    for (int i=0 ; i<citiesNum ; i++) {
//
//        if (state->visitedNodesUntilNow[i])
//            continue;
//
//        /* create shortestPathUntilNow array */
//        int *shortestPathUntilNow = malloc(citiesNum * sizeof(int));
//        if ( !shortestPathUntilNow ) {
//            printf("cannot allocate shortestPathUntilNow\n");
//            return ERROR;
//        }
//        bool updated = false;
//        for (int k=0 ; k<citiesNum ; k++) {
//            if(state->shortestPathUntilNow[k] == NOT_SET && !updated) {
//                shortestPathUntilNow[k] = i;
//                updated = true;
//            } else {
//                shortestPathUntilNow[k] = state->shortestPathUntilNow[k];
//            }
//        }
//
//        /* create visitedNodesUntilNow array */
//        int *visitedNodesUntilNow = malloc(citiesNum * sizeof(int));
//        if ( !visitedNodesUntilNow ) {
//            printf("cannot allocate visitedNodesUntilNow\n");
//            return ERROR;
//        }
//        for (int k=0 ; k<citiesNum ; k++) {
//            visitedNodesUntilNow[k] = state->visitedNodesUntilNow[k];
//        }
//        visitedNodesUntilNow[i] = 1;
//
//        /* create the Son state */
//        State *son = create_state(i, shortestPathUntilNow, visitedNodesUntilNow,
//                citiesNum, agencyMatrix);
//        if ( !son ) {
//            printf("cannot allocate son\n");
//            return ERROR;
//        }
//
//        /* prune branches if there is no chance to obtain shortest path from 
//         * this son */
//        if (heuristic(son, citiesNum, agencyMatrix) > minPathLen) {
//            free_state(son);
//            continue;
//        }
//
//        /* compute the result for this son */
//        int res = cpu_main(son, citiesNum, agencyMatrix, shortestPath);
//        if (res == ERROR) {
//            printf("ERROR returned from cpu_main()\n");
//            return ERROR;
//        }
//
//        if (res < minPathLen) {
//            minPathLen = res;
//            free_state(bestSon);
//            bestSon = son;
//        }
//
//        if (son != bestSon)
//            free_state(son);
//    }
//        
//    copy_array_of_int(bestSon->shortestPathRes, state->shortestPathRes,
//            citiesNum);
//    copy_array_of_int(state->shortestPathRes, shortestPath, citiesNum);
//    free_state(bestSon);
//    return minPathLen;
//}



//-----------------------------------------------------------------------------
//                                    MPI
//-----------------------------------------------------------------------------

///* register State struct into mpi library */
//void build_state_type(State *state, int citiesNum, MPI_Datatype *newTypeName) {
//    
//    int blockLengths[5] = {1, 1, citiesNum, citiesNum, citiesNum};
//    MPI_Aint displacements[5];
//    MPI_Datatype typelist[5] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};
//    MPI_Aint addresses[6]; //helper array
//
//    /* compute displacements */
//    MPI_Get_address(state, &addresses[0]);
//    MPI_Get_address(&(state->vertex), &addresses[1]);
//    MPI_Get_address(&(state->cost), &addresses[2]);
//    MPI_Get_address(state->shortestPathUntilNow, &addresses[3]);
//    MPI_Get_address(state->visitedNodesUntilNow, &addresses[4]);
//    MPI_Get_address(state->shortestPathRes, &addresses[5]);
//
//    displacements[0] = addresses[1] - addresses[0];
//    displacements[1] = addresses[2] - addresses[0];
//    displacements[2] = addresses[3] - addresses[0];
//    displacements[3] = addresses[4] - addresses[0];
//    displacements[4] = addresses[5] - addresses[0];
//
//    /* create the derived type */
//    MPI_Type_create_struct(5, blockLengths, displacements, typelist, newTypeName);
//
//    /* commit the new type to mpi library */
//    MPI_Type_commit(newTypeName);
//}
//
/* allocate the memory for statesArr and initiallize it to NOT_SET values */
//State* allocate_array_of_states(int size) {
//    State *res = malloc(size * sizeof(State));
//    assert(res);
//    for (int i=0 ; i<size ; i++) {
//        res[i].vertex = NOT_SET;
//        res[i].cost = NOT_SET;
//        res[i].shortestPathUntilNow = allocate_empty_array_of_int(size);
//        res[i].visitedNodesUntilNow = allocate_empty_array_of_int(size);
//        res[i].shortestPathRes = allocate_empty_array_of_int(size);
//    }
//    return res;
//}
//
//void free_array_of_states(State *statesArr, int size) {
//    for (int i=0 ; i<size ; i++) {
//        free_array_of_int(statesArr[i].shortestPathUntilNow);
//        free_array_of_int(statesArr[i].visitedNodesUntilNow);
//        free_array_of_int(statesArr[i].shortestPathRes);
//    }
//    free(statesArr);
//}
//

/* initiallize the array of states to be sent to other tasks 
 * NOTE:
 * arrRes is the result and must be initiallized with all vertex = NOT_SET */
void initiallize_array_of_states(int size, int citiesNum, State *state,
        int depth, int maxDepth, int **agencyMatrix, State *arrRes) {
//void initiallize_array_of_states(int **agencyMatrix, State *state, int citiesNum,
//        int depth, int maxDepth, int size, State *arrRes) {

    /* if we have reached required depth insert states to arrRes */
    if (depth == maxDepth) {
        for (int i=0 ; i<size ; i++) {
            if (arrRes[i].vertex == NOT_SET) {
                printf("before : arrRes[%d] = {%d, {", i, arrRes[i].vertex);
                for (int k=0 ; k<citiesNum ; k++) {
                    if (k == citiesNum-1)
                        printf("%d", arrRes[i].shortestPathUntilNow[k]);
                    else
                        printf("%d, ", arrRes[i].shortestPathUntilNow[k]);
                }
                printf("}\n");
                copy_array_of_states(state, &arrRes[i], /*size=*/1, citiesNum);
                //copy_state(state, &arrRes[i], citiesNum);
                //printf("inserted vertex %d to index %d\n", state->vertex, i);
                return;
            }
        }
    }

    for (int i=0 ; i<citiesNum ; i++) {

        if (is_in(i, state->shortestPathUntilNow, citiesNum))
        //if (state->visitedNodesUntilNow[i])
            continue;

        /* create shortestPathUntilNow array */
        int *shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
        //int *shortestPathUntilNow = malloc(citiesNum * sizeof(int));
        assert(shortestPathUntilNow);
        copy_array_of_int(state->shortestPathUntilNow, shortestPathUntilNow,
                citiesNum);
        for (int k=0 ; k<citiesNum ; k++) {
            if(state->shortestPathUntilNow[k] == NOT_SET) {
                shortestPathUntilNow[k] = i;
                break;
            }
        }
        //bool updated = false;
        //for (int k=0 ; k<citiesNum ; k++) {
        //    if(state->shortestPathUntilNow[k] == NOT_SET && !updated) {
        //        shortestPathUntilNow[k] = i;
        //        updated = true;
        //    } else {
        //        shortestPathUntilNow[k] = state->shortestPathUntilNow[k];
        //    }
        //}

        ///* create visitedNodesUntilNow array */
        //int *visitedNodesUntilNow = malloc(citiesNum * sizeof(int));
        //assert(visitedNodesUntilNow);
        //for (int k=0 ; k<citiesNum ; k++) {
        //    visitedNodesUntilNow[k] = state->visitedNodesUntilNow[k];
        //}
        //visitedNodesUntilNow[i] = 1;

        /* create the Son state */
        State *son = create_array_of_state(1, citiesNum, i, shortestPathUntilNow,
                agencyMatrix);
        //State *son = create_state(i, shortestPathUntilNow, visitedNodesUntilNow,
        //        citiesNum, agencyMatrix);
        assert(son);

        initiallize_array_of_states(size, citiesNum, son, depth+1,
                maxDepth, agencyMatrix, arrRes);

        free_array_of_states(son);
        //free_state(son);
    }
}
///* initiallize the array of states array of states to be sent to other tasks */
//void initiallize_array_of_states(int **agencyMatrix, State *state, int citiesNum,
//        int depth, int maxDepth, int arrSize, State *arrRes) {
//
//    /* if we have reached required depth insert states to arrRes */
//    if (depth == maxDepth) {
//        for (int i=0 ; i<arrSize ; i++) {
//            if (arrRes[i].vertex == NOT_SET) {
//                copy_state(state, &arrRes[i], citiesNum);
//                return;
//            }
//        }
//    }
//
//    for (int i=0 ; i<citiesNum ; i++) {
//
//        if (state->visitedNodesUntilNow[i])
//            continue;
//
//        /* create shortestPathUntilNow array */
//        int *shortestPathUntilNow = malloc(citiesNum * sizeof(int));
//        assert(shortestPathUntilNow);
//        bool updated = false;
//        for (int k=0 ; k<citiesNum ; k++) {
//            if(state->shortestPathUntilNow[k] == NOT_SET && !updated) {
//                shortestPathUntilNow[k] = i;
//                updated = true;
//            } else {
//                shortestPathUntilNow[k] = state->shortestPathUntilNow[k];
//            }
//        }
//
//        /* create visitedNodesUntilNow array */
//        int *visitedNodesUntilNow = malloc(citiesNum * sizeof(int));
//        assert(visitedNodesUntilNow);
//        for (int k=0 ; k<citiesNum ; k++) {
//            visitedNodesUntilNow[k] = state->visitedNodesUntilNow[k];
//        }
//        visitedNodesUntilNow[i] = 1;
//
//        /* create the Son state */
//        State *son = create_state(i, shortestPathUntilNow, visitedNodesUntilNow,
//                citiesNum, agencyMatrix);
//        assert(son);
//
//        initiallize_array_of_states(agencyMatrix, son, citiesNum, depth+1,
//                maxDepth, arrSize, arrRes);
//
//        free_state(son);
//    }
//}
////FIXME: remove
////void initiallize_array_of_states(int **agencyMatrix, State *state, int citiesNum,
////        int depth, int maxDepth, int arrSize, State **arrRes) {
////
////    /* if we have reached required depth insert states to arrRes */
////    if (depth == maxDepth) {
////        for (int i=0 ; i<arrSize ; i++) {
////            if (arrRes[i] == NULL) {
////                arrRes[i] = state;
////                return;
////            }
////        }
////    }
////
////    for (int i=0 ; i<citiesNum ; i++) {
////
////        if (state->visitedNodesUntilNow[i])
////            continue;
////
////        /* create shortestPathUntilNow array */
////        int *shortestPathUntilNow = malloc(citiesNum * sizeof(int));
////        assert(shortestPathUntilNow);
////        bool updated = false;
////        for (int k=0 ; k<citiesNum ; k++) {
////            if(state->shortestPathUntilNow[k] == NOT_SET && !updated) {
////                shortestPathUntilNow[k] = i;
////                updated = true;
////            } else {
////                shortestPathUntilNow[k] = state->shortestPathUntilNow[k];
////            }
////        }
////
////        /* create visitedNodesUntilNow array */
////        int *visitedNodesUntilNow = malloc(citiesNum * sizeof(int));
////        assert(visitedNodesUntilNow);
////        for (int k=0 ; k<citiesNum ; k++) {
////            visitedNodesUntilNow[k] = state->visitedNodesUntilNow[k];
////        }
////        visitedNodesUntilNow[i] = 1;
////
////        /* create the Son state */
////        State *son = create_state(i, shortestPathUntilNow, visitedNodesUntilNow,
////                citiesNum, agencyMatrix);
////        assert(son);
////
////        initiallize_array_of_states(agencyMatrix, son, citiesNum, depth+1, maxDepth,
////                arrSize, arrRes);
////    }
////    free_state(state);
////}
//
////void free_array_of_states(State **statesArr, int size) {
////    for (int i=0 ; i<size ; i++) {
////        free_state(statesArr[i]);
////    }
////    free(statesArr);
////}
//
///* send citiesNum, xCoord and yCoord from root to all other tasks */
//int send_initial_data(int citiesNum, int *xCoord, int *yCoord) {
//
//    int numTasks, rc;
//    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
//
//    int bsendBuffSize = numTasks*((2*citiesNum+1)*sizeof(int))+MPI_BSEND_OVERHEAD;
//    //int bsendBuffSize = numTasks * (sizeof(int) + MPI_BSEND_OVERHEAD);
//    int *bsendBuff = malloc(bsendBuffSize);
//    assert(bsendBuff);
//    MPI_Buffer_attach(bsendBuff, bsendBuffSize);
//
//    for (int i=1 ; i<numTasks ; i++) {
//        rc = MPI_Bsend(&citiesNum, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
//        assert(rc == MPI_SUCCESS);
//        rc = MPI_Bsend(xCoord, citiesNum, MPI_INT, i, TAG, MPI_COMM_WORLD);
//        assert(rc == MPI_SUCCESS);
//        rc = MPI_Bsend(yCoord, citiesNum, MPI_INT, i, TAG, MPI_COMM_WORLD);
//        assert(rc == MPI_SUCCESS);
//    }
//    free(bsendBuff);
//    return MPI_SUCCESS;
//}
//
///* receive citiesNum, xCoord and yCoord from root 
// * NOTE:
// * citiesNum, xCoord and yCoord are only variable names and we assume they
// * will containe the correct values only after this function as requested 
// * in the PDF exercise */
//int receive_initial_data(int *citiesNum, int *xCoord, int *yCoord) {
//
//    MPI_Status status;
//    int rank, rc1, rc2, rc3;
//    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//
//    rc1 = MPI_Recv(citiesNum, 1, MPI_INT, /*src=*/0, TAG, MPI_COMM_WORLD,
//            &status);
//    rc2 = MPI_Recv(xCoord, *citiesNum, MPI_INT, /*src=*/0, TAG, MPI_COMM_WORLD,
//            &status);
//    rc3 = MPI_Recv(yCoord, *citiesNum, MPI_INT, /*src=*/0, TAG, MPI_COMM_WORLD,
//            &status);
//    //printf("cpu %d : received citiesNum\n", rank);
//    //printf("cpu %d : received xCoord[]\n", rank);
//    //printf("cpu %d : received yCoord[]\n", rank);
//
//    assert(rc1 == MPI_SUCCESS && rc2 == MPI_SUCCESS && rc3 == MPI_SUCCESS);
//    return MPI_SUCCESS;
//}
//
///* send the data to other tasks - rank{1, 2,...numTasks}.
// * NOTE:
// * this fuction send all it gots, it is not aware that a part of the Arr
// * stays for root task */
//int send_data(State *dataArr, int size, MPI_Datatype stateTypeName) {
//
//    /* get num of tasks */
//    int numTasks;
//    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
//
//    //FIXME: less overhead?
//    /* allocate buffers for MPI_Bsend */
//    int bsendBuffSize = size * (sizeof(int) + MPI_BSEND_OVERHEAD);
//    int *bsendBuff = malloc(bsendBuffSize);
//    assert(bsendBuff);
//
//    /* attach buffer for MPI_Bsend() */
//    MPI_Buffer_attach(bsendBuff, bsendBuffSize);
//
//    int minSendSize = size / (numTasks-1);
//    int tasksNumToSendMore = size % (numTasks-1);
//    printf("root : numStates to send = %d\n", size);
//    printf("root : minSendSize = %d\n", minSendSize);
//    printf("root : tasksNumToSendMore = %d\n", tasksNumToSendMore);
//
//    for (int i=1, offset=0 ; i<numTasks ; i++) {
//        if(i == 1) {
//            if (i <= tasksNumToSendMore) {
//                MPI_Bsend(dataArr+offset, minSendSize+1, stateTypeName, i,
//                        TAG, MPI_COMM_WORLD);
//                printf("root : sendig %d states to task_%d\n", (minSendSize+1), i);
//                offset += minSendSize+1;
//            } else {
//                MPI_Bsend(dataArr+offset, minSendSize, stateTypeName, i,
//                        TAG, MPI_COMM_WORLD);
//                printf("root : sendig %d states to task_%d\n", minSendSize, i);
//                offset += minSendSize;
//            }
//        }
//    }
//    //for (int i=1, offset=0 ; i<numTasks ; i++) {
//    //    if (i <= tasksNumToSendMore) {
//    //        MPI_Bsend(dataArr+offset, minSendSize+1, stateTypeName, i,
//    //                TAG, MPI_COMM_WORLD);
//    //        printf("root : sendig %d states to task_%d\n", (minSendSize+1), i);
//    //        offset += minSendSize+1;
//    //    } else {
//    //        MPI_Bsend(dataArr+offset, minSendSize, stateTypeName, i,
//    //                TAG, MPI_COMM_WORLD);
//    //        printf("root : sendig %d states to task_%d\n", minSendSize, i);
//    //        offset += minSendSize;
//    //    }
//    //}
//    ////FIXME: send States insead of this
//    //int msg[] = {1, 2, 3};
//    //for (int i=1 ; i<numTasks ; i++) {
//    //    if (i <= tasksNumToSendMore) {
//    //        printf("root : sendig %d elements to task_%d\n", (minSendSize+1), i);
//    //        MPI_Bsend(&msg, minSendSize+1, MPI_INT, i, TAG, MPI_COMM_WORLD);
//    //    } else {
//    //        printf("root : sendig %d elements to task_%d\n", minSendSize, i);
//    //        MPI_Bsend(&msg, minSendSize, MPI_INT, i, TAG, MPI_COMM_WORLD);
//    //    }
//    //}
//
//    free(bsendBuff);
//    return MPI_SUCCESS;
//}
//
//
//
///* the root task send all the data to other tasks, wait to all the data to be
// * received on other task, compute a sub problem itself, gather the results
// * of all other tasks and return the best result */
//void rootExec(int citiesNum, int **agencyMatrix, MPI_Datatype stateTypeName,
//        int *shortestPath) {
//
//    /* compute how deep the root task need to go in order to have enought 
//     * States for all CPUs */
//    int maxDepth = 1;
//    int citiesNumLeft = citiesNum-1;
//    int numStates = citiesNumLeft;
//    int numTasks;
//    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
//    while (numStates < numTasks) {
//        maxDepth++;
//        citiesNumLeft--;
//        numStates *= citiesNumLeft;
//    }
//    printf("root : #CPUs = %d\nroot : numStates = %d\n", numTasks, numStates);
//    printf("root : maxDepth = %d\n", maxDepth);
//
//    /* create rootState for initiallizing statesArr */
//    int *shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
//    int *visitedNodesUntilNow = allocate_empty_array_of_int(citiesNum);
//    assert(shortestPathUntilNow && visitedNodesUntilNow);
//    shortestPathUntilNow[0] = 0;
//    visitedNodesUntilNow[0] = 1;
//    State *rootState = create_state(0, shortestPathUntilNow,
//            visitedNodesUntilNow, citiesNum, agencyMatrix);
//
//    State *statesArr = allocate_array_of_states(numStates);
//    initiallize_array_of_states(agencyMatrix, rootState, citiesNum, 0, maxDepth,
//            numStates, statesArr);
//    free_state(rootState);
//    printf("root : created statesArr\n");
//
//    /* send the data to other tasks:
//     * statesArr[0:numStatesForRootTask-1] --> root states
//     * statesArr[numStatesForRootTask, numTasks-1] --> distributed tasks */
//    int numStatesForRootTask = numStates / numTasks;
//    int rc = send_data(statesArr+numStatesForRootTask,
//            numStates-numStatesForRootTask, stateTypeName);
//    assert(rc == MPI_SUCCESS);
//    printf("root : all data has been send\n");
//
//    /* wait for all tasks to receive the data */
//    MPI_Barrier(MPI_COMM_WORLD);
//    printf("root : continue after barrier\n");
//
//    /////* compute a sub problem like all other tasks */
//    ////int minPathLen = INF;
//    ////int *shortestPathLocal = malloc(citiesNum * sizeof(int));
//    ////assert(shortestPathLocal);
//    ////for (int i=0 ; i<numStatesForRootTask ; i++) {
//    ////    res = cpu_main(statesArr[i], citiesNum, agencyMatrix, shortestPathLocal);
//    ////    if (res < minPathLen) {
//    ////        minPathLen = res;
//    ////        copy_array_of_int(shortestPathLocal, shortestPath, citiesNum)
//    ////    }
//    ////}
//
//    /////* gather the result from all other tasks */
//    ////MPI_Gather(NULL, 1, stateTypeName, res...)
//
//    ///* free memory */
//    //free_array_of_states(statesArr, numStates);
//
//    /////* now shortestPath hold the best value best result */
//}
//
///* receive the data from the root task, compute a sub problem and return the
// * best local result to root task */
//void otherExec(int citiesNum, int **agencyMatrix, MPI_Datatype stateTypeName,
//        int *shortestPath) {
//
//    int rank, count;
//    State *recvBuff = NULL;
//    //int *recvBuff = NULL;
//    MPI_Status status;
//    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//
//    if (rank == 1) {
//        /* reciver message size */
//        int rc1 = MPI_Probe(/*src=*/0, TAG, MPI_COMM_WORLD, &status);
//        int rc2 = MPI_Get_count(&status, stateTypeName, &count);
//        //int rc2 = MPI_Get_count(&status, MPI_INT, &count);
//        assert(rc1 == MPI_SUCCESS && rc2 == MPI_SUCCESS);
//        printf("cpu %d : received %d messages from root\n", rank, count);
//
//        /* allocate receive buffer and receive the data from root task */
//        recvBuff = allocate_array_of_states(count);
//        ////recvBuff = malloc(count * sizeof(int));
//        assert(recvBuff);
//        //MPI_Recv(recvBuff, count, stateTypeName, /*src=*/0, TAG, MPI_COMM_WORLD, &status);
//        ////MPI_Recv(recvBuff, count, MPI_INT, /*src=*/0, TAG, MPI_COMM_WORLD, &status);
//        ////printf("cpu %d : received message [%d]\n", rank, recvBuff[0]);
//        ////printf("cpu %d : received message [%d]\n", rank, recvBuff[1]);
//        ////if (rank >= 1 && rank <= 4) {
//        ////    printf("cpu %d : received message [%d]\n", rank, recvBuff[2]);
//        ////}
//    }
//
//    /* wait for all tasks to receive the data */
//    MPI_Barrier(MPI_COMM_WORLD);
//    printf("cpu %d : continue after barrier\n", rank);
//
//    ///* compute a sub problems and keep the best one */
//
//    ///* send the result to root task */
//
//}
//
//
//// The static parellel algorithm main function.
//int tsp_main(int citiesNum, int xCoord[], int yCoord[], int shortestPath[])
//{
//    int numTasks, rank, rc;
//    MPI_Datatype stateTypeName;
//
//    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
//    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//
//    /* transmit initiall data (citiesNum, xCoord, yCoord) */
//    if (rank == 0) {
//        rc = send_initial_data(citiesNum, xCoord, yCoord);
//        assert(rc == MPI_SUCCESS);
//    } else {
//        rc = receive_initial_data(&citiesNum, xCoord, yCoord);
//        assert(rc == MPI_SUCCESS);
//    }
//
//    /* create agencyMatrix */
//    int **agencyMatrix = create_agency_matrix(xCoord, yCoord, citiesNum);
//
//    /* create rootState for registering State struct to mpi library */
//    int *shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
//    int *visitedNodesUntilNow = allocate_empty_array_of_int(citiesNum);
//    assert(shortestPathUntilNow && visitedNodesUntilNow);
//    shortestPathUntilNow[0] = 0;
//    visitedNodesUntilNow[0] = 1;
//    State *rootState = create_state(0, shortestPathUntilNow,
//            visitedNodesUntilNow, citiesNum, agencyMatrix);
//
//    /* register State struct to MPI library */
//    build_state_type(rootState, citiesNum, &stateTypeName);
//
//    /* free rootState */
//    free_state(rootState);
//
//    if (rank == 0) {
//        rootExec(citiesNum, agencyMatrix, stateTypeName, shortestPath);
//    } else {
//        otherExec(citiesNum, agencyMatrix, stateTypeName, shortestPath);
//    }
//
//
//	return -1;	//TODO
//}



//-----------------------------------------------------------------------------
//                                  tests
//                              FIXME:remove
//-----------------------------------------------------------------------------

//FIXME: for testing
int main() {

    int xCoord[3] = {1, 1, 1};
    int yCoord[3] = {1, 2, 4};

    /* create matrix test */
    int citiesNum = 3;
    int **agencyMatrix = create_agency_matrix(xCoord, yCoord, citiesNum);
    assert(agencyMatrix[0][0] == INF);
    assert(agencyMatrix[0][1] == 3);
    assert(agencyMatrix[0][2] == 7);
    assert(agencyMatrix[1][0] == 3);
    assert(agencyMatrix[1][1] == INF);
    assert(agencyMatrix[1][2] == 5);
    assert(agencyMatrix[2][0] == 7);
    assert(agencyMatrix[2][1] == 5);
    assert(agencyMatrix[2][2] == INF);

    /* create_array_of_state test */
    int *shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow[0] = 1;
    /* {1, NOT_SET, NOT_SET} */
    State *states = create_array_of_state(1, citiesNum, 1, shortestPathUntilNow, agencyMatrix);
    assert(states[0].vertex == 1);
    assert(states[0].cost == 0);
    assert(states[0].shortestPathUntilNow[0] == 1);
    assert(states[0].shortestPathUntilNow[1] == NOT_SET);
    assert(states[0].shortestPathUntilNow[2] == NOT_SET);
    free_array_of_states(states);

    shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow[0] = 1;
    shortestPathUntilNow[1] = 2;
    /* {1, 2, NOT_SET} */
    states = create_array_of_state(1, citiesNum, 2, shortestPathUntilNow, agencyMatrix);
    assert(states[0].vertex == 2);
    assert(states[0].cost == 5);
    free_array_of_states(states);

    shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow[0] = 1;
    shortestPathUntilNow[1] = 2;
    shortestPathUntilNow[2] = 0;
    /* {1, 2, 0} */
    states = create_array_of_state(1, citiesNum, 0, shortestPathUntilNow, agencyMatrix);
    assert(states[0].vertex == 0);
    assert(states[0].cost == 12);
    free_array_of_states(states);
    free_agency_matrix(agencyMatrix, citiesNum);

    /* heuristic test */
    citiesNum = 3;
    agencyMatrix = create_agency_matrix(xCoord, yCoord, citiesNum);
    int *shortestPathUntilNow20 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow20[0] = 0;
    /* {0, NOT_SET, NOT_SET} */
    State *state20 = create_array_of_state(1, citiesNum, 0, shortestPathUntilNow20,
            agencyMatrix);
    assert(state20[0].cost == 0);
    assert(heuristic(state20, citiesNum, agencyMatrix) == 11);
    free_array_of_states(state20);

    shortestPathUntilNow20 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow20[0] = 0;
    shortestPathUntilNow20[1] = 2;
    /* {0, 2, NOT_SET} */
    state20 = create_array_of_state(1, citiesNum, 2, shortestPathUntilNow20, agencyMatrix);
    assert(state20[0].cost == 7);
    assert(heuristic(state20, citiesNum, agencyMatrix) == 13);
    free_array_of_states(state20);

    shortestPathUntilNow20 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow20[0] = 0;
    shortestPathUntilNow20[1] = 2;
    shortestPathUntilNow20[2] = 1;
    /* {0, 2, 1} */
    state20 = create_array_of_state(1, citiesNum, 1, shortestPathUntilNow20, agencyMatrix);
    assert(state20->cost == 12);
    assert(heuristic(state20, citiesNum, agencyMatrix) == 15);
    free_array_of_states(state20);

    shortestPathUntilNow20 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow20[0] = 0;
    shortestPathUntilNow20[1] = 1;
    /* {0, 1, NOT_SET} */
    state20 = create_array_of_state(1, citiesNum, 1, shortestPathUntilNow20, agencyMatrix);
    assert(state20->cost == 3);
    assert(heuristic(state20, citiesNum, agencyMatrix) == 11);
    free_array_of_states(state20);

    shortestPathUntilNow20 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow20[0] = 0;
    shortestPathUntilNow20[1] = 1;
    shortestPathUntilNow20[2] = 2;
    /* {0, 1, 2} */
    state20 = create_array_of_state(1, citiesNum, 2, shortestPathUntilNow20, agencyMatrix);
    assert(state20->cost == 8);
    assert(heuristic(state20, citiesNum, agencyMatrix) == 11);
    free_array_of_states(state20);
    free_agency_matrix(agencyMatrix, citiesNum);


    /* is_leaf test */
    citiesNum = 3;
    agencyMatrix = create_agency_matrix(xCoord, yCoord, citiesNum);
    int *shortestPathUntilNow2 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow2[0] = 1;
    /* {1, NOT_SET, NOT_SET} */
    State *state2 = create_array_of_state(1, citiesNum, 1, shortestPathUntilNow2, agencyMatrix);
    assert(!is_leaf(state2, citiesNum));
    free_array_of_states(state2);

    shortestPathUntilNow2 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow2[0] = 1;
    shortestPathUntilNow2[1] = 2;
    state2 = create_array_of_state(1, citiesNum, 2, shortestPathUntilNow2, agencyMatrix);
    assert(!is_leaf(state2, citiesNum));
    free_array_of_states(state2);

    shortestPathUntilNow2 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow2[0] = 1;
    shortestPathUntilNow2[1] = 2;
    shortestPathUntilNow2[2] = 0;
    state2 = create_array_of_state(1, citiesNum, 0, shortestPathUntilNow2, agencyMatrix);
    assert(is_leaf(state2, citiesNum));
    free_array_of_states(state2);
    free_agency_matrix(agencyMatrix, citiesNum);


    /* cpu_main test */
    int xCoord2[] = {0, 1, 1, 0};
    int yCoord2[] = {0, 0, 1, 1};
    citiesNum = 4;
    int shortestPath[4];
    agencyMatrix = create_agency_matrix(xCoord2, yCoord2, citiesNum);
    
//---------------------------------------------------------------- len 4

    int *shortestPathUntilNow3 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow3[0] = 0;
    shortestPathUntilNow3[1] = 1;
    shortestPathUntilNow3[2] = 2;
    shortestPathUntilNow3[3] = 3;
    /* {0,1,2,3} */
    State *state3 = create_array_of_state(1, citiesNum, 3, shortestPathUntilNow3,
            agencyMatrix);   
    assert(cpu_main(state3, citiesNum, agencyMatrix, shortestPath) == 12);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 1);
    assert(shortestPath[2] == 2);
    assert(shortestPath[3] == 3);
    free_array_of_states(state3);

    int *shortestPathUntilNow4 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow4[0] = 0;
    shortestPathUntilNow4[1] = 1;
    shortestPathUntilNow4[2] = 3;
    shortestPathUntilNow4[3] = 2;
    /* {0,1,3,2} */
    State *state4 = create_array_of_state(1, citiesNum, 2, shortestPathUntilNow4,
            agencyMatrix);   
    assert(cpu_main(state4, citiesNum, agencyMatrix, shortestPath) == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 1);
    assert(shortestPath[2] == 3);
    assert(shortestPath[3] == 2);
    free_array_of_states(state4);

    int *shortestPathUntilNow5 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow5[0] = 0;
    shortestPathUntilNow5[1] = 2;
    shortestPathUntilNow5[2] = 1;
    shortestPathUntilNow5[3] = 3;
    /* {0,2,1,3} */
    State *state5 = create_array_of_state(1, citiesNum, 3, shortestPathUntilNow5,
            agencyMatrix);   
    assert(cpu_main(state5, citiesNum, agencyMatrix, shortestPath) == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 2);
    assert(shortestPath[2] == 1);
    assert(shortestPath[3] == 3);
    free_array_of_states(state5);

    int *shortestPathUntilNow6 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow6[0] = 0;
    shortestPathUntilNow6[1] = 2;
    shortestPathUntilNow6[2] = 3;
    shortestPathUntilNow6[3] = 1;
    /* {0,2,3,1} */
    State *state6 = create_array_of_state(1, citiesNum, 1, shortestPathUntilNow6,
            agencyMatrix);   
    assert(cpu_main(state6, citiesNum, agencyMatrix, shortestPath) == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 2);
    assert(shortestPath[2] == 3);
    assert(shortestPath[3] == 1);
    free_array_of_states(state6);

    int *shortestPathUntilNow7 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow7[0] = 0;
    shortestPathUntilNow7[1] = 3;
    shortestPathUntilNow7[2] = 1;
    shortestPathUntilNow7[3] = 2;
    /* {0,3,1,2} */
    State *state7 = create_array_of_state(1, citiesNum, 2, shortestPathUntilNow7,
            agencyMatrix);   
    assert(cpu_main(state7, citiesNum, agencyMatrix, shortestPath) == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 3);
    assert(shortestPath[2] == 1);
    assert(shortestPath[3] == 2);
    free_array_of_states(state7);

    int *shortestPathUntilNow8 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow8[0] = 0;
    shortestPathUntilNow8[1] = 3;
    shortestPathUntilNow8[2] = 2;
    shortestPathUntilNow8[3] = 1;
    /* {0,3,2,1} */
    State *state8 = create_array_of_state(1, citiesNum, 1, shortestPathUntilNow8,
            agencyMatrix);   
    assert(cpu_main(state8, citiesNum, agencyMatrix, shortestPath) == 12);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 3);
    assert(shortestPath[2] == 2);
    assert(shortestPath[3] == 1);
    free_array_of_states(state8);

//---------------------------------------------------------------- len 3

    int *shortestPathUntilNow9 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow9[0] = 0;
    shortestPathUntilNow9[1] = 1;
    shortestPathUntilNow9[2] = 2;
    /* {0,1,2,NOT_SET} */
    State *state9 = create_array_of_state(1, citiesNum, 2, shortestPathUntilNow9,
            agencyMatrix);   
    assert(cpu_main(state9, citiesNum, agencyMatrix, shortestPath) == 12);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 1);
    assert(shortestPath[2] == 2);
    assert(shortestPath[3] == 3);
    free_array_of_states(state9);

    int *shortestPathUntilNow10 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow10[0] = 0;
    shortestPathUntilNow10[1] = 1;
    shortestPathUntilNow10[2] = 3;
    /* {0,1,3,NOT_SET} */
    State *state10 = create_array_of_state(1, citiesNum, 3, shortestPathUntilNow10,
            agencyMatrix);   
    assert(cpu_main(state10, citiesNum, agencyMatrix, shortestPath) == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 1);
    assert(shortestPath[2] == 3);
    assert(shortestPath[3] == 2);
    free_array_of_states(state10);

    int *shortestPathUntilNow11 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow11[0] = 0;
    shortestPathUntilNow11[1] = 2;
    shortestPathUntilNow11[2] = 1;
    /* {0,2,1,NOT_SET} */
    State *state11 = create_array_of_state(1, citiesNum, 1, shortestPathUntilNow11,
            agencyMatrix);   
    assert(cpu_main(state11, citiesNum, agencyMatrix, shortestPath) == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 2);
    assert(shortestPath[2] == 1);
    assert(shortestPath[3] == 3);
    free_array_of_states(state11);

    int *shortestPathUntilNow12 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow12[0] = 0;
    shortestPathUntilNow12[1] = 2;
    shortestPathUntilNow12[2] = 3;
    /* {0,2,3,NOT_SET} */
    State *state12 = create_array_of_state(1, citiesNum, 3, shortestPathUntilNow12,
            agencyMatrix);   
    assert(cpu_main(state12, citiesNum, agencyMatrix, shortestPath) == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 2);
    assert(shortestPath[2] == 3);
    assert(shortestPath[3] == 1);
    free_array_of_states(state12);

    int *shortestPathUntilNow13 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow13[0] = 0;
    shortestPathUntilNow13[1] = 3;
    shortestPathUntilNow13[2] = 1;
    /* {0,3,1,NOT_SET} */
    State *state13 = create_array_of_state(1, citiesNum, 1, shortestPathUntilNow13,
            agencyMatrix);   
    assert(cpu_main(state13, citiesNum, agencyMatrix, shortestPath) == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 3);
    assert(shortestPath[2] == 1);
    assert(shortestPath[3] == 2);
    free_array_of_states(state13);

    int *shortestPathUntilNow14 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow14[0] = 0;
    shortestPathUntilNow14[1] = 3;
    shortestPathUntilNow14[2] = 2;
    /* {0,3,2,NOT_SET} */
    State *state14 = create_array_of_state(1, citiesNum, 2, shortestPathUntilNow14, agencyMatrix);   
    assert(cpu_main(state14, citiesNum, agencyMatrix, shortestPath) == 12);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 3);
    assert(shortestPath[2] == 2);
    assert(shortestPath[3] == 1);
    free_array_of_states(state14);

//---------------------------------------------------------------- len 2

    int *shortestPathUntilNow15 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow15[0] = 0;
    shortestPathUntilNow15[1] = 1;
    /* {0,1,NOT_SET,NOT_SET} */
    State *state15 = create_array_of_state(1, citiesNum, 1, shortestPathUntilNow15,
            agencyMatrix);   
    assert(cpu_main(state15, citiesNum, agencyMatrix, shortestPath) == 12);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 1);
    assert(shortestPath[2] == 2);
    assert(shortestPath[3] == 3);
    free_array_of_states(state15);

    int *shortestPathUntilNow16 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow16[0] = 0;
    shortestPathUntilNow16[1] = 2;
    /* {0,2,NOT_SET,NOT_SET} */
    State *state16 = create_array_of_state(1, citiesNum, 2, shortestPathUntilNow16,
            agencyMatrix);   
    assert(cpu_main(state16, citiesNum, agencyMatrix, shortestPath) == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 2);
    assert((shortestPath[2] == 1 && shortestPath[3] == 3) ||
           (shortestPath[2] == 3 && shortestPath[3] == 1));
    free_array_of_states(state16);

    int *shortestPathUntilNow17 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow17[0] = 0;
    shortestPathUntilNow17[1] = 3;
    /* {0,3,NOT_SET,NOT_SET} */
    State *state17 = create_array_of_state(1, citiesNum, 3, shortestPathUntilNow17,
            agencyMatrix);   
    assert(cpu_main(state17, citiesNum, agencyMatrix, shortestPath) == 12);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 3);
    assert(shortestPath[2] == 2);
    assert(shortestPath[3] == 1);
    free_array_of_states(state17);

    int *shortestPathUntilNow18 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow18[0] = 0;
    /* {0,NOT_SET,NOT_SET,NOT_SET} */
    State *state18 = create_array_of_state(1, citiesNum, 0, shortestPathUntilNow18,
            agencyMatrix);   
    assert(cpu_main(state18, citiesNum, agencyMatrix, shortestPath) == 12);
    assert(shortestPath[0] == 0);
    assert((shortestPath[1]==1 && shortestPath[2]==2 && shortestPath[3]==3) ||
           (shortestPath[1]==3 && shortestPath[2]==2 && shortestPath[3]==1));
    free_array_of_states(state18);
    free_agency_matrix(agencyMatrix, citiesNum);






    int xCoord3[] = {0, 1, 1, 0, 2, 4};
    int yCoord3[] = {0, 0, 1, 1 ,0, 0};
    citiesNum = 6;
    int shortestPath3[6];
    agencyMatrix = create_agency_matrix(xCoord3, yCoord3, citiesNum);
    int *shortestPathUntilNow30 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow30[0] = 0;
    /* {0, NOT_SET, NOT_SET, NOT_SET, NOT_SET, NOT_SET} */
    int numStates = 20;
    int *emptyArr = allocate_empty_array_of_int(citiesNum);
    State *statesArr = create_array_of_state(numStates, citiesNum, NOT_SET,
            emptyArr, agencyMatrix);
    State *rootState = create_array_of_state(1, citiesNum, 0, 
            shortestPathUntilNow30, agencyMatrix);
    initiallize_array_of_states(numStates, citiesNum, rootState, 0, 2,
            agencyMatrix, statesArr);
    free_array_of_states(rootState);

    //for (int i=0 ; i<numStates ; i++) {
    //    printf("statesArr[%d] = %d\n", i, statesArr[i].vertex);
    //}


    //assert(statesArr[0].vertex == 2);
    //assert(statesArr[0].shortestPathUntilNow[0] == 0);
    //assert(statesArr[0].shortestPathUntilNow[1] == 1);
    //assert(statesArr[0].shortestPathUntilNow[2] == 2);
    //assert(statesArr[1].vertex == 3);
    //assert(statesArr[1].shortestPathUntilNow[0] == 0);
    //assert(statesArr[1].shortestPathUntilNow[1] == 1);
    //assert(statesArr[1].shortestPathUntilNow[2] == 3);
    //assert(statesArr[2].vertex == 4);
    //assert(statesArr[2].shortestPathUntilNow[0] == 0);
    //assert(statesArr[2].shortestPathUntilNow[1] == 1);
    //assert(statesArr[2].shortestPathUntilNow[2] == 4);
    //assert(statesArr[3].vertex == 5);
    //assert(statesArr[3].shortestPathUntilNow[0] == 0);
    //assert(statesArr[3].shortestPathUntilNow[1] == 1);
    //assert(statesArr[3].shortestPathUntilNow[2] == 5);
    //assert(statesArr[4].vertex == 1);
//    assert(statesArr[4].shortestPathUntilNow[0] == 0);
//    assert(statesArr[4].shortestPathUntilNow[1] == 2);
//    assert(statesArr[4].shortestPathUntilNow[2] == 1);
//    assert(statesArr[5].vertex == 3);
//    assert(statesArr[5].shortestPathUntilNow[0] == 0);
//    assert(statesArr[5].shortestPathUntilNow[1] == 2);
//    assert(statesArr[5].shortestPathUntilNow[2] == 3);
//    assert(statesArr[6].vertex == 4);
//    assert(statesArr[6].shortestPathUntilNow[0] == 0);
//    assert(statesArr[6].shortestPathUntilNow[1] == 2);
//    assert(statesArr[6].shortestPathUntilNow[2] == 4);
//    assert(statesArr[7].vertex == 5);
//    assert(statesArr[7].shortestPathUntilNow[0] == 0);
//    assert(statesArr[7].shortestPathUntilNow[1] == 2);
//    assert(statesArr[7].shortestPathUntilNow[2] == 5);
//    assert(statesArr[8].vertex == 1);
//    assert(statesArr[8].shortestPathUntilNow[0] == 0);
//    assert(statesArr[8].shortestPathUntilNow[1] == 3);
//    assert(statesArr[8].shortestPathUntilNow[2] == 1);
//    assert(statesArr[9].vertex == 2);
//    assert(statesArr[9].shortestPathUntilNow[0] == 0);
//    assert(statesArr[9].shortestPathUntilNow[1] == 3);
//    assert(statesArr[9].shortestPathUntilNow[2] == 2);
//    assert(statesArr[10].vertex == 4);
//    assert(statesArr[10].shortestPathUntilNow[0] == 0);
//    assert(statesArr[10].shortestPathUntilNow[1] == 3);
//    assert(statesArr[10].shortestPathUntilNow[2] == 4);
//    assert(statesArr[11].vertex == 5);
//    assert(statesArr[11].shortestPathUntilNow[0] == 0);
//    assert(statesArr[11].shortestPathUntilNow[1] == 3);
//    assert(statesArr[11].shortestPathUntilNow[2] == 5);
//    assert(statesArr[12].vertex == 1);
//    assert(statesArr[12].shortestPathUntilNow[0] == 0);
//    assert(statesArr[12].shortestPathUntilNow[1] == 4);
//    assert(statesArr[12].shortestPathUntilNow[2] == 1);
//    assert(statesArr[13].vertex == 2);
//    assert(statesArr[13].shortestPathUntilNow[0] == 0);
//    assert(statesArr[13].shortestPathUntilNow[1] == 4);
//    assert(statesArr[13].shortestPathUntilNow[2] == 2);
//    assert(statesArr[14].vertex == 3);
//    assert(statesArr[14].shortestPathUntilNow[0] == 0);
//    assert(statesArr[14].shortestPathUntilNow[1] == 4);
//    assert(statesArr[14].shortestPathUntilNow[2] == 3);
//    assert(statesArr[15].vertex == 5);
//    assert(statesArr[15].shortestPathUntilNow[0] == 0);
//    assert(statesArr[15].shortestPathUntilNow[1] == 4);
//    assert(statesArr[15].shortestPathUntilNow[2] == 5);
//    assert(statesArr[16].vertex == 1);
//    assert(statesArr[16].shortestPathUntilNow[0] == 0);
//    assert(statesArr[16].shortestPathUntilNow[1] == 5);
//    assert(statesArr[16].shortestPathUntilNow[2] == 1);
//    assert(statesArr[17].vertex == 2);
//    assert(statesArr[17].shortestPathUntilNow[0] == 0);
//    assert(statesArr[17].shortestPathUntilNow[1] == 5);
//    assert(statesArr[17].shortestPathUntilNow[2] == 2);
//    assert(statesArr[18].vertex == 3);
//    assert(statesArr[18].shortestPathUntilNow[0] == 0);
//    assert(statesArr[18].shortestPathUntilNow[1] == 5);
//    assert(statesArr[18].shortestPathUntilNow[2] == 3);
//    assert(statesArr[19].vertex == 4);
//    assert(statesArr[19].shortestPathUntilNow[0] == 0);
//    assert(statesArr[19].shortestPathUntilNow[1] == 5);
//    assert(statesArr[19].shortestPathUntilNow[2] == 4);
//
//    free_array_of_states(statesArr, numStates);
//    free_agency_matrix(agencyMatrix, citiesNum);
//
//
//
//    /* allocate_empty_array_of_int, free_array_of_int and copy_state test */
//    int xCoord5[] = {0, 1, 1};
//    int yCoord5[] = {0, 0, 1};
//    citiesNum = 3;
//    agencyMatrix = create_agency_matrix(xCoord5, yCoord5, citiesNum);
//
//    int *arr1 = allocate_empty_array_of_int(citiesNum);
//    int *arr2 = allocate_empty_array_of_int(citiesNum);
//    for (int i=0 ; i<citiesNum ; i++) {
//        assert(arr1[i] == NOT_SET);
//        assert(arr2[i] == NOT_SET);
//    }
//    State *emptyState = create_state(NOT_SET, arr1, arr2, citiesNum, agencyMatrix);
//
//    int shortestPathUntilNow50[] = {1, 2, 0};
//    int visitedNodesUntilNow50[] = {1, 1, 1};
//    State *realState = create_state(0, shortestPathUntilNow50,
//            visitedNodesUntilNow50, citiesNum, agencyMatrix);
//
//    copy_state(realState, emptyState, citiesNum);
//    assert(emptyState->vertex == 0);
//    assert(emptyState->cost == realState->cost);
//    assert(emptyState->shortestPathUntilNow[0] == 1);
//    assert(emptyState->shortestPathUntilNow[1] == 2);
//    assert(emptyState->shortestPathUntilNow[2] == 0);
//    assert(emptyState->visitedNodesUntilNow[0] == 1);
//    assert(emptyState->visitedNodesUntilNow[1] == 1);
//    assert(emptyState->visitedNodesUntilNow[2] == 1);
//
//    free(realState->shortestPathRes);
//    free(realState);
//    free_state(emptyState);
//    free_agency_matrix(agencyMatrix, citiesNum);
}








