#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>


#define INF 3000000
#define NOT_SET -1
#define ERROR -2
#define TAG 99

#define PREFIX_LEN(citiesNum)   ((citiesNum)-3)



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

bool are_equal_array_of_int(int *a, int *b, int size) {
    for (int i=0 ; i<size ; i++) {
        if (a[i] != b[i])
            return false;
    }
    return true;
}

/* return true if num is in arr and false otherwise */
bool is_in(int num, int *arr, int size) {
    
    for (int i=0 ; i<size ; i++) {
        if (num == arr[i])
            return true;
    }
    return false;
}

void increas_prefix_rec(int *oldPrefix, int *newPrefix, int prefixLen,
        int citiesNum, int depth, bool *foundCurrPrefix, bool *foundNextPrefix) {

    if  (depth == prefixLen) { 

        if (*foundCurrPrefix)
            *foundNextPrefix = true;
        else if (are_equal_array_of_int(oldPrefix, newPrefix, prefixLen))
            *foundCurrPrefix = true;

        return;
    }

    for (int i=0 ; i<citiesNum ; i++) {

        if (is_in(i, newPrefix, prefixLen))
            continue;

        int *tmpPrefix = allocate_empty_array_of_int(prefixLen);
        copy_array_of_int(newPrefix, tmpPrefix, prefixLen);
        tmpPrefix[depth] = i;
        increas_prefix_rec(oldPrefix, tmpPrefix, prefixLen, citiesNum, depth+1,
                foundCurrPrefix, foundNextPrefix);
        
        if (*foundNextPrefix) {
            copy_array_of_int(tmpPrefix, newPrefix, prefixLen);
            free_array_of_int(tmpPrefix);
            return;
        }

        free_array_of_int(tmpPrefix);
    }
}

/* increase the prefix by 1.
 * ASSERT that prefix[0] == 0 */
void increas_prefix(int *prefix, int prefixLen, int citiesNum) {
   
    assert(prefix[0] == 0);
    bool foundCurrPrefix = false, foundNextPrefix = false;
    int *newPrefix = allocate_empty_array_of_int(prefixLen);

    increas_prefix_rec(prefix, newPrefix, prefixLen, citiesNum, 0,
            &foundCurrPrefix, &foundNextPrefix); 

    copy_array_of_int(newPrefix, prefix, prefixLen);
    free_array_of_int(newPrefix);
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
    printf("{%d, %d ", state->vertex, state->cost);
    print_array_of_int(state->shortestPathUntilNow, citiesNum);
    printf("} ");
}

//-----------------------------------------------------------------------------
//                           array of States
//-----------------------------------------------------------------------------

/* since the pointer aritmethic is broken here because of the State struct 
 * definition (shortestPathUntilNow[1] instead of dynamically allocated array),
 * we must supply wrrapper to read and write to the array */


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

/* put a first State for each task in the array.
 * arr[0] will contain an empty State since root task does't do work 
 * returns:
 * the next prefix in turn */
int* init_array_of_states(void *arr, int size, int citiesNum, int prefixLen,
        int **agencyMatrix) {

    /* the first state is an empty state for root task */
    int *emptyIntArr = allocate_empty_array_of_int(citiesNum);
    State *emptyState = create_state(NOT_SET, citiesNum, emptyIntArr, agencyMatrix);
    copy_state_into_array_of_states(arr, 0, citiesNum, emptyState);

    /* set the first prefix to [0, 1, 2, ...,NOT_SET, NOT_SET, ...] */
    int *prefix = allocate_empty_array_of_int(citiesNum);
    for (int i=0 ; i<prefixLen ; i++) {
        prefix[i] = i;
    }

    /* insert the states into the array */
    for (int i=1 ; i<size ; i++) {

        int *shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
        copy_array_of_int(prefix, shortestPathUntilNow, citiesNum);
        State *nextState = create_state(shortestPathUntilNow[prefixLen-1],
                citiesNum, shortestPathUntilNow, agencyMatrix);

        /* insert the state into the array */
        copy_state_into_array_of_states(arr, i, citiesNum, nextState);
        free_state(nextState);

        increas_prefix(prefix, prefixLen, citiesNum);
    }

    /* free memory */
    free_state(emptyState);

    return prefix;
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

/* register State struct into mpi library */
void build_state_type(State *state, int citiesNum, MPI_Datatype *newTypeName) {
    
    int blockLengths[3] = {1, 1, citiesNum};
    MPI_Aint displacements[3];
    MPI_Datatype typelist[3] = {MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint addresses[4]; //helper array

    /* compute displacements */
    MPI_Get_address(state, &addresses[0]);
    MPI_Get_address(&(state->vertex), &addresses[1]);
    MPI_Get_address(&(state->cost), &addresses[2]);
    MPI_Get_address(state->shortestPathUntilNow, &addresses[3]);

    displacements[0] = addresses[1] - addresses[0];
    displacements[1] = addresses[2] - addresses[0];
    displacements[2] = addresses[3] - addresses[0];

    /* create the derived type */
    MPI_Type_create_struct(3, blockLengths, displacements, typelist, newTypeName);

    /* commit the new type to mpi library */
    MPI_Type_commit(newTypeName);
}

/////* send citiesNum, xCoord and yCoord from root to all other tasks */
////int send_initial_data(int citiesNum, int *xCoord, int *yCoord) {
////
////    int numTasks, rc1, rc2, rc3;
////    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
////
////    int bsendBuffSize = numTasks*((2*citiesNum+1)*sizeof(int))+MPI_BSEND_OVERHEAD;
////    int *bsendBuff = malloc(bsendBuffSize);
////    assert(bsendBuff);
////    MPI_Buffer_attach(bsendBuff, bsendBuffSize);
////
////    for (int i=1 ; i<numTasks ; i++) {
////        rc1 = MPI_Bsend(&citiesNum, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
////        rc2 = MPI_Bsend(xCoord, citiesNum, MPI_INT, i, TAG, MPI_COMM_WORLD);
////        rc3 = MPI_Bsend(yCoord, citiesNum, MPI_INT, i, TAG, MPI_COMM_WORLD);
////        assert(rc1 == MPI_SUCCESS && rc2 == MPI_SUCCESS && rc3 == MPI_SUCCESS);
////    }
////    free(bsendBuff);
////    return MPI_SUCCESS;
////}
////
/////* receive citiesNum, xCoord and yCoord from root 
//// * NOTE:
//// * citiesNum, xCoord and yCoord are only variable names and we assume they
//// * will containe the correct values only after this function as requested 
//// * in the PDF exercise */
////int receive_initial_data(int *citiesNum, int *xCoord, int *yCoord) {
////
////    MPI_Status status;
////    int rank, rc1, rc2, rc3;
////    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
////
////    rc1 = MPI_Recv(citiesNum, 1, MPI_INT, /*src=*/0, TAG, MPI_COMM_WORLD,
////            &status);
////    rc2 = MPI_Recv(xCoord, *citiesNum, MPI_INT, /*src=*/0, TAG, MPI_COMM_WORLD,
////            &status);
////    rc3 = MPI_Recv(yCoord, *citiesNum, MPI_INT, /*src=*/0, TAG, MPI_COMM_WORLD,
////            &status);
////
////    assert(rc1 == MPI_SUCCESS && rc2 == MPI_SUCCESS && rc3 == MPI_SUCCESS);
////    return MPI_SUCCESS;
////}
////
////
/////* send the data to other tasks - rank{1, 2,...numTasks}.
//// * NOTE:
//// * this fuction send all it gots, it is not aware that a part of the Arr
//// * stays for root task */
////int send_data(State *dataArr, int numStates, int citiesNum,
////        int numStatesForRootTask, MPI_Datatype stateTypeName) {
////
////    /* get num of tasks */
////    int numTasks;
////    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
////
////    int elementSize = sizeof(State) + (citiesNum-1) * sizeof(int);
////    int numStatesToSend = numStates - numStatesForRootTask;
////
////    /* allocate buffers for MPI_Bsend */
////    int bsendBuffSize = numStatesToSend * elementSize + MPI_BSEND_OVERHEAD;
////    int *bsendBuff = malloc(bsendBuffSize);
////    assert(bsendBuff);
////
////    /* attach buffer for MPI_Bsend() */
////    MPI_Buffer_attach(bsendBuff, bsendBuffSize);
////
////    int minSendSize = numStatesToSend / (numTasks-1);
////    int tasksNumToSendMore = numStatesToSend % (numTasks-1);
////
////    void *start = (char*)dataArr + numStatesForRootTask * elementSize;
////    for (int i=1 ; i<numTasks ; i++) {
////        if (i <= tasksNumToSendMore) {
////            MPI_Bsend(start, minSendSize+1, stateTypeName, i, TAG, MPI_COMM_WORLD);
////            start = (char*)start + (minSendSize+1) * elementSize;
////        } else {
////            MPI_Bsend(start, minSendSize, stateTypeName, i, TAG, MPI_COMM_WORLD);
////            start = (char*)start + minSendSize * elementSize;
////        }
////    }
////
////    /* free memeory allocated */
////    free(bsendBuff);
////
////    return MPI_SUCCESS;
////}
////
////
/* the root task send all the data to other tasks, wait to all the data to be
 * received on other task, compute a sub problem itself, gather the results
 * of all other tasks and return the best result */
int rootExec(int citiesNum, int **agencyMatrix, MPI_Datatype stateTypeName,
        int *shortestPath) {

    int numTasks, rc/*, count*/;
    //MPI_Status status;
    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);

    /* keep the best len found and the best coresponding path */
    int minPathLen = INF;
    //shortestPath - define as function input
    
    /* create first numTasks States - the first one is a deme State for root 
     * NOTE:
     * at this initial stage we will send only prefixed in collective
     * communication because the MPI library does'n suppurt MPI_Gather with
     * complex data type as State struct */
    void *statesArr = allocate_array_of_states(numTasks, citiesNum, agencyMatrix);
    int *nextPrefix = init_array_of_states(statesArr, numTasks, citiesNum,
            PREFIX_LEN(citiesNum), agencyMatrix);

    //FIXME:remove
    //int x[] = {0, 1, 2, 3, 4, 5, 6, 7}; 
    //int y;
    //MPI_Scatter(x, 1, MPI_INT, &y, 1, MPI_INT, /*root=*/0, MPI_COMM_WORLD);
    
    /* scatter the buffer - collective communicatioin for the first send */
    //int *emptyIntArr = allocate_empty_array_of_int(citiesNum);
    //State *emptyState = create_state(NOT_SET, citiesNum, emptyIntArr, agencyMatrix);
    //rc = MPI_Scatter(statesArr, 1, stateTypeName, emptyState, 1, stateTypeName,
    //        /*root=*/0, MPI_COMM_WORLD);
    //assert(rc);


    //do {

    //    /* get the next state */

    //    /* wait for State request - new State, new bound or path result */
    //    
    //    /* handle the request */



    //    break;

    //} while (true);



    /* free the memory allocated */
    free_array_of_int(nextPrefix);
    free_array_of_states(statesArr);

    /* wait for all tasks to receive the end messages before exiting */
    MPI_Barrier(MPI_COMM_WORLD);
    printf("cpu %d : continued after barrier\n", 0);

    return minPathLen;
}

/* receive the data from the root task, compute a sub problem and return the
 * best local result to root task */
void otherExec(int citiesNum, int **agencyMatrix, MPI_Datatype stateTypeName,
        int *shortestPath) {

    int rank, rc/*, count*/;
    MPI_Status status;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* receive the first collective send from root */
    int *emptyIntArr = allocate_empty_array_of_int(citiesNum);
    State *state = create_state(NOT_SET, citiesNum, emptyIntArr, agencyMatrix);
    rc = MPI_Scatter(NULL, 1, stateTypeName, state, 1, stateTypeName,
            /*root=*/0, MPI_COMM_WORLD);
    assert(rc);
    //printf("cpu %d received ", rank);
    //print_state(state, citiesNum);
    //printf("\n");
    //int x[] = {0, 0, 0, 0, 0, 0, 0, 0}; 
    //int x;
    //MPI_Scatter(NULL, 1, MPI_INT, &x, 1, MPI_INT, /*root=*/0, MPI_COMM_WORLD);
    //printf("cpu %d received --> %d\n", rank, x);



    ///* reciver message size */
    //int rc1 = MPI_Probe(/*src=*/0, TAG, MPI_COMM_WORLD, &status);
    //int rc2 = MPI_Get_count(&status, stateTypeName, &count);
    //assert(rc1 == MPI_SUCCESS && rc2 == MPI_SUCCESS);

    ///* allocate receive buffer and receive the data from root task */
    //int elementSize = sizeof(State) + (citiesNum-1) * sizeof(int);
    //void *recvBuff = malloc(count * elementSize);
    //assert(recvBuff);

    //MPI_Recv(recvBuff, count, stateTypeName, /*src=*/0, TAG, MPI_COMM_WORLD,
    //        &status);

    /* wait for all tasks to receive the data */
    MPI_Barrier(MPI_COMM_WORLD);
    //printf("cpu %d : continued after barrier\n", rank);

    ///* compute a sub problems and keep the best one */
    //int minPathLen = INF;
    //int pathLenLocal = INF;
    //int *shortestPathLocal = allocate_empty_array_of_int(citiesNum);
    //int *emptyIntArr = allocate_empty_array_of_int(citiesNum);
    //State *it = create_state(NOT_SET, citiesNum, emptyIntArr, agencyMatrix);
    //for (int i=0 ; i<count ; i++) {
    //    copy_state_from_array_of_states(recvBuff, i, citiesNum, it);
    //    cpu_main(it, citiesNum, agencyMatrix, &pathLenLocal, shortestPathLocal);
    //    if (pathLenLocal < minPathLen) {
    //        minPathLen = pathLenLocal;
    //        copy_array_of_int(shortestPathLocal, shortestPath, citiesNum);
    //    }
    //}
    //free_state(it);
    //free_array_of_int(shortestPathLocal);
    //free(recvBuff);

    ///* send the result to root task */
    //MPI_Gather(shortestPath, citiesNum, MPI_INT, NULL, citiesNum, MPI_INT, 0,
    //        MPI_COMM_WORLD);

    /* free memory */
    free_state(state);

}



// The dynamic parellel algorithm main function.
int tsp_main(int citiesNum, int xCoord[], int yCoord[], int shortestPath[])
{
    int numTasks, rank/*, rc*/;
    MPI_Datatype stateTypeName;

    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* as requested, if the program is called with a signe CPU - exit */
    if (numTasks <= 1)
        exit(1);

    /* create agencyMatrix */
    int **agencyMatrix = create_agency_matrix(xCoord, yCoord, citiesNum);

    /* create rootState for registering State struct to mpi library */
    int *shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
    shortestPathUntilNow[0] = 0;
    State *rootState = create_state(0, citiesNum, shortestPathUntilNow,
            agencyMatrix);

    /* register State struct to MPI library */
    build_state_type(rootState, citiesNum, &stateTypeName);

    /* master-slave routine */
    int minPathLen = INF;
    if (rank == 0) {
        minPathLen = rootExec(citiesNum, agencyMatrix, stateTypeName, shortestPath);
    } else {
        otherExec(citiesNum, agencyMatrix, stateTypeName, shortestPath);
    }

    /* free memory allocated */
    free_state(rootState);
    free_agency_matrix(agencyMatrix, citiesNum);
    MPI_Type_free(&stateTypeName);

    /* return the result */
    if (rank != 0)
        return 0;

    return minPathLen;
}



//int main() {
//
//
//    /* are_equal_array_of_int test */
//    int a1[] = {1, 2, 3, 5};
//    int a2[] = {1, 2, 3, 5};
//    assert(are_equal_array_of_int(a1, a2, 4));
//    int a3[] = {0, 2, 3, 5};
//    assert(!are_equal_array_of_int(a1, a3, 4));
//
//
//
//
//    /* increas_prefix test */
//    int prefixLen = 4;
//    int *prefix = allocate_empty_array_of_int(prefixLen);
//    for(int i=0 ; i<prefixLen ; i++) {
//        prefix[i] = i;
//    }
//
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 1);
//    assert(prefix[2] == 2);
//    assert(prefix[3] == 3);
//
//    int citiesNum = 4;
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 1);
//    assert(prefix[2] == 3);
//    assert(prefix[3] == 2);
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 2);
//    assert(prefix[2] == 1);
//    assert(prefix[3] == 3);
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 2);
//    assert(prefix[2] == 3);
//    assert(prefix[3] == 1);
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 3);
//    assert(prefix[2] == 1);
//    assert(prefix[3] == 2);
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 3);
//    assert(prefix[2] == 2);
//    assert(prefix[3] == 1);
//
//    prefix[0] = 0;
//    prefix[1] = 1;
//    prefix[2] = 2;
//    prefix[3] = 3;
//
//    citiesNum = 6;
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 1);
//    assert(prefix[2] == 2);
//    assert(prefix[3] == 4);
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 1);
//    assert(prefix[2] == 2);
//    assert(prefix[3] == 5);
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 1);
//    assert(prefix[2] == 3);
//    assert(prefix[3] == 2);
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 1);
//    assert(prefix[2] == 3);
//    assert(prefix[3] == 4);
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 1);
//    assert(prefix[2] == 3);
//    assert(prefix[3] == 5);
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 1);
//    assert(prefix[2] == 4);
//    assert(prefix[3] == 2);
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 1);
//    assert(prefix[2] == 4);
//    assert(prefix[3] == 3);
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 1);
//    assert(prefix[2] == 4);
//    assert(prefix[3] == 5);
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 1);
//    assert(prefix[2] == 5);
//    assert(prefix[3] == 2);
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 1);
//    assert(prefix[2] == 5);
//    assert(prefix[3] == 3);
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 1);
//    assert(prefix[2] == 5);
//    assert(prefix[3] == 4);
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 2);
//    assert(prefix[2] == 1);
//    assert(prefix[3] == 3);
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 2);
//    assert(prefix[2] == 1);
//    assert(prefix[3] == 4);
//
//    free_array_of_int(prefix);
//
//
//
//
//
//    /* init_array_of_states test */
//    int numTasks = 8;
//    citiesNum = 6;
//    int xCoord[] = {0, 1, 2, 3, 4, 5};
//    int yCoord[] = {0, 0, 0, 0, 0, 0};
//    int **agencyMatrix = create_agency_matrix(xCoord, yCoord, citiesNum);
//    void *statesArr = allocate_array_of_states(numTasks, citiesNum, agencyMatrix);
//    int *nextPrefix = init_array_of_states(statesArr, numTasks, citiesNum,
//            /*prefixLen=*/4, agencyMatrix);
//    int requestedRes[] = {0, 1, 4, 3};
//    assert(are_equal_array_of_int(nextPrefix, requestedRes, /*prefixLen=*/4));
//    free_array_of_int(nextPrefix);
//
//    int *shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
//    shortestPathUntilNow[0];
//    State *state = create_state(0, citiesNum, shortestPathUntilNow, agencyMatrix);
//    
//    copy_state_from_array_of_states(statesArr, 0, citiesNum, state);
//    assert(state->vertex == NOT_SET);
//    assert(state->cost == 0);
//    assert(state->shortestPathUntilNow[0] == NOT_SET);
//    assert(state->shortestPathUntilNow[1] == NOT_SET);
//    assert(state->shortestPathUntilNow[2] == NOT_SET);
//    assert(state->shortestPathUntilNow[3] == NOT_SET);
//
//    copy_state_from_array_of_states(statesArr, 1, citiesNum, state);
//    assert(state->vertex == 3);
//    assert(state->cost == 9);
//    assert(state->shortestPathUntilNow[0] == 0);
//    assert(state->shortestPathUntilNow[1] == 1);
//    assert(state->shortestPathUntilNow[2] == 2);
//    assert(state->shortestPathUntilNow[3] == 3);
//
//    copy_state_from_array_of_states(statesArr, 2, citiesNum, state);
//    assert(state->vertex == 4);
//    assert(state->cost == 11);
//    assert(state->shortestPathUntilNow[0] == 0);
//    assert(state->shortestPathUntilNow[1] == 1);
//    assert(state->shortestPathUntilNow[2] == 2);
//    assert(state->shortestPathUntilNow[3] == 4);
//    
//    copy_state_from_array_of_states(statesArr, 7, citiesNum, state);
//    assert(state->vertex == 2);
//    assert(state->cost == 15);
//    assert(state->shortestPathUntilNow[0] == 0);
//    assert(state->shortestPathUntilNow[1] == 1);
//    assert(state->shortestPathUntilNow[2] == 4);
//    assert(state->shortestPathUntilNow[3] == 2);
//
//    free_agency_matrix(agencyMatrix, citiesNum);
//
//    citiesNum = 4;
//    numTasks = 7;
//    agencyMatrix = create_agency_matrix(xCoord, yCoord, citiesNum);
//    nextPrefix = init_array_of_states(statesArr, numTasks, citiesNum,
//            /*prefixLen=*/4, agencyMatrix);
//    int requestedRes2[] = {1, 0, 2, 3};
//    assert(are_equal_array_of_int(nextPrefix, requestedRes2, /*prefixLen=*/4));
//    free_array_of_int(nextPrefix);
//
//    copy_state_from_array_of_states(statesArr, 1, citiesNum, state);
//    assert(state->vertex == 3);
//    assert(state->cost == 9);
//    assert(state->shortestPathUntilNow[0] == 0);
//    assert(state->shortestPathUntilNow[1] == 1);
//    assert(state->shortestPathUntilNow[2] == 2);
//    assert(state->shortestPathUntilNow[3] == 3);
//
//    copy_state_from_array_of_states(statesArr, 2, citiesNum, state);
//    assert(state->vertex == 2);
//    assert(state->cost == 11);
//    assert(state->shortestPathUntilNow[0] == 0);
//    assert(state->shortestPathUntilNow[1] == 1);
//    assert(state->shortestPathUntilNow[2] == 3);
//    assert(state->shortestPathUntilNow[3] == 2);
//
//    copy_state_from_array_of_states(statesArr, 3, citiesNum, state);
//    assert(state->vertex == 3);
//    assert(state->cost == 13);
//    assert(state->shortestPathUntilNow[0] == 0);
//    assert(state->shortestPathUntilNow[1] == 2);
//    assert(state->shortestPathUntilNow[2] == 1);
//    assert(state->shortestPathUntilNow[3] == 3);
//
//    copy_state_from_array_of_states(statesArr, 4, citiesNum, state);
//    assert(state->vertex == 1);
//    assert(state->cost == 13);
//    assert(state->shortestPathUntilNow[0] == 0);
//    assert(state->shortestPathUntilNow[1] == 2);
//    assert(state->shortestPathUntilNow[2] == 3);
//    assert(state->shortestPathUntilNow[3] == 1);
//
//    copy_state_from_array_of_states(statesArr, 5, citiesNum, state);
//    assert(state->vertex == 2);
//    assert(state->cost == 15);
//    assert(state->shortestPathUntilNow[0] == 0);
//    assert(state->shortestPathUntilNow[1] == 3);
//    assert(state->shortestPathUntilNow[2] == 1);
//    assert(state->shortestPathUntilNow[3] == 2);
//
//    copy_state_from_array_of_states(statesArr, 6, citiesNum, state);
//    assert(state->vertex == 1);
//    assert(state->cost == 13);
//    assert(state->shortestPathUntilNow[0] == 0);
//    assert(state->shortestPathUntilNow[1] == 3);
//    assert(state->shortestPathUntilNow[2] == 2);
//    assert(state->shortestPathUntilNow[3] == 1);
//
//    free_state(state);
//    free_array_of_states(statesArr);
//    free_agency_matrix(agencyMatrix, citiesNum);
//
//
//
//}













