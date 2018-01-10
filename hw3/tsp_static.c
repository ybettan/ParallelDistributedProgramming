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
//                            arrays of INT
//-----------------------------------------------------------------------------

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

/* return true if num is in arr and false otherwise */
bool is_in(int num, int *arr, int size) {
    
    for (int i=0 ; i<size ; i++) {
        if (num == arr[i])
            return true;
    }
    return false;
}

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

/* creates an array of states which each state get vertex, its cost and a copy
 * of shortestPathUntilNow input array.
 * MEM NOTE:
 * shortestPathUntilNow is freed inside the function */
State* create_state(int vertex, int citiesNum, int *shortestPathUntilNow,
        int **agencyMatrix) {

    State *state = malloc(sizeof(State) + (citiesNum-1) * sizeof(int));
    assert(state);

    state->vertex = vertex;

    /* compute the state's cost */
    int cost = 0;
    for (int k=1 ; k<citiesNum ; k++) {
        if (shortestPathUntilNow[k] == NOT_SET)
            break;
        cost += agencyMatrix[shortestPathUntilNow[k]][shortestPathUntilNow[k-1]];
    }
    state->cost = cost;

    /* copy shortestPathUntilNow */
    copy_array_of_int(shortestPathUntilNow, state->shortestPathUntilNow,
            citiesNum);

    free(shortestPathUntilNow);
    return state;
}

/* copy src array of states to dst array of states */
void copy_state(State *src, State *dst, int citiesNum) {

    dst->vertex = src->vertex;
    dst->cost = src->cost;
    copy_array_of_int(src->shortestPathUntilNow,
            dst->shortestPathUntilNow, citiesNum);
}

/* free the memory allocated for the array.
 * MEM NOTE:
 * do nothing in input is NULL */
void free_state(State *state) {
    if ( !state )
        return;
    free(state);
}

/* return true if state is a leaf in the search tree and false otherwise */
bool is_leaf(State *state, int citiesNum) {

    for (int i=0 ; i<citiesNum ; i++) {
        if (state->shortestPathUntilNow[i] == NOT_SET)
            return false;
    }
    return true;
}

//FIXME: remove
void print_array_of_int(int *arr, int size) {
    printf("[");
    for (int i=0 ; i<size ; i++) {
        if (i == size-1)
            printf("%d", arr[i]);
        else
            printf("%d, ", arr[i]);
    }
    printf("] ");
}
void print_state(State *state, int citiesNum) {
    printf("{%d, %d, ", state->vertex, state->cost);
    print_array_of_int(state->shortestPathUntilNow, citiesNum);
    printf("}\n");
}


//-----------------------------------------------------------------------------
//                           array of States
//-----------------------------------------------------------------------------

/* copy a state into the array at index placment */
void copy_state_into_array_of_states(void *arr, int index, int citiesNum,
        State *state) {

    int offset = index * (sizeof(State) + (citiesNum-1)*sizeof(int));
    void *ptr = (char*)arr + offset;

    /* set vertex */
    ((State*)ptr)->vertex = state->vertex;

    /* set cost */
    ((State*)ptr)->cost = state->cost;

    /* set shortestPathUntilNow */
    for (int i=0 ; i<citiesNum ; i++) {
        (((State*)ptr)->shortestPathUntilNow)[i] =
            (state->shortestPathUntilNow)[i];
    }
}

/* copy the state at index placment into the state input */
void copy_state_from_array_of_states(void *arr, int index, int citiesNum,
        State *state) {

    int offset = index * (sizeof(State) + (citiesNum-1)*sizeof(int));
    void *ptr = (char*)arr + offset;

    /* copy vertex */
    state->vertex = ((State*)ptr)->vertex;

    /* copy cost */
    state->cost = ((State*)ptr)->cost;

    /* copy shortestPathUntilNow */
    for (int i=0 ; i<citiesNum ; i++) {
        (state->shortestPathUntilNow)[i] =
            (((State*)ptr)->shortestPathUntilNow)[i];
    }
}


/* since the pinter aritmethic is broken here because of the State struct 
 * definition (shortestPathUntilNow[1] instead of dynamically allocated array),
 * we must supply wrrapper to read and write to the array */

/* allocated the memory for the array 
 * NOTE:
 * put NOT_SET in each vertex, 0 in each cost and an empty array (NOT_SET) in
 * shortestPathUntilNow field */
void* allocate_array_of_states(int size, int citiesNum, int **agencyMatrix) {

    void *arr = malloc(size * (sizeof(State) + (citiesNum-1) * sizeof(int)));
    assert(arr);

    int *emptyIntArr = allocate_empty_array_of_int(citiesNum);
    State *tmp = create_state(NOT_SET, citiesNum, emptyIntArr, agencyMatrix);
    for (int i=0 ; i<size ; i++) {
        copy_state_into_array_of_states(arr, i, citiesNum, tmp);
    }
    free_state(tmp);
    return arr;
}

void free_array_of_states(void *arr) {
    free(arr);
}

/* initiallize the array of states to be sent to other tasks 
 * NOTE:
 * arr is the result and must be initiallized with all vertex=NOT_SET */
void init_array_of_states(void *arr, int size, int citiesNum, State *state,
        int depth, int maxDepth, int **agencyMatrix) {

    /* if we have reached required depth insert states to arr */
    if (depth == maxDepth) {

        /* created a tmp State to old the result from the array */
        int *emptyIntArr = allocate_empty_array_of_int(citiesNum);
        State *tmp = create_state(NOT_SET, citiesNum, emptyIntArr,
                agencyMatrix);
        for (int i=0 ; i<size ; i++) {
            copy_state_from_array_of_states(arr, i, citiesNum, tmp);
            if (tmp->vertex == NOT_SET) {
                copy_state_into_array_of_states(arr, i, citiesNum, state); 
                free_state(tmp);
                return;
            }
        }
        free_state(tmp);
    }

    for (int i=0 ; i<citiesNum ; i++) {

        if (is_in(i, state->shortestPathUntilNow, citiesNum))
            continue;

        /* create shortestPathUntilNow array */
        int *shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
        copy_array_of_int(state->shortestPathUntilNow, shortestPathUntilNow,
                citiesNum);
        for (int k=0 ; k<citiesNum ; k++) {
            if(state->shortestPathUntilNow[k] == NOT_SET) {
                shortestPathUntilNow[k] = i;
                break;
            }
        }

        /* create the Son state */
        State *son = create_state(i, citiesNum, shortestPathUntilNow,
                agencyMatrix);
        assert(son);

        init_array_of_states(arr, size, citiesNum, son, depth+1,
                maxDepth, agencyMatrix);

        free_state(son);
    }
}

//-----------------------------------------------------------------------------
//                             heuristic
//-----------------------------------------------------------------------------


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


//-----------------------------------------------------------------------------
//                              CPU main
//-----------------------------------------------------------------------------

/* get the cost of a path */
int get_path_len(int *arr, int size, int **agencyMatrix) {

    int cost = 0;
    for (int k=1 ; k<size ; k++) {
        cost += agencyMatrix[arr[k]][arr[k-1]];
    }
    cost += agencyMatrix[arr[size-1]][arr[0]];

    return cost;
}

/* the main function runned by a single CPU
 * returns:
 * the minimum lenght as a return value
 * the shortest path in shortestPath variable */
void cpu_main(State *state, int citiesNum, int **agencyMatrix, int *minPathLen,
        int *shortestPath) {

    if (is_leaf(state, citiesNum)) {
        int rootVertex = state->shortestPathUntilNow[0];
        int lastVertex = state->shortestPathUntilNow[citiesNum-1];
        int costBackToRoot = agencyMatrix[rootVertex][lastVertex];
        int cost = state->cost + costBackToRoot;

        if (cost < *minPathLen) {
            *minPathLen = cost;
            copy_array_of_int(state->shortestPathUntilNow, shortestPath,
                    citiesNum);
        }
    }

    //int minPathLen = INF;
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
        State *son = create_state(i, citiesNum, shortestPathUntilNow,
                agencyMatrix);
        assert(son);

        /* prune branches if there is no chance to obtain shortest path from 
         * this son */
        if (heuristic(son, citiesNum, agencyMatrix) > *minPathLen) {
            free_array_of_states(son);
            continue;
        }

        /* compute the result for this son */
        cpu_main(son, citiesNum, agencyMatrix, minPathLen, shortestPath);

        free_state(son);
    }
        
    free_array_of_int(bestShortestPath);
}


//-----------------------------------------------------------------------------
//                                    MPI
//-----------------------------------------------------------------------------

///* register State struct into mpi library */
//void build_state_type(State *state, int citiesNum, MPI_Datatype *newTypeName) {
//    
//    int blockLengths[3] = {1, 1, citiesNum};
//    MPI_Aint displacements[3];
//    MPI_Datatype typelist[3] = {MPI_INT, MPI_INT, MPI_INT};
//    MPI_Aint addresses[4]; //helper array
//
//    /* compute displacements */
//    MPI_Get_address(state, &addresses[0]);
//    MPI_Get_address(&(state->vertex), &addresses[1]);
//    MPI_Get_address(&(state->cost), &addresses[2]);
//    MPI_Get_address(state->shortestPathUntilNow, &addresses[3]);
//
//    displacements[0] = addresses[1] - addresses[0];
//    displacements[1] = addresses[2] - addresses[0];
//    displacements[2] = addresses[3] - addresses[0];
//
//    /* create the derived type */
//    MPI_Type_create_struct(3, blockLengths, displacements, typelist, newTypeName);
//
//    /* commit the new type to mpi library */
//    MPI_Type_commit(newTypeName);
//}
//
///* send citiesNum, xCoord and yCoord from root to all other tasks */
//int send_initial_data(int citiesNum, int *xCoord, int *yCoord) {
//
//    int numTasks, rc1, rc2, rc3;
//    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
//
//    int bsendBuffSize = numTasks*((2*citiesNum+1)*sizeof(int))+MPI_BSEND_OVERHEAD;
//    int *bsendBuff = malloc(bsendBuffSize);
//    assert(bsendBuff);
//    MPI_Buffer_attach(bsendBuff, bsendBuffSize);
//
//    for (int i=1 ; i<numTasks ; i++) {
//        rc1 = MPI_Bsend(&citiesNum, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
//        rc2 = MPI_Bsend(xCoord, citiesNum, MPI_INT, i, TAG, MPI_COMM_WORLD);
//        rc3 = MPI_Bsend(yCoord, citiesNum, MPI_INT, i, TAG, MPI_COMM_WORLD);
//        assert(rc1 == MPI_SUCCESS && rc2 == MPI_SUCCESS && rc3 == MPI_SUCCESS);
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
//
//    assert(rc1 == MPI_SUCCESS && rc2 == MPI_SUCCESS && rc3 == MPI_SUCCESS);
//    return MPI_SUCCESS;
//}
//
//
///* send the data to other tasks - rank{1, 2,...numTasks}.
// * NOTE:
// * this fuction send all it gots, it is not aware that a part of the Arr
// * stays for root task */
//int send_data(State *dataArr, int numStates, int citiesNum,
//        int numStatesForRootTask, MPI_Datatype stateTypeName) {
//
//    /* get num of tasks */
//    int numTasks;
//    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
//
//    int elementSize = sizeof(State) + (citiesNum-1) * sizeof(int);
//    int numStatesToSend = numStates - numStatesForRootTask;
//
//    /* allocate buffers for MPI_Bsend */
//    int bsendBuffSize = numStatesToSend * elementSize + MPI_BSEND_OVERHEAD;
//    int *bsendBuff = malloc(bsendBuffSize);
//    assert(bsendBuff);
//
//    /* attach buffer for MPI_Bsend() */
//    MPI_Buffer_attach(bsendBuff, bsendBuffSize);
//
//    int minSendSize = numStatesToSend / (numTasks-1);
//    int tasksNumToSendMore = numStatesToSend % (numTasks-1);
//
//    void *start = (char*)dataArr + numStatesForRootTask * elementSize;
//    for (int i=1 ; i<numTasks ; i++) {
//        if (i <= tasksNumToSendMore) {
//            MPI_Bsend(start, minSendSize+1, stateTypeName, i, TAG, MPI_COMM_WORLD);
//            start = (char*)start + (minSendSize+1) * elementSize;
//        } else {
//            MPI_Bsend(start, minSendSize, stateTypeName, i, TAG, MPI_COMM_WORLD);
//            start = (char*)start + minSendSize * elementSize;
//        }
//    }
//
//    /* free memeory allocated */
//    free(bsendBuff);
//
//    return MPI_SUCCESS;
//}
//
//
///* the root task send all the data to other tasks, wait to all the data to be
// * received on other task, compute a sub problem itself, gather the results
// * of all other tasks and return the best result */
//int rootExec(int citiesNum, int **agencyMatrix, MPI_Datatype stateTypeName,
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
//
//    /* create rootState for initiallizing statesArr */
//    int *shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
//    shortestPathUntilNow[0] = 0;
//    State *rootState = create_state(0, citiesNum, shortestPathUntilNow,
//            agencyMatrix);
//
//    /* init array of states */
//    State *statesArr = allocate_array_of_states(numStates, citiesNum, agencyMatrix);
//    init_array_of_states(statesArr, numStates, citiesNum, rootState, 0, maxDepth,
//            agencyMatrix);
//
//
//    /* send the data to other tasks:
//     * statesArr[0:numStatesForRootTask-1] --> root states
//     * statesArr[numStatesForRootTask, numTasks-1] --> distributed tasks */
//    int numStatesForRootTask = numStates / numTasks;
//    int rc = send_data(statesArr,numStates, citiesNum, numStatesForRootTask,
//            stateTypeName);
//    assert(rc == MPI_SUCCESS);
//
//    /* wait for all tasks to receive the data */
//    MPI_Barrier(MPI_COMM_WORLD);
//
//    /* compute a sub problem like all other tasks */
//    int minPathLen = INF;
//    int pathLenLocal;
//    int *shortestPathLocal = allocate_empty_array_of_int(citiesNum);
//    int *emptyIntArr = allocate_empty_array_of_int(citiesNum);
//    State *it = create_state(NOT_SET, citiesNum, emptyIntArr, agencyMatrix);
//    for (int i=0 ; i<numStatesForRootTask ; i++) {
//        copy_state_from_array_of_states(statesArr, i, citiesNum, it);
//        pathLenLocal = cpu_main(it, citiesNum, agencyMatrix, shortestPathLocal);
//        if (pathLenLocal < minPathLen) {
//            minPathLen = pathLenLocal;
//            copy_array_of_int(shortestPathLocal, shortestPath, citiesNum);
//        }
//    }
//
//    //FIXME:------------------------------------------------------------------
//
//    printf("root local result = {%d, ", minPathLen);
//    print_array_of_int(shortestPath, citiesNum);
//    printf("}\n");
//
//    //FIXME:------------------------------------------------------------------
//
//    /* allocate a buffer to hold all local shortestPath */
//    int *allShortestPath = allocate_empty_array_of_int(numTasks * citiesNum);
//
//    /* gather the result from all other tasks */
//    MPI_Gather(shortestPath, citiesNum, MPI_INT, allShortestPath, citiesNum,
//            MPI_INT, 0, MPI_COMM_WORLD);
//    
//    //FIXME:------------------------------------------------------------------
//
//    for (int i=0 ; i<numTasks ; i++) {
//        int *x = allShortestPath + i*citiesNum;
//        printf("cpu %d on root = {%d, ", i, get_path_len(x, citiesNum, agencyMatrix));
//        print_array_of_int(x, citiesNum);
//        printf("}\n");
//    }
//
//    //FIXME:------------------------------------------------------------------
//    
//    /* update the best result */
//    for (int i=0, *start = allShortestPath ; i<numTasks ; i++, start+=citiesNum) {
//       pathLenLocal = get_path_len(start, citiesNum, agencyMatrix); 
//       if (pathLenLocal < minPathLen) {
//           minPathLen = pathLenLocal;
//           copy_array_of_int(start, shortestPath, citiesNum);
//       }
//    }
//
//    /* free the memory allocated */
//    free_state(rootState);
//    free_state(it);
//    free_array_of_int(shortestPathLocal);
//    free_array_of_int(allShortestPath);
//    free_array_of_states(statesArr);
//
//    return minPathLen;
//}
//
///* receive the data from the root task, compute a sub problem and return the
// * best local result to root task */
//void otherExec(int citiesNum, int **agencyMatrix, MPI_Datatype stateTypeName,
//        int *shortestPath) {
//
//    int rank, count;
//    MPI_Status status;
//    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//
//    /* reciver message size */
//    int rc1 = MPI_Probe(/*src=*/0, TAG, MPI_COMM_WORLD, &status);
//    int rc2 = MPI_Get_count(&status, stateTypeName, &count);
//    assert(rc1 == MPI_SUCCESS && rc2 == MPI_SUCCESS);
//
//    /* allocate receive buffer and receive the data from root task */
//    int elementSize = sizeof(State) + (citiesNum-1) * sizeof(int);
//    void *recvBuff = malloc(count * elementSize);
//    assert(recvBuff);
//
//    MPI_Recv(recvBuff, count, stateTypeName, /*src=*/0, TAG, MPI_COMM_WORLD,
//            &status);
//
//    /* wait for all tasks to receive the data */
//    MPI_Barrier(MPI_COMM_WORLD);
//
//    /* compute a sub problems and keep the best one */
//    int minPathLen = INF;
//    int pathLenLocal;
//    int *shortestPathLocal = allocate_empty_array_of_int(citiesNum);
//    int *emptyIntArr = allocate_empty_array_of_int(citiesNum);
//    State *it = create_state(NOT_SET, citiesNum, emptyIntArr, agencyMatrix);
//    for (int i=0 ; i<count ; i++) {
//        copy_state_from_array_of_states(recvBuff, i, citiesNum, it);
//        pathLenLocal = cpu_main(it, citiesNum, agencyMatrix, shortestPathLocal);
//        if (pathLenLocal < minPathLen) {
//            minPathLen = pathLenLocal;
//            copy_array_of_int(shortestPathLocal, shortestPath, citiesNum);
//        }
//    }
//    free_state(it);
//    free_array_of_int(shortestPathLocal);
//    free(recvBuff);
//
//    //FIXME:------------------------------------------------------------------
//
//    printf("cpu %d local result = {%d, ", rank, minPathLen);
//    print_array_of_int(shortestPath, citiesNum);
//    printf("}\n");
//
//    //FIXME:------------------------------------------------------------------
//
//    /* send the result to root task */
//    MPI_Gather(shortestPath, citiesNum, MPI_INT, NULL, citiesNum, MPI_INT, 0,
//            MPI_COMM_WORLD);
//
//}
//
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
//    shortestPathUntilNow[0] = 0;
//    State *rootState = create_state(0, citiesNum, shortestPathUntilNow,
//            agencyMatrix);
//
//    /* register State struct to MPI library */
//    build_state_type(rootState, citiesNum, &stateTypeName);
//
//    /* main logic master-slave */
//    int minPathLen;
//    if (rank == 0) {
//        minPathLen = rootExec(citiesNum, agencyMatrix, stateTypeName, shortestPath);
//    } else {
//        otherExec(citiesNum, agencyMatrix, stateTypeName, shortestPath);
//    }
//
//    /* free memory allocated */
//    free_state(rootState);
//    free_agency_matrix(agencyMatrix, citiesNum);
//    MPI_Type_free(&stateTypeName);
//
//    /* return the result */
//    if (rank != 0)
//        return 0;
//
//    return minPathLen;
//}



//-----------------------------------------------------------------------------
//                                  tests
//                              FIXME:remove
//-----------------------------------------------------------------------------

int main() {



    /* allocate_empty_array_of_int, copy_array_of_int, is_in and 
     * free_array_of_int test */
    int size = 5;
    int *arr100 = allocate_empty_array_of_int(size);
    int *arr101 = allocate_empty_array_of_int(size);
    for (int i=0 ; i<size ; i++) {
        assert(arr100[i] == NOT_SET);
        assert(arr101[i] == NOT_SET);
    }

    for (int i=0 ; i<size ; i++) {
        arr100[i] = i;
        arr101[i] = i;
    }

    int *arr102 = allocate_empty_array_of_int(size);
    int *arr103 = allocate_empty_array_of_int(size);

    assert( !is_in(3, arr102, size));
    assert( !is_in(5, arr102, size));
    assert( !is_in(0, arr103, size));

    copy_array_of_int(arr100, arr102, size);
    copy_array_of_int(arr101, arr103, size);

    for (int i=0 ; i<size ; i++) {
        assert(arr102[i] == i);
        assert(arr103[i] == i);
    }

    assert( is_in(3, arr102, size));
    assert( !is_in(5, arr102, size));
    assert( is_in(0, arr103, size));

    free_array_of_int(arr100);
    free_array_of_int(arr101);
    free_array_of_int(arr102);
    free_array_of_int(arr103);










    int xCoord[3] = {1, 1, 1};
    int yCoord[3] = {1, 2, 4};
    int citiesNum = 3;








    /* create matrix test */
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
    free_agency_matrix(agencyMatrix, citiesNum);









    /* state test */
    agencyMatrix = create_agency_matrix(xCoord, yCoord, citiesNum);
    int *shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow[0] = 1;
    /* {1, NOT_SET, NOT_SET} */
    State *state = create_state(1, citiesNum, shortestPathUntilNow, agencyMatrix);
    assert(state->vertex == 1);
    assert(state->cost == 0);
    assert(state->shortestPathUntilNow[0] == 1);
    assert(state->shortestPathUntilNow[1] == NOT_SET);
    assert(state->shortestPathUntilNow[2] == NOT_SET);
    assert( !is_leaf(state, citiesNum) );
    free_state(state);

    shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow[0] = 1;
    shortestPathUntilNow[1] = 2;
    /* {1, 2, NOT_SET} */
    state = create_state(2, citiesNum, shortestPathUntilNow, agencyMatrix);
    assert(state->vertex == 2);
    assert(state->cost == 5);
    assert( !is_leaf(state, citiesNum) );
    free_state(state);

    shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow[0] = 1;
    shortestPathUntilNow[1] = 2;
    shortestPathUntilNow[2] = 0;
    /* {1, 2, 0} */
    state = create_state(0, citiesNum,shortestPathUntilNow, agencyMatrix);
    assert( is_leaf(state, citiesNum) );
    assert(state->vertex == 0);
    assert(state->cost == 12);
    free_state(state);

    int *shortestPathUntilNow2 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow2[0] = 1;
    /* {1, NOT_SET, NOT_SET} */
    State *state2 = create_state(1, citiesNum, shortestPathUntilNow2, agencyMatrix);
    assert(!is_leaf(state2, citiesNum));
    free_state(state2);

    shortestPathUntilNow2 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow2[0] = 1;
    shortestPathUntilNow2[1] = 2;
    /* {1, 2, NOT_SET} */
    state2 = create_state(2, citiesNum, shortestPathUntilNow2, agencyMatrix);
    assert(!is_leaf(state2, citiesNum));
    free_state(state2);

    shortestPathUntilNow2 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow2[0] = 1;
    shortestPathUntilNow2[1] = 2;
    shortestPathUntilNow2[2] = 0;
    /* {1, 2, 0} */
    state2 = create_state(0, citiesNum, shortestPathUntilNow2, agencyMatrix);
    assert(is_leaf(state2, citiesNum));
    free_state(state2);
    free_agency_matrix(agencyMatrix, citiesNum);








    /* array of states test */
    int arraySize = 3;
    agencyMatrix = create_agency_matrix(xCoord, yCoord, citiesNum);
    shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow[0] = 0;
    shortestPathUntilNow[1] = 1;
    shortestPathUntilNow[2] = 2;
    State *oldState0 = create_state(2, citiesNum, shortestPathUntilNow, agencyMatrix);
    shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow[0] = 0;
    shortestPathUntilNow[1] = 2;
    shortestPathUntilNow[2] = 1;
    State *oldState1 = create_state(1, citiesNum, shortestPathUntilNow, agencyMatrix);
    shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow[0] = 1;
    shortestPathUntilNow[1] = 0;
    shortestPathUntilNow[2] = 2;
    State *oldState2 = create_state(2, citiesNum, shortestPathUntilNow, agencyMatrix);

    void *statesArr = allocate_array_of_states(arraySize, citiesNum, agencyMatrix);

    copy_state_into_array_of_states(statesArr, 0, citiesNum, oldState0);
    copy_state_into_array_of_states(statesArr, 1, citiesNum, oldState1);
    copy_state_into_array_of_states(statesArr, 2, citiesNum, oldState2);

    int *emptyIntArr = allocate_empty_array_of_int(citiesNum);
    State *newState0 = create_state(NOT_SET, citiesNum, emptyIntArr, agencyMatrix);
    emptyIntArr = allocate_empty_array_of_int(citiesNum);
    State *newState1 = create_state(NOT_SET, citiesNum, emptyIntArr, agencyMatrix);
    emptyIntArr = allocate_empty_array_of_int(citiesNum);
    State *newState2 = create_state(NOT_SET, citiesNum, emptyIntArr, agencyMatrix);

    copy_state_from_array_of_states(statesArr, 0, citiesNum, newState0);
    copy_state_from_array_of_states(statesArr, 1, citiesNum, newState1);
    copy_state_from_array_of_states(statesArr, 2, citiesNum, newState2);

    assert(newState0->vertex == 2);
    assert(newState0->cost == oldState0->cost);
    assert(newState0->shortestPathUntilNow[0] == 0);
    assert(newState0->shortestPathUntilNow[1] == 1);
    assert(newState0->shortestPathUntilNow[2] == 2);

    assert(newState1->vertex == 1);
    assert(newState1->cost == oldState1->cost);
    assert(newState1->shortestPathUntilNow[0] == 0);
    assert(newState1->shortestPathUntilNow[1] == 2);
    assert(newState1->shortestPathUntilNow[2] == 1);

    assert(newState2->vertex == 2);
    assert(newState2->cost == oldState2->cost);
    assert(newState2->shortestPathUntilNow[0] == 1);
    assert(newState2->shortestPathUntilNow[1] == 0);
    assert(newState2->shortestPathUntilNow[2] == 2);

    free_state(oldState0);
    free_state(oldState1);
    free_state(oldState2);
    free_state(newState0);
    free_state(newState1);
    free_state(newState2);
    free_array_of_states(statesArr);
    free_agency_matrix(agencyMatrix, citiesNum);

    int xCoord3[] = {0, 1, 1, 0, 2, 4};
    int yCoord3[] = {0, 0, 1, 1 ,0, 0};
    int shortestPath3[6];
    citiesNum = 6;
    int numStates = 20;
    agencyMatrix = create_agency_matrix(xCoord3, yCoord3, citiesNum);

    int *shortestPathUntilNow30 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow30[0] = 0;
    /* {0, NOT_SET, NOT_SET, NOT_SET, NOT_SET, NOT_SET} */
    State *rootState = create_state(0, citiesNum, shortestPathUntilNow30, agencyMatrix);

    statesArr = allocate_array_of_states(numStates, citiesNum, agencyMatrix);
    init_array_of_states(statesArr, numStates, citiesNum, rootState, 0, 2, agencyMatrix);

    free_state(rootState);

    int *emptyIntArr2 = allocate_empty_array_of_int(citiesNum);
    State *res = create_state(NOT_SET, citiesNum, emptyIntArr2, agencyMatrix);

    copy_state_from_array_of_states(statesArr, 0, citiesNum, res);
    assert(res->vertex == 2);
    assert(res->cost == 6);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 1);
    assert(res->shortestPathUntilNow[2] == 2);

    copy_state_from_array_of_states(statesArr, 1, citiesNum, res);
    assert(res->vertex == 3);
    assert(res->cost == 8);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 1);
    assert(res->shortestPathUntilNow[2] == 3);

    copy_state_from_array_of_states(statesArr, 2, citiesNum, res);
    assert(res->vertex == 4);
    assert(res->cost == 6);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 1);
    assert(res->shortestPathUntilNow[2] == 4);

    copy_state_from_array_of_states(statesArr, 3, citiesNum, res);
    assert(res->vertex == 5);
    assert(res->cost == 10);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 1);
    assert(res->shortestPathUntilNow[2] == 5);

    copy_state_from_array_of_states(statesArr, 4, citiesNum, res);
    assert(res->vertex == 1);
    assert(res->cost == 8);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 2);
    assert(res->shortestPathUntilNow[2] == 1);

    copy_state_from_array_of_states(statesArr, 5, citiesNum, res);
    assert(res->vertex == 3);
    assert(res->cost == 8);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 2);
    assert(res->shortestPathUntilNow[2] == 3);

    copy_state_from_array_of_states(statesArr, 6, citiesNum, res);
    assert(res->vertex == 4);
    assert(res->cost == 10);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 2);
    assert(res->shortestPathUntilNow[2] == 4);

    copy_state_from_array_of_states(statesArr, 7, citiesNum, res);
    assert(res->vertex == 5);
    assert(res->cost == 14);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 2);
    assert(res->shortestPathUntilNow[2] == 5);

    copy_state_from_array_of_states(statesArr, 8, citiesNum, res);
    assert(res->vertex == 1);
    assert(res->cost == 8);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 3);
    assert(res->shortestPathUntilNow[2] == 1);

    copy_state_from_array_of_states(statesArr, 9, citiesNum, res);
    assert(res->vertex == 2);
    assert(res->cost == 6);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 3);
    assert(res->shortestPathUntilNow[2] == 2);

    copy_state_from_array_of_states(statesArr, 10, citiesNum, res);
    assert(res->vertex == 4);
    assert(res->cost == 10);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 3);
    assert(res->shortestPathUntilNow[2] == 4);

    copy_state_from_array_of_states(statesArr, 11, citiesNum, res);
    assert(res->vertex == 5);
    assert(res->cost == 14);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 3);
    assert(res->shortestPathUntilNow[2] == 5);

    copy_state_from_array_of_states(statesArr, 12, citiesNum, res);
    assert(res->vertex == 1);
    assert(res->cost == 8);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 4);
    assert(res->shortestPathUntilNow[2] == 1);

    copy_state_from_array_of_states(statesArr, 13, citiesNum, res);
    assert(res->vertex == 2);
    assert(res->cost == 10);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 4);
    assert(res->shortestPathUntilNow[2] == 2);

    copy_state_from_array_of_states(statesArr, 14, citiesNum, res);
    assert(res->vertex == 3);
    assert(res->cost == 12);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 4);
    assert(res->shortestPathUntilNow[2] == 3);

    copy_state_from_array_of_states(statesArr, 15, citiesNum, res);
    assert(res->vertex == 5);
    assert(res->cost == 10);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 4);
    assert(res->shortestPathUntilNow[2] == 5);

    copy_state_from_array_of_states(statesArr, 16, citiesNum, res);
    assert(res->vertex == 1);
    assert(res->cost == 16);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 5);
    assert(res->shortestPathUntilNow[2] == 1);

    copy_state_from_array_of_states(statesArr, 17, citiesNum, res);
    assert(res->vertex == 2);
    assert(res->cost == 18);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 5);
    assert(res->shortestPathUntilNow[2] == 2);

    copy_state_from_array_of_states(statesArr, 18, citiesNum, res);
    assert(res->vertex == 3);
    assert(res->cost == 20);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 5);
    assert(res->shortestPathUntilNow[2] == 3);

    copy_state_from_array_of_states(statesArr, 19, citiesNum, res);
    assert(res->vertex == 4);
    assert(res->cost == 14);
    assert(res->shortestPathUntilNow[0] == 0);
    assert(res->shortestPathUntilNow[1] == 5);
    assert(res->shortestPathUntilNow[2] == 4);

    free_state(res);
    free_array_of_states(statesArr);
    free_agency_matrix(agencyMatrix, citiesNum);












    /* heuristic test */
    citiesNum = 3;
    agencyMatrix = create_agency_matrix(xCoord, yCoord, citiesNum);
    int *shortestPathUntilNow20 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow20[0] = 0;
    /* {0, NOT_SET, NOT_SET} */
    State *state20 = create_state(0, citiesNum, shortestPathUntilNow20, agencyMatrix);
    assert(state20->cost == 0);
    assert(heuristic(state20, citiesNum, agencyMatrix) == 11);
    free_state(state20);

    shortestPathUntilNow20 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow20[0] = 0;
    shortestPathUntilNow20[1] = 2;
    /* {0, 2, NOT_SET} */
    state20 = create_state(2, citiesNum,shortestPathUntilNow20, agencyMatrix);
    assert(state20->cost == 7);
    assert(heuristic(state20, citiesNum, agencyMatrix) == 13);
    free_state(state20);

    shortestPathUntilNow20 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow20[0] = 0;
    shortestPathUntilNow20[1] = 2;
    shortestPathUntilNow20[2] = 1;
    /* {0, 2, 1} */
    state20 = create_state(1, citiesNum, shortestPathUntilNow20, agencyMatrix);
    assert(state20->cost == 12);
    assert(heuristic(state20, citiesNum, agencyMatrix) == 15);
    free_state(state20);

    shortestPathUntilNow20 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow20[0] = 0;
    shortestPathUntilNow20[1] = 1;
    /* {0, 1, NOT_SET} */
    state20 = create_state(1, citiesNum, shortestPathUntilNow20, agencyMatrix);
    assert(state20->cost == 3);
    assert(heuristic(state20, citiesNum, agencyMatrix) == 11);
    free_state(state20);

    shortestPathUntilNow20 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow20[0] = 0;
    shortestPathUntilNow20[1] = 1;
    shortestPathUntilNow20[2] = 2;
    /* {0, 1, 2} */
    state20 = create_state(2, citiesNum, shortestPathUntilNow20, agencyMatrix);
    assert(state20->cost == 8);
    assert(heuristic(state20, citiesNum, agencyMatrix) == 11);
    free_state(state20);
    free_agency_matrix(agencyMatrix, citiesNum);











    /* cpu_main test */
    int xCoord2[] = {0, 1, 1, 0};
    int yCoord2[] = {0, 0, 1, 1};
    citiesNum = 4;
    int shortestPath[4];
    agencyMatrix = create_agency_matrix(xCoord2, yCoord2, citiesNum);
    
//---------------------------------------------------------------- len 4

    int *shortestPathUntilNow3 = allocate_empty_array_of_int(citiesNum);
    int minPathLen2 = INF;
    shortestPathUntilNow3[0] = 0;
    shortestPathUntilNow3[1] = 1;
    shortestPathUntilNow3[2] = 2;
    shortestPathUntilNow3[3] = 3;
    /* {0,1,2,3} */
    State *state3 = create_state(3, citiesNum, shortestPathUntilNow3, agencyMatrix);   
    cpu_main(state3, citiesNum, agencyMatrix, &minPathLen2, shortestPath);
    assert(minPathLen2 == 12);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 1);
    assert(shortestPath[2] == 2);
    assert(shortestPath[3] == 3);
    free_state(state3);

    int *shortestPathUntilNow4 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow4[0] = 0;
    shortestPathUntilNow4[1] = 1;
    shortestPathUntilNow4[2] = 3;
    shortestPathUntilNow4[3] = 2;
    /* {0,1,3,2} */
    State *state4 = create_state(2, citiesNum, shortestPathUntilNow4, agencyMatrix);   
    minPathLen2 = INF;
    cpu_main(state4, citiesNum, agencyMatrix, &minPathLen2, shortestPath);
    assert(minPathLen2 == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 1);
    assert(shortestPath[2] == 3);
    assert(shortestPath[3] == 2);
    free_state(state4);

    int *shortestPathUntilNow5 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow5[0] = 0;
    shortestPathUntilNow5[1] = 2;
    shortestPathUntilNow5[2] = 1;
    shortestPathUntilNow5[3] = 3;
    /* {0,2,1,3} */
    State *state5 = create_state(3, citiesNum, shortestPathUntilNow5, agencyMatrix);   
    minPathLen2 = INF;
    cpu_main(state5, citiesNum, agencyMatrix, &minPathLen2, shortestPath);
    assert(minPathLen2 == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 2);
    assert(shortestPath[2] == 1);
    assert(shortestPath[3] == 3);
    free_state(state5);

    int *shortestPathUntilNow6 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow6[0] = 0;
    shortestPathUntilNow6[1] = 2;
    shortestPathUntilNow6[2] = 3;
    shortestPathUntilNow6[3] = 1;
    /* {0,2,3,1} */
    State *state6 = create_state(1, citiesNum, shortestPathUntilNow6, agencyMatrix);   
    minPathLen2 = INF;
    cpu_main(state6, citiesNum, agencyMatrix, &minPathLen2, shortestPath);
    assert(minPathLen2 == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 2);
    assert(shortestPath[2] == 3);
    assert(shortestPath[3] == 1);
    free_state(state6);

    int *shortestPathUntilNow7 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow7[0] = 0;
    shortestPathUntilNow7[1] = 3;
    shortestPathUntilNow7[2] = 1;
    shortestPathUntilNow7[3] = 2;
    /* {0,3,1,2} */
    State *state7 = create_state(2, citiesNum, shortestPathUntilNow7, agencyMatrix);   
    minPathLen2 = INF;
    cpu_main(state7, citiesNum, agencyMatrix, &minPathLen2, shortestPath);
    assert(minPathLen2 == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 3);
    assert(shortestPath[2] == 1);
    assert(shortestPath[3] == 2);
    free_state(state7);

    int *shortestPathUntilNow8 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow8[0] = 0;
    shortestPathUntilNow8[1] = 3;
    shortestPathUntilNow8[2] = 2;
    shortestPathUntilNow8[3] = 1;
    /* {0,3,2,1} */
    State *state8 = create_state(1, citiesNum, shortestPathUntilNow8, agencyMatrix);   
    minPathLen2 = INF;
    cpu_main(state8, citiesNum, agencyMatrix, &minPathLen2, shortestPath);
    assert(minPathLen2 == 12);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 3);
    assert(shortestPath[2] == 2);
    assert(shortestPath[3] == 1);
    free_state(state8);

//---------------------------------------------------------------- len 3

    int *shortestPathUntilNow9 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow9[0] = 0;
    shortestPathUntilNow9[1] = 1;
    shortestPathUntilNow9[2] = 2;
    /* {0,1,2,NOT_SET} */
    State *state9 = create_state(2, citiesNum, shortestPathUntilNow9, agencyMatrix);   
    minPathLen2 = INF;
    cpu_main(state9, citiesNum, agencyMatrix, &minPathLen2, shortestPath);
    assert(minPathLen2 == 12);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 1);
    assert(shortestPath[2] == 2);
    assert(shortestPath[3] == 3);
    free_state(state9);

    int *shortestPathUntilNow10 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow10[0] = 0;
    shortestPathUntilNow10[1] = 1;
    shortestPathUntilNow10[2] = 3;
    /* {0,1,3,NOT_SET} */
    State *state10 = create_state(3, citiesNum, shortestPathUntilNow10, agencyMatrix);   
    minPathLen2 = INF;
    cpu_main(state10, citiesNum, agencyMatrix, &minPathLen2, shortestPath);
    assert(minPathLen2 == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 1);
    assert(shortestPath[2] == 3);
    assert(shortestPath[3] == 2);
    free_state(state10);

    int *shortestPathUntilNow11 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow11[0] = 0;
    shortestPathUntilNow11[1] = 2;
    shortestPathUntilNow11[2] = 1;
    /* {0,2,1,NOT_SET} */
    State *state11 = create_state(1, citiesNum, shortestPathUntilNow11, agencyMatrix);   
    minPathLen2 = INF;
    cpu_main(state11, citiesNum, agencyMatrix, &minPathLen2, shortestPath);
    assert(minPathLen2 == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 2);
    assert(shortestPath[2] == 1);
    assert(shortestPath[3] == 3);
    free_state(state11);

    int *shortestPathUntilNow12 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow12[0] = 0;
    shortestPathUntilNow12[1] = 2;
    shortestPathUntilNow12[2] = 3;
    /* {0,2,3,NOT_SET} */
    State *state12 = create_state(3, citiesNum, shortestPathUntilNow12, agencyMatrix);   
    minPathLen2 = INF;
    cpu_main(state12, citiesNum, agencyMatrix, &minPathLen2, shortestPath);
    assert(minPathLen2 == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 2);
    assert(shortestPath[2] == 3);
    assert(shortestPath[3] == 1);
    free_state(state12);

    int *shortestPathUntilNow13 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow13[0] = 0;
    shortestPathUntilNow13[1] = 3;
    shortestPathUntilNow13[2] = 1;
    /* {0,3,1,NOT_SET} */
    State *state13 = create_state(1, citiesNum, shortestPathUntilNow13, agencyMatrix);   
    minPathLen2 = INF;
    cpu_main(state13, citiesNum, agencyMatrix, &minPathLen2, shortestPath);
    assert(minPathLen2 == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 3);
    assert(shortestPath[2] == 1);
    assert(shortestPath[3] == 2);
    free_state(state13);

    int *shortestPathUntilNow14 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow14[0] = 0;
    shortestPathUntilNow14[1] = 3;
    shortestPathUntilNow14[2] = 2;
    /* {0,3,2,NOT_SET} */
    State *state14 = create_state(2, citiesNum, shortestPathUntilNow14, agencyMatrix);   
    minPathLen2 = INF;
    cpu_main(state14, citiesNum, agencyMatrix, &minPathLen2, shortestPath);
    assert(minPathLen2 == 12);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 3);
    assert(shortestPath[2] == 2);
    assert(shortestPath[3] == 1);
    free_state(state14);

//---------------------------------------------------------------- len 2

    int *shortestPathUntilNow15 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow15[0] = 0;
    shortestPathUntilNow15[1] = 1;
    /* {0,1,NOT_SET,NOT_SET} */
    State *state15 = create_state(1, citiesNum, shortestPathUntilNow15, agencyMatrix);   
    minPathLen2 = INF;
    cpu_main(state15, citiesNum, agencyMatrix, &minPathLen2, shortestPath);
    assert(minPathLen2 == 12);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 1);
    assert(shortestPath[2] == 2);
    assert(shortestPath[3] == 3);
    free_state(state15);

    int *shortestPathUntilNow16 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow16[0] = 0;
    shortestPathUntilNow16[1] = 2;
    /* {0,2,NOT_SET,NOT_SET} */
    State *state16 = create_state(2, citiesNum, shortestPathUntilNow16, agencyMatrix);   
    minPathLen2 = INF;
    cpu_main(state16, citiesNum, agencyMatrix, &minPathLen2, shortestPath);
    assert(minPathLen2 == 16);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 2);
    assert((shortestPath[2] == 1 && shortestPath[3] == 3) ||
           (shortestPath[2] == 3 && shortestPath[3] == 1));
    free_state(state16);

    int *shortestPathUntilNow17 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow17[0] = 0;
    shortestPathUntilNow17[1] = 3;
    /* {0,3,NOT_SET,NOT_SET} */
    State *state17 = create_state(3, citiesNum, shortestPathUntilNow17, agencyMatrix);   
    minPathLen2 = INF;
    cpu_main(state17, citiesNum, agencyMatrix, &minPathLen2, shortestPath);
    assert(minPathLen2 == 12);
    assert(shortestPath[0] == 0);
    assert(shortestPath[1] == 3);
    assert(shortestPath[2] == 2);
    assert(shortestPath[3] == 1);
    free_state(state17);
    
//---------------------------------------------------------------- len 1

    int *shortestPathUntilNow18 = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow18[0] = 0;
    /* {0,NOT_SET,NOT_SET,NOT_SET} */
    State *state18 = create_state(0, citiesNum, shortestPathUntilNow18, agencyMatrix);   
    minPathLen2 = INF;
    cpu_main(state18, citiesNum, agencyMatrix, &minPathLen2, shortestPath);
    assert(minPathLen2 == 12);
    assert(shortestPath[0] == 0);
    assert((shortestPath[1]==1 && shortestPath[2]==2 && shortestPath[3]==3) ||
           (shortestPath[1]==3 && shortestPath[2]==2 && shortestPath[3]==1));
    free_state(state18);
    free_agency_matrix(agencyMatrix, citiesNum);







    /* extra test */
	citiesNum = 6;
	int xCoord60[] = {1, 3, 5, 9, 2,  3};   
	int yCoord60[] = {1, 3, 5, 7, 9, 11};   
    agencyMatrix = create_agency_matrix(xCoord60, yCoord60, citiesNum);
    int *sp = allocate_empty_array_of_int(citiesNum);
    int *ea5 = allocate_empty_array_of_int(citiesNum);
    ea5[0] = 0;
    State *rs = create_state(0, citiesNum, ea5, agencyMatrix);

    int minPathLen = INF;
    minPathLen2 = INF;
    cpu_main(rs, citiesNum, agencyMatrix, &minPathLen, sp);
    assert(minPathLen == get_path_len(sp, citiesNum, agencyMatrix)); 


    free_state(rs);
    free_array_of_int(sp);
    free_agency_matrix(agencyMatrix, citiesNum);



}






