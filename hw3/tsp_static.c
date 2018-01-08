#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>


#define INF 3000000
#define NOT_SET -1
#define ERROR -2
#define TAG 99

/*
 * restruction:
 *
 * data distribution --> only point to point communication is allowed
 * results gathering --> only collective communication is allowed
 * we can't bound the number of citys
 */



//-----------------------------------------------------------------------------
//                               State
//-----------------------------------------------------------------------------

typedef struct {

    int vertex;
    int cost;
    /* for example {3, 5, 1, NOT_SET, NOT_SET} for citiesNum=5 */
    int *shortestPathUntilNow;
    /* visitedNodesUntilNow[i] == 1 if and only if node i was visited */
    int *visitedNodesUntilNow;
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
        int *visitedNodesUntilNow, int citiesNum, int **agencyMatrix) {

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

/* copy src state to dst state */
void copy_state(State *src, State *dst, int citiesNum) {
    dst->vertex = src->vertex;
    dst->cost = src->cost;
    for (int i=0 ; i<citiesNum ; i++) {
        dst->shortestPathUntilNow[i] = src->shortestPathUntilNow[i];
        dst->visitedNodesUntilNow[i] = src->visitedNodesUntilNow[i];
        dst->shortestPathRes[i] = src->shortestPathRes[i];
    }
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

/* allocate memory for arrays of in with all initiallized to NOT_SET */
int* create_empty_generic_arr(int citiesNum) {
    int *res = malloc(citiesNum * sizeof(int));
    assert(res);
    for (int i=0 ; i<citiesNum ; i++) {
        res[i] = NOT_SET;
    }
    return res;
}

void free_generic_arr(int *arr) {
    free(arr);
}

//-----------------------------------------------------------------------------
//                                heuristic
//-----------------------------------------------------------------------------

/* for a given cost of a node, caculate the lower bound of the path using the
 * lightest edge of each vertex that wehere not visited.
 * (the path may not exist but this will be out lower bound) */
int heuristic(State *state, int citiesNum, int **agencyMatrix) {

    int res = state->cost;

    /* go over all not-visited vertex */
    for (int i=0 ; i<citiesNum ; i++) {

        if (state->visitedNodesUntilNow[i] && i!=state->shortestPathUntilNow[0])
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
//                             agency matrix
//-----------------------------------------------------------------------------

/* crate an agency matrix for the given graph for easier caculations */
int** create_agency_matrix(int *xCoord, int *yCoord, int citiesNum) {

    /* allocate  the matrix */
    int **agencyMatrix = malloc(citiesNum * sizeof(int*));
    if ( !agencyMatrix ) {
        printf("cannot allocate agencyMatrix\n");
        return NULL;
    }
    for (int i=0 ; i<citiesNum ; i++) {
        agencyMatrix[i] = malloc(citiesNum * sizeof(int));
        if ( !agencyMatrix[i] ) {
            printf("cannot allocate agencyMatrix\n");
            return NULL;
        }
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
    return agencyMatrix;
}


/* free the allocated memory for the agency matrix */
void free_agency_matrix(int **agencyMatrix, int citiesNum) {

    for (int i=0 ; i<citiesNum ; i++) {
        free(agencyMatrix[i]);
    }
    free(agencyMatrix);
}


//-----------------------------------------------------------------------------
//                              CPU main
//-----------------------------------------------------------------------------

bool is_leaf(State *state, int citiesNum) {
    for (int i=0 ; i<citiesNum ; i++) {
        if ( !state->visitedNodesUntilNow[i] )
            return false;
    }
    return true;
}

/* copy state->shortestPathUntilNow into the result array */
void copy_shortest_path(int *src, int *dst, int citiesNum) {
    for (int i=0 ; i<citiesNum ; i++) {
        dst[i] = src[i];
    }
}

/* the main function runned by a single CPU
 * returns:
 * the minimum lenght as a return value
 * the shortest path in shortestPath variable */
int cpu_main(State *state, int citiesNum, int **agencyMatrix,
        int *shortestPath) {

    if (is_leaf(state, citiesNum)) {
        int rootVertex = state->shortestPathUntilNow[0];
        int lastVertex = state->shortestPathUntilNow[citiesNum-1];
        int costBackToRoot = agencyMatrix[rootVertex][lastVertex];
        /* insert the shortest path result to the state */
        copy_shortest_path(state->shortestPathUntilNow, state->shortestPathRes,
                citiesNum);
        copy_shortest_path(state->shortestPathRes, shortestPath, citiesNum);
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
        int *visitedNodesUntilNow = malloc(citiesNum * sizeof(int));
        if ( !visitedNodesUntilNow ) {
            printf("cannot allocate visitedNodesUntilNow\n");
            return ERROR;
        }
        for (int k=0 ; k<citiesNum ; k++) {
            visitedNodesUntilNow[k] = state->visitedNodesUntilNow[k];
        }
        visitedNodesUntilNow[i] = 1;

        /* create the Son state */
        State *son = create_state(i, shortestPathUntilNow, visitedNodesUntilNow,
                citiesNum, agencyMatrix);
        if ( !son ) {
            printf("cannot allocate son\n");
            return ERROR;
        }

        /* prune branches if there is no chance to obtain shortest path from 
         * this son */
        if (heuristic(son, citiesNum, agencyMatrix) > minPathLen) {
            free_state(son);
            continue;
        }

        /* compute the result for this son */
        int res = cpu_main(son, citiesNum, agencyMatrix, shortestPath);
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
        
    copy_shortest_path(bestSon->shortestPathRes, state->shortestPathRes,
            citiesNum);
    copy_shortest_path(state->shortestPathRes, shortestPath, citiesNum);
    free_state(bestSon);
    return minPathLen;
}



//-----------------------------------------------------------------------------
//                                    MPI
//-----------------------------------------------------------------------------

/* register State struct into mpi library */
void build_state_type(State *state, int citiesNum, MPI_Datatype *newTypeName) {
    
    int blockLengths[5] = {1, 1, citiesNum, citiesNum, citiesNum};
    MPI_Aint displacements[5];
    MPI_Datatype typelist[5] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint addresses[6]; //helper array

    /* compute displacements */
    MPI_Get_address(state, &addresses[0]);
    MPI_Get_address(&state->vertex, &addresses[1]);
    MPI_Get_address(&state->cost, &addresses[2]);
    MPI_Get_address(&state->shortestPathUntilNow, &addresses[3]);
    MPI_Get_address(&state->visitedNodesUntilNow, &addresses[4]);
    MPI_Get_address(&state->shortestPathRes, &addresses[5]);

    displacements[0] = addresses[1] - addresses[0];
    displacements[1] = addresses[2] - addresses[0];
    displacements[2] = addresses[3] - addresses[0];
    displacements[3] = addresses[4] - addresses[0];
    displacements[4] = addresses[5] - addresses[0];

    /* create the derived type */
    MPI_Type_create_struct(5, blockLengths, displacements, typelist, newTypeName);

    /* commit the new type to mpi library */
    MPI_Type_commit(newTypeName);
}

/* allocate the memory for statesArr and initiallize it to NOT_SET values */
State* allocate_array_of_states(int size) {
    State *res = malloc(size * sizeof(State));
    assert(res);
    for (int i=0 ; i<size ; i++) {
        res[i].vertex = NOT_SET;
        res[i].cost = NOT_SET;
        res[i].shortestPathUntilNow = create_empty_generic_arr(size);
        res[i].visitedNodesUntilNow = create_empty_generic_arr(size);
        res[i].shortestPathRes = create_empty_generic_arr(size);
    }
    return res;
}

void free_array_of_states(State *statesArr, int size) {
    for (int i=0 ; i<size ; i++) {
        free_generic_arr(statesArr[i].shortestPathUntilNow);
        free_generic_arr(statesArr[i].visitedNodesUntilNow);
        free_generic_arr(statesArr[i].shortestPathRes);
    }
    free(statesArr);
}

/* initiallize the array of states array of states to be sent to other tasks */
void initiallize_array_of_states(int **agencyMatrix, State *state, int citiesNum,
        int depth, int maxDepth, int arrSize, State *arrRes) {

    /* if we have reached required depth insert states to arrRes */
    if (depth == maxDepth) {
        for (int i=0 ; i<arrSize ; i++) {
            if (arrRes[i].vertex == NOT_SET) {
                copy_state(state, &arrRes[i], citiesNum);
                return;
            }
        }
    }

    for (int i=0 ; i<citiesNum ; i++) {

        if (state->visitedNodesUntilNow[i])
            continue;

        /* create shortestPathUntilNow array */
        int *shortestPathUntilNow = malloc(citiesNum * sizeof(int));
        assert(shortestPathUntilNow);
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
        int *visitedNodesUntilNow = malloc(citiesNum * sizeof(int));
        assert(visitedNodesUntilNow);
        for (int k=0 ; k<citiesNum ; k++) {
            visitedNodesUntilNow[k] = state->visitedNodesUntilNow[k];
        }
        visitedNodesUntilNow[i] = 1;

        /* create the Son state */
        State *son = create_state(i, shortestPathUntilNow, visitedNodesUntilNow,
                citiesNum, agencyMatrix);
        assert(son);

        initiallize_array_of_states(agencyMatrix, son, citiesNum, depth+1,
                maxDepth, arrSize, arrRes);

        free_state(son);
    }
}
//void initiallize_array_of_states(int **agencyMatrix, State *state, int citiesNum,
//        int depth, int maxDepth, int arrSize, State **arrRes) {
//
//    /* if we have reached required depth insert states to arrRes */
//    if (depth == maxDepth) {
//        for (int i=0 ; i<arrSize ; i++) {
//            if (arrRes[i] == NULL) {
//                arrRes[i] = state;
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
//        initiallize_array_of_states(agencyMatrix, son, citiesNum, depth+1, maxDepth,
//                arrSize, arrRes);
//    }
//    free_state(state);
//}

//void free_array_of_states(State **statesArr, int size) {
//    for (int i=0 ; i<size ; i++) {
//        free_state(statesArr[i]);
//    }
//    free(statesArr);
//}

/* send messages to other tasks rank{1, 2,...numTasks}.
 * NOTE:
 * this fuction send all it gots, it is not aware that a part of the statesArr
 * stays for root task */
//FIXME: update statesArr
int send_to_other_tasks(State *statesArrToOther, int numStates,
        MPI_Datatype stateTypeName) {

    /* get num of tasks */
    int numTasks;
    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);

    /* allocate buffers for MPI_Bsend */
    int bsendBuffSize = numStates * (sizeof(int) + MPI_BSEND_OVERHEAD);
    int *bsendBuff = malloc(bsendBuffSize);
    assert(bsendBuff);

    /* attach buffer for MPI_Bsend() */
    MPI_Buffer_attach(bsendBuff, bsendBuffSize);

    int minSendSize = numStates / (numTasks-1);
    int tasksNumToSendMore = numStates % (numTasks-1);
    //FIXME:remove
    //printf("root : minSendSize = %d\n", minSendSize);
    //printf("root : tasksNumToSendMore = %d\n", tasksNumToSendMore);

    ////FIXME: send States insead of this
    //int offset = 0;
    //for (int i=1 ; i<numTasks ; i++) {
    //    if (i <= tasksNumToSendMore) {
    //        printf("root : sendig %d elements to task_%d\n", (minSendSize+1), i);
    //        MPI_Bsend(statesArrToOther+offset, minSendSize+1, stateTypeName, i,
    //                TAG, MPI_COMM_WORLD);
    //        offset += minSendSize+1;
    //    } else {
    //        printf("root : sendig %d elements to task_%d\n", minSendSize, i);
    //        MPI_Bsend(statesArrToOther+offset, minSendSize, stateTypeName, i,
    //                TAG, MPI_COMM_WORLD);
    //        offset += minSendSize;
    //    }
    //}
    int msg[] = {1, 2, 3};
    for (int i=1 ; i<numTasks ; i++) {
        if (i <= tasksNumToSendMore) {
            printf("root : sendig %d elements to task_%d\n", (minSendSize+1), i);
            MPI_Bsend(&msg, minSendSize+1, MPI_INT, i, TAG, MPI_COMM_WORLD);
        } else {
            printf("root : sendig %d elements to task_%d\n", minSendSize, i);
            MPI_Bsend(&msg, minSendSize, MPI_INT, i, TAG, MPI_COMM_WORLD);
        }
    }

    free(bsendBuff);
    return MPI_SUCCESS;
}



/* the root task send all the data to other tasks, wait to all the data to be
 * received on other task, compute a sub problem itself, gather the results
 * of all other tasks and return the best result */
void rootExec(int citiesNum, int *xCoord, int *yCoord, int *shortestPath) {

    printf("root : performing rootExec()\n");
    MPI_Datatype stateTypeName;

    /* create agency matrix */
    int **agencyMatrix = create_agency_matrix(xCoord, yCoord, citiesNum);
    assert(agencyMatrix);
    printf("root : created agencyMatrix\n");

    /* create rootState for registering State struct to mpi library */
    int *shortestPathUntilNow = malloc(citiesNum * sizeof(int));
    int *visitedNodesUntilNow = malloc(citiesNum * sizeof(int));
    assert(shortestPathUntilNow && visitedNodesUntilNow);
    shortestPathUntilNow[0] = 0;
    visitedNodesUntilNow[0] = 1;
    for (int i=1 ; i<citiesNum ; i++) {
        shortestPathUntilNow[i] = NOT_SET;
        visitedNodesUntilNow[i] = 0;
    }
    State *rootState = create_state(0, shortestPathUntilNow,
            visitedNodesUntilNow, citiesNum, agencyMatrix);
    printf("root : created rootState\n");

    /* register State struct to MPI library */
    build_state_type(rootState, citiesNum, &stateTypeName);
    printf("root : registerd rootState to MPI library\n");


    /* compute how deep the root task need to go in order to have enought 
     * States for all CPUs */
    int maxDepth = 1;
    int citiesNumLeft = citiesNum-1;
    int numStates = citiesNumLeft;
    int numTasks;
    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
    while (numStates < numTasks) {
        maxDepth++;
        citiesNumLeft--;
        numStates *= citiesNumLeft;
    }
    printf("root : #CPUs = %d\nroot : maxDepth = %d\n", numTasks, maxDepth);

    State *statesArr = allocate_array_of_states(numStates);
    initiallize_array_of_states(agencyMatrix, rootState, citiesNum, 0, maxDepth,
            numStates, statesArr);
    free_state(rootState);
    printf("root : created statesArr\n");

    /* send the data to other tasks:
     * statesArr[0:numStatesForRootTask-1] - root states
     * statesArr[numStatesForRootTask, numTasks-1] - distributed tasks */
    int numStatesForRootTask = numStates / numTasks;
    int rc = send_to_other_tasks(statesArr+numStatesForRootTask,
            numStates-numStatesForRootTask, stateTypeName);
    assert(rc == MPI_SUCCESS);

    /* wait for all tasks to receive the data */
    MPI_Barrier(MPI_COMM_WORLD);
    printf("root : continue after barrier\n");

    ///* compute a sub problem like all other tasks */
    //int minPathLen = INF;
    //int *shortestPathLocal = malloc(citiesNum * sizeof(int));
    //assert(shortestPathLocal);
    //for (int i=0 ; i<numStatesForRootTask ; i++) {
    //    res = cpu_main(statesArr[i], citiesNum, agencyMatrix, shortestPathLocal);
    //    if (res < minPathLen) {
    //        minPathLen = res;
    //        copy_shortest_path(shortestPathLocal, shortestPath, citiesNum)
    //    }
    //}

    ///* gather the result from all other tasks */
    //MPI_Gather(NULL, 1, stateTypeName, res...)

    /* free memory */
    free_array_of_states(statesArr, numStates);

    ///* now shortestPath hold the best value best result */
}

/* receive the data from the root task, compute a sub problem and return the
 * best local result to root task */
void otherExec() {

    int rank, rc, count;
    //State *recvBuff = NULL;
    int *recvBuff = NULL;
    MPI_Status status;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("cpu %d : performing otherExec()\n", rank);

    /* reciver message size */
    rc = MPI_Probe(/*src=*/0, TAG, MPI_COMM_WORLD, &status);
    assert(rc == MPI_SUCCESS);
    rc = MPI_Get_count(&status, MPI_INT, &count);
    assert(rc == MPI_SUCCESS);
    printf("cpu %d : received %d messages from root\n", rank, count);

    /* allocate receive buffer and receive the data from root task */
    //recvBuff = allocate_array_of_states(count);
    recvBuff = malloc(count * sizeof(int));
    assert(recvBuff);
    //MPI_Recv(recvBuff, count, stateTypeName, /*src=*/0, TAG, MPI_COMM_WORLD, &status);
    MPI_Recv(recvBuff, count, MPI_INT, /*src=*/0, TAG, MPI_COMM_WORLD, &status);
    //FIXME:remove
    //printf("cpu %d : received message [%d]\n", rank, recvBuff[0]);
    //printf("cpu %d : received message [%d]\n", rank, recvBuff[1]);
    //if (rank >= 1 && rank <= 6) {
    //    printf("cpu %d : received message [%d]\n", rank, recvBuff[2]);
    //}

    /* wait for all tasks to receive the data */
    MPI_Barrier(MPI_COMM_WORLD);
    printf("cpu %d : continue after barrier\n", rank);

    /* compute a sub problems and keep the best one */

    /* send the result to root task */

}

// The static parellel algorithm main function.
int tsp_main(int citiesNum, int xCoord[], int yCoord[], int shortestPath[])
{
    int numTasks, myRank;

    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

    if (myRank == 0) {
        rootExec(citiesNum, xCoord, yCoord, shortestPath);
    } else {
        otherExec(citiesNum, xCoord, yCoord, shortestPath);
    }


	return -1;	//TODO
}



////-----------------------------------------------------------------------------
////                                  tests
////                              FIXME:remove
////-----------------------------------------------------------------------------
//
////FIXME: for testing
//int main() {
//
//    int xCoord[3] = {1, 1, 1};
//    int yCoord[3] = {1, 2, 4};
//
//    /* create matrix test */
//    int citiesNum = 3;
//    int **agencyMatrix = create_agency_matrix(xCoord, yCoord, citiesNum);
//    assert(agencyMatrix[0][0] == INF);
//    assert(agencyMatrix[0][1] == 3);
//    assert(agencyMatrix[0][2] == 7);
//    assert(agencyMatrix[1][0] == 3);
//    assert(agencyMatrix[1][1] == INF);
//    assert(agencyMatrix[1][2] == 5);
//    assert(agencyMatrix[2][0] == 7);
//    assert(agencyMatrix[2][1] == 5);
//    assert(agencyMatrix[2][2] == INF);
//
//    /* heuristic test */
//    int visitedNodesUntilNow20[] = {true, false, false};
//    int shortestPathUntilNow20[] = {0, NOT_SET, NOT_SET};
//    State *state20;
//    state20 = create_state(0, shortestPathUntilNow20, visitedNodesUntilNow20,
//            citiesNum, agencyMatrix);
//    assert(state20->cost == 0);
//    assert(heuristic(state20, citiesNum, agencyMatrix) == 11);
//    free(state20->shortestPathRes);
//    free(state20);
//    visitedNodesUntilNow20[2] = true;
//    /* not visitedNodesUntilNow20 is {true, false, true} */
//    shortestPathUntilNow20[1] = 2;
//    /* not shortestPathUntilNow20 is {0, 2, NOT_SET} */
//    state20 = create_state(0, shortestPathUntilNow20, visitedNodesUntilNow20,
//            citiesNum, agencyMatrix);
//    assert(state20->cost == 7);
//    assert(heuristic(state20, citiesNum, agencyMatrix) == 13);
//    free(state20->shortestPathRes);
//    free(state20);
//    visitedNodesUntilNow20[1] = true;
//    /* not visitedNodesUntilNow20 is {true, true, true} */
//    shortestPathUntilNow20[2] = 1;
//    /* not shortestPathUntilNow20 is {0, 2, 1} */
//    state20 = create_state(0, shortestPathUntilNow20, visitedNodesUntilNow20,
//            citiesNum, agencyMatrix);
//    assert(state20->cost == 12);
//    assert(heuristic(state20, citiesNum, agencyMatrix) == 15);
//    free(state20->shortestPathRes);
//    free(state20);
//    visitedNodesUntilNow20[2] = false;
//    /* not visitedNodesUntilNow20 is {true, true, false} */
//    shortestPathUntilNow20[1] = 1;
//    shortestPathUntilNow20[2] = NOT_SET;
//    /* not shortestPathUntilNow20 is {0, 1, NOT_SET} */
//    state20 = create_state(0, shortestPathUntilNow20, visitedNodesUntilNow20,
//            citiesNum, agencyMatrix);
//    assert(state20->cost == 3);
//    assert(heuristic(state20, citiesNum, agencyMatrix) == 11);
//    free(state20->shortestPathRes);
//    free(state20);
//    visitedNodesUntilNow20[2] = true;
//    /* not visitedNodesUntilNow20 is {true, true, false} */
//    shortestPathUntilNow20[2] = 2;
//    /* not shortestPathUntilNow20 is {0, 1, 2} */
//    state20 = create_state(0, shortestPathUntilNow20, visitedNodesUntilNow20,
//            citiesNum, agencyMatrix);
//    assert(state20->cost == 8);
//    assert(heuristic(state20, citiesNum, agencyMatrix) == 11);
//    free(state20->shortestPathRes);
//    free(state20);
//
//    /* create_test test */
//    int shortestPathUntilNow[] = {1, NOT_SET, NOT_SET};
//    int visitedNodesUntilNow[] = {false, true, false};
//    State *state = create_state(1, shortestPathUntilNow, visitedNodesUntilNow,
//            citiesNum, agencyMatrix);
//    assert(state->vertex == 1);
//    assert(state->cost == 0);
//    assert(state->shortestPathUntilNow[0] == 1);
//    assert(state->shortestPathUntilNow[1] == NOT_SET);
//    assert(state->shortestPathUntilNow[2] == NOT_SET);
//    assert(state->visitedNodesUntilNow[0] == false);
//    assert(state->visitedNodesUntilNow[1] == true);
//    assert(state->visitedNodesUntilNow[2] == false);
//    shortestPathUntilNow[1] = 2;
//    /* now shortestPathUntilNow[] = {1, 2, NOT_SET} */
//    visitedNodesUntilNow[2] = true;
//    /* now visitedNodesUntilNow[] = {false, true, true} */
//    free(state->shortestPathRes);
//    free(state);
//    state = create_state(2, shortestPathUntilNow, visitedNodesUntilNow,
//            citiesNum, agencyMatrix);
//    assert(state->vertex == 2);
//    assert(state->cost == 5);
//    shortestPathUntilNow[2] = 0;
//    /* now shortestPathUntilNow[] = {1, 2, 0} */
//    visitedNodesUntilNow[0] = true;
//    /* now visitedNodesUntilNow[] = {true, true, true} */
//    free(state->shortestPathRes);
//    free(state);
//    state = create_state(0, shortestPathUntilNow, visitedNodesUntilNow,
//            citiesNum, agencyMatrix);
//    assert(state->vertex == 0);
//    assert(state->cost == 12);
//    free(state->shortestPathRes);
//    free(state);
//
//    /* is_leaf test */
//    int shortestPathUntilNow2[] = {1, NOT_SET, NOT_SET};
//    int visitedNodesUntilNow2[3] = {false, false, false};
//    State *state2 = create_state(3, shortestPathUntilNow2, visitedNodesUntilNow2,
//            citiesNum, agencyMatrix);
//    assert(!is_leaf(state2, citiesNum));
//    visitedNodesUntilNow2[1] = true;
//    free(state2->shortestPathRes);
//    free(state2);
//    state2 = create_state(3, shortestPathUntilNow2, visitedNodesUntilNow2,
//            citiesNum, agencyMatrix);
//    assert(!is_leaf(state2, citiesNum));
//    visitedNodesUntilNow2[0] = true;
//    visitedNodesUntilNow2[2] = true;
//    free(state2->shortestPathRes);
//    free(state2);
//    state2 = create_state(3, shortestPathUntilNow2, visitedNodesUntilNow2,
//            citiesNum, agencyMatrix);
//    assert(is_leaf(state2, citiesNum));
//    free(state2->shortestPathRes);
//    free(state2);
//
//    free_agency_matrix(agencyMatrix, citiesNum);
//
//
//    /* cpu_main test */
//    int xCoord2[] = {0, 1, 1, 0};
//    int yCoord2[] = {0, 0, 1, 1};
//    citiesNum = 4;
//    int shortestPath[4];
//    agencyMatrix = create_agency_matrix(xCoord2, yCoord2, citiesNum);
//    
//    //---------------------------------------------------------------- len 4
//    int shortestPathUntilNow3[] = {0,1,2,3};
//    int visitedNodesUntilNow3[] = {true, true, true, true};
//    State *state3 = create_state(0, shortestPathUntilNow3, visitedNodesUntilNow3,
//            citiesNum, agencyMatrix);   
//    assert(cpu_main(state3, citiesNum, agencyMatrix, shortestPath) == 12);
//    assert(shortestPath[0] == 0);
//    assert(shortestPath[1] == 1);
//    assert(shortestPath[2] == 2);
//    assert(shortestPath[3] == 3);
//    free(state3->shortestPathRes);
//    free(state3);
//
//    int shortestPathUntilNow4[] = {0,1,3,2};
//    int visitedNodesUntilNow4[] = {true, true, true, true};
//    State *state4 = create_state(0, shortestPathUntilNow4, visitedNodesUntilNow4,
//            citiesNum, agencyMatrix);   
//    assert(cpu_main(state4, citiesNum, agencyMatrix, shortestPath) == 16);
//    assert(shortestPath[0] == 0);
//    assert(shortestPath[1] == 1);
//    assert(shortestPath[2] == 3);
//    assert(shortestPath[3] == 2);
//    free(state4->shortestPathRes);
//    free(state4);
//
//    int shortestPathUntilNow5[] = {0,2,1,3};
//    int visitedNodesUntilNow5[] = {true, true, true, true};
//    State *state5 = create_state(0, shortestPathUntilNow5, visitedNodesUntilNow5,
//            citiesNum, agencyMatrix);   
//    assert(cpu_main(state5, citiesNum, agencyMatrix, shortestPath) == 16);
//    assert(shortestPath[0] == 0);
//    assert(shortestPath[1] == 2);
//    assert(shortestPath[2] == 1);
//    assert(shortestPath[3] == 3);
//    free(state5->shortestPathRes);
//    free(state5);
//
//    int shortestPathUntilNow6[] = {0,2,3,1};
//    int visitedNodesUntilNow6[] = {true, true, true, true};
//    State *state6 = create_state(0, shortestPathUntilNow6, visitedNodesUntilNow6,
//            citiesNum, agencyMatrix);   
//    assert(cpu_main(state6, citiesNum, agencyMatrix, shortestPath) == 16);
//    assert(shortestPath[0] == 0);
//    assert(shortestPath[1] == 2);
//    assert(shortestPath[2] == 3);
//    assert(shortestPath[3] == 1);
//    free(state6->shortestPathRes);
//    free(state6);
//
//    int shortestPathUntilNow7[] = {0,3,1,2};
//    int visitedNodesUntilNow7[] = {true, true, true, true};
//    State *state7 = create_state(0, shortestPathUntilNow7, visitedNodesUntilNow7,
//            citiesNum, agencyMatrix);   
//    assert(cpu_main(state7, citiesNum, agencyMatrix, shortestPath) == 16);
//    assert(shortestPath[0] == 0);
//    assert(shortestPath[1] == 3);
//    assert(shortestPath[2] == 1);
//    assert(shortestPath[3] == 2);
//    free(state7->shortestPathRes);
//    free(state7);
//
//    int shortestPathUntilNow8[] = {0,3,2,1};
//    int visitedNodesUntilNow8[] = {true, true, true, true};
//    State *state8 = create_state(0, shortestPathUntilNow8, visitedNodesUntilNow8,
//            citiesNum, agencyMatrix);   
//    assert(cpu_main(state8, citiesNum, agencyMatrix, shortestPath) == 12);
//    assert(shortestPath[0] == 0);
//    assert(shortestPath[1] == 3);
//    assert(shortestPath[2] == 2);
//    assert(shortestPath[3] == 1);
//    free(state8->shortestPathRes);
//    free(state8);
//
//    //---------------------------------------------------------------- len 3
//    int shortestPathUntilNow9[] = {0,1,2,NOT_SET};
//    int visitedNodesUntilNow9[] = {true, true, true, false};
//    State *state9 = create_state(0, shortestPathUntilNow9, visitedNodesUntilNow9,
//            citiesNum, agencyMatrix);   
//    assert(cpu_main(state9, citiesNum, agencyMatrix, shortestPath) == 12);
//    assert(shortestPath[0] == 0);
//    assert(shortestPath[1] == 1);
//    assert(shortestPath[2] == 2);
//    assert(shortestPath[3] == 3);
//    free(state9->shortestPathRes);
//    free(state9);
//
//    int shortestPathUntilNow10[] = {0,1,3,NOT_SET};
//    int visitedNodesUntilNow10[] = {true, true, false, true};
//    State *state10=create_state(0, shortestPathUntilNow10, visitedNodesUntilNow10,
//            citiesNum, agencyMatrix);   
//    assert(cpu_main(state10, citiesNum, agencyMatrix, shortestPath) == 16);
//    assert(shortestPath[0] == 0);
//    assert(shortestPath[1] == 1);
//    assert(shortestPath[2] == 3);
//    assert(shortestPath[3] == 2);
//    free(state10->shortestPathRes);
//    free(state10);
//
//    int shortestPathUntilNow11[] = {0,2,1,NOT_SET};
//    int visitedNodesUntilNow11[] = {true, true, true, false};
//    State *state11=create_state(0, shortestPathUntilNow11, visitedNodesUntilNow11,
//            citiesNum, agencyMatrix);   
//    assert(cpu_main(state11, citiesNum, agencyMatrix, shortestPath) == 16);
//    assert(shortestPath[0] == 0);
//    assert(shortestPath[1] == 2);
//    assert(shortestPath[2] == 1);
//    assert(shortestPath[3] == 3);
//    free(state11->shortestPathRes);
//    free(state11);
//
//    int shortestPathUntilNow12[] = {0,2,3,NOT_SET};
//    int visitedNodesUntilNow12[] = {true, false, true, true};
//    State *state12=create_state(0, shortestPathUntilNow12, visitedNodesUntilNow12,
//            citiesNum, agencyMatrix);   
//    assert(cpu_main(state12, citiesNum, agencyMatrix, shortestPath) == 16);
//    assert(shortestPath[0] == 0);
//    assert(shortestPath[1] == 2);
//    assert(shortestPath[2] == 3);
//    assert(shortestPath[3] == 1);
//    free(state12->shortestPathRes);
//    free(state12);
//
//    int shortestPathUntilNow13[] = {0,3,1,NOT_SET};
//    int visitedNodesUntilNow13[] = {true, true, false, true};
//    State *state13=create_state(0, shortestPathUntilNow13, visitedNodesUntilNow13,
//            citiesNum, agencyMatrix);   
//    assert(cpu_main(state13, citiesNum, agencyMatrix, shortestPath) == 16);
//    assert(shortestPath[0] == 0);
//    assert(shortestPath[1] == 3);
//    assert(shortestPath[2] == 1);
//    assert(shortestPath[3] == 2);
//    free(state13->shortestPathRes);
//    free(state13);
//
//    int shortestPathUntilNow14[] = {0,3,2,NOT_SET};
//    int visitedNodesUntilNow14[] = {true, false, true, true};
//    State *state14=create_state(0, shortestPathUntilNow14, visitedNodesUntilNow14,
//            citiesNum, agencyMatrix);   
//    assert(cpu_main(state14, citiesNum, agencyMatrix, shortestPath) == 12);
//    assert(shortestPath[0] == 0);
//    assert(shortestPath[1] == 3);
//    assert(shortestPath[2] == 2);
//    assert(shortestPath[3] == 1);
//    free(state14->shortestPathRes);
//    free(state14);
//
//    //---------------------------------------------------------------- len 2
//    int shortestPathUntilNow15[] = {0,1,NOT_SET,NOT_SET};
//    int visitedNodesUntilNow15[] = {true, true, false, false};
//    State *state15=create_state(0, shortestPathUntilNow15, visitedNodesUntilNow15,
//            citiesNum, agencyMatrix);   
//    assert(cpu_main(state15, citiesNum, agencyMatrix, shortestPath) == 12);
//    assert(shortestPath[0] == 0);
//    assert(shortestPath[1] == 1);
//    assert(shortestPath[2] == 2);
//    assert(shortestPath[3] == 3);
//    free(state15->shortestPathRes);
//    free(state15);
//
//    int shortestPathUntilNow16[] = {0,2,NOT_SET,NOT_SET};
//    int visitedNodesUntilNow16[] = {true, false, true, false};
//    State *state16=create_state(0, shortestPathUntilNow16, visitedNodesUntilNow16,
//            citiesNum, agencyMatrix);   
//    assert(cpu_main(state16, citiesNum, agencyMatrix, shortestPath) == 16);
//    assert(shortestPath[0] == 0);
//    assert(shortestPath[1] == 2);
//    assert((shortestPath[2] == 1 && shortestPath[3] == 3) ||
//           (shortestPath[2] == 3 && shortestPath[3] == 1));
//    free(state16->shortestPathRes);
//    free(state16);
//
//    int shortestPathUntilNow17[] = {0,3,NOT_SET,NOT_SET};
//    int visitedNodesUntilNow17[] = {true, false, false, true};
//    State *state17=create_state(0, shortestPathUntilNow17, visitedNodesUntilNow17,
//            citiesNum, agencyMatrix);   
//    assert(cpu_main(state17, citiesNum, agencyMatrix, shortestPath) == 12);
//    assert(shortestPath[0] == 0);
//    assert(shortestPath[1] == 3);
//    assert(shortestPath[2] == 2);
//    assert(shortestPath[3] == 1);
//    free(state17->shortestPathRes);
//    free(state17);
//
//    int shortestPathUntilNow18[] = {0,NOT_SET,NOT_SET,NOT_SET};
//    int visitedNodesUntilNow18[] = {true, false, false, false};
//    State *state18=create_state(0, shortestPathUntilNow18, visitedNodesUntilNow18,
//            citiesNum, agencyMatrix);   
//    assert(cpu_main(state18, citiesNum, agencyMatrix, shortestPath) == 12);
//    assert(shortestPath[0] == 0);
//    assert((shortestPath[1]==1 && shortestPath[2]==2 && shortestPath[3]==3) ||
//           (shortestPath[1]==3 && shortestPath[2]==2 && shortestPath[3]==1));
//    free(state18->shortestPathRes);
//    free(state18);
//    free_agency_matrix(agencyMatrix, citiesNum);
//
//    /* create_array_of_state test */
//    int xCoord3[] = {0, 1, 1, 0, 2, 4};
//    int yCoord3[] = {0, 0, 1, 1 ,0, 0};
//    citiesNum = 6;
//    int shortestPath3[6];
//    agencyMatrix = create_agency_matrix(xCoord3, yCoord3, citiesNum);
//    int *shortestPathUntilNow30 = malloc(citiesNum * sizeof(int));
//    int *visitedNodesUntilNow30 = malloc(citiesNum * sizeof(int));
//    shortestPathUntilNow30[0] = 0;
//    visitedNodesUntilNow30[0] = 1;
//    for (int i=1 ; i<citiesNum ; i++) {
//        shortestPathUntilNow30[i] = NOT_SET;
//        visitedNodesUntilNow30[i] = 0;
//    }
//    State *rootState = create_state(0, shortestPathUntilNow30,
//            visitedNodesUntilNow30, citiesNum, agencyMatrix);
//    int numStates = 20;
//    State *statesArr = allocate_array_of_states(numStates);
//    initiallize_array_of_states(agencyMatrix, rootState, citiesNum, 0, 2, numStates,
//            statesArr);
//    free_state(rootState);
//
//    assert(statesArr[0].vertex == 2);
//    assert(statesArr[0].shortestPathUntilNow[0] == 0);
//    assert(statesArr[0].shortestPathUntilNow[1] == 1);
//    assert(statesArr[0].shortestPathUntilNow[2] == 2);
//    assert(statesArr[1].vertex == 3);
//    assert(statesArr[1].shortestPathUntilNow[0] == 0);
//    assert(statesArr[1].shortestPathUntilNow[1] == 1);
//    assert(statesArr[1].shortestPathUntilNow[2] == 3);
//    assert(statesArr[2].vertex == 4);
//    assert(statesArr[2].shortestPathUntilNow[0] == 0);
//    assert(statesArr[2].shortestPathUntilNow[1] == 1);
//    assert(statesArr[2].shortestPathUntilNow[2] == 4);
//    assert(statesArr[3].vertex == 5);
//    assert(statesArr[3].shortestPathUntilNow[0] == 0);
//    assert(statesArr[3].shortestPathUntilNow[1] == 1);
//    assert(statesArr[3].shortestPathUntilNow[2] == 5);
//    assert(statesArr[4].vertex == 1);
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
//    /* create_empty_generic_arr, free_generic_arr and copy_state test */
//    int xCoord5[] = {0, 1, 1};
//    int yCoord5[] = {0, 0, 1};
//    citiesNum = 3;
//    agencyMatrix = create_agency_matrix(xCoord5, yCoord5, citiesNum);
//
//    int *arr1 = create_empty_generic_arr(citiesNum);
//    int *arr2 = create_empty_generic_arr(citiesNum);
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
//}








