#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>


#define INF 3000000
#define NOT_SET -1
#define ERROR -2
#define TAG 99
#define BOUND_TAG 100
#define JOB_TAG 101
#define JOB_REQUEST_TAG 102

#define PREFIX_LEN(citiesNum)   ((citiesNum)-3)
#define MIN(a, b)   (((a) < (b)) ? (a) : (b))



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
 * NOTES:
 * - if the  given prefix is the last one (starting with 0) then the prefix is
 *   updated to [NOT_SET, NOT_SET, ..., NOT_SET]
 * - if the  given prefix is [NOT_SET, NOT_SET, ..., NOT_SET] then the prefix
 *   remain untached.
 * ASSERT that prefix[0] == (0 || NOT_SET) */
void increas_prefix(int *prefix, int prefixLen, int citiesNum) {
   
    assert(prefix[0] == 0 || prefix[0] == NOT_SET);

    /* check if prefix is an empty prefix */
    int *tmpPrefix = allocate_empty_array_of_int(citiesNum);
    bool isEmpty = are_equal_array_of_int(tmpPrefix, prefix, citiesNum);
    if (isEmpty) {
        free_array_of_int(tmpPrefix);
        return;
    }

    /* check if prefix is the last prefix */
    tmpPrefix[0] = 0;
    for (int i=1 ; i<prefixLen ; i++) {
        tmpPrefix[i] = citiesNum - i;
    }
    bool isLast = are_equal_array_of_int(tmpPrefix, prefix, citiesNum);
    if (isLast) {
        free_array_of_int(tmpPrefix);
        for (int i=0 ; i<citiesNum ; i++) {
            prefix[i] = NOT_SET;
        }
        return;
    }

    /* if the prefix is a non-empty non-last prefix, is been increased */
    bool foundCurrPrefix = false, foundNextPrefix = false;
    int *newPrefix = allocate_empty_array_of_int(prefixLen);

    increas_prefix_rec(prefix, newPrefix, prefixLen, citiesNum, 0,
            &foundCurrPrefix, &foundNextPrefix); 

    copy_array_of_int(newPrefix, prefix, prefixLen);
    free_array_of_int(newPrefix);
    free_array_of_int(tmpPrefix);
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
            free_state(son);
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


/* start a async listeninig to new bound updates */
void listen_bound_update_async(MPI_Request *request, int *newBound) {

    int rc = MPI_Irecv(newBound, 1, MPI_INT, MPI_ANY_SOURCE, BOUND_TAG,
            MPI_COMM_WORLD, request); 
    assert(rc == MPI_SUCCESS);
}

/* start a async listeninig to new job requests */
void listen_job_request_async(MPI_Request *request) {

    int jobTmp;
    int rc = MPI_Irecv(&jobTmp, 1, MPI_INT, MPI_ANY_SOURCE, JOB_REQUEST_TAG,
            MPI_COMM_WORLD, request); 
    assert(rc == MPI_SUCCESS);
}

/* test if a bound update has arrived and notify all tasks if needed
 * return:
 * true if new bound arrived and false otherwise */
bool test_and_handle_bound_update(MPI_Request *request, int *minPathLen,
        int *newBound) {

    MPI_Status status;
    MPI_Request broadcastRequest;
    int boundArrived = false;

    int rc = MPI_Test(request, &boundArrived, &status);
    assert(rc == MPI_SUCCESS);

    if (boundArrived) {
        if (*newBound < *minPathLen) {
            *minPathLen = *newBound;
            MPI_Ibcast(minPathLen, 1, MPI_INT, /*root=*/0, MPI_COMM_WORLD,
                    &broadcastRequest);
            //printf("cpu %d : broadcast bound = %d\n", 0, *minPathLen);
        }
    }

    return boundArrived;
}

/* test if a job request has arrived and and send new job if needed
 * return:
 * true if new job request arrived and false otherwise */
bool test_and_handle_job_request(MPI_Request *request, State *nextState,
        MPI_Datatype stateTypeName) { 

    MPI_Status status;
    int jobRequestArrived = false;

    int rc = MPI_Test(request, &jobRequestArrived, &status);
    assert(rc == MPI_SUCCESS);

    if (jobRequestArrived) {
        rc = MPI_Ssend(nextState, 1, stateTypeName, status.MPI_SOURCE, JOB_TAG,
                MPI_COMM_WORLD); 
        assert(rc == MPI_SUCCESS);
    }

    return jobRequestArrived;
}

/* the root task send all the data to other tasks, wait to all the data to be
 * received on other task, compute a sub problem itself, gather the results
 * of all other tasks and return the best result */
int rootExec(int citiesNum, int **agencyMatrix, MPI_Datatype stateTypeName,
        int *shortestPath) {

    int numTasks;
    int numEmptyStatesSent = 0;
    MPI_Request boundRequest, jobRequest;
    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);

    /* keep the best len found and the best coresponding path */
    int *minPathLen = malloc(sizeof(int)); // set on heap beacause it is 
    *minPathLen = INF;                     // asyncronusly sent
    //shortestPath - define as function input
    
    /* create the first prefix [0, 1, ..., prefixLen-1, NOT_SET, ... NOT_SET] */
    int *nextPrefix = allocate_empty_array_of_int(citiesNum);
    for (int i=0 ; i <PREFIX_LEN(citiesNum) ; i++) {
        nextPrefix[i] = i;
    }

    /* listen to new bound update and job request asyncronusly */
    int minPathLenTmp;
    listen_bound_update_async(&boundRequest, &minPathLenTmp);
    listen_job_request_async(&jobRequest);
    
    /* start the main routine */
    bool needJob = true;
    State *nextState;
    while ( numEmptyStatesSent < numTasks-1 ) {

        /* create the next state - may be an empty state if we finished */
        if (needJob) {

            int *shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
            copy_array_of_int(nextPrefix, shortestPathUntilNow, citiesNum);
            if (shortestPathUntilNow[0] == NOT_SET) {
                nextState = create_state(NOT_SET, citiesNum, shortestPathUntilNow,
                        agencyMatrix);
                numEmptyStatesSent++;            
            } else {
                nextState = create_state(shortestPathUntilNow[PREFIX_LEN(citiesNum)-1],
                        citiesNum, shortestPathUntilNow, agencyMatrix);
            }

            increas_prefix(nextPrefix, PREFIX_LEN(citiesNum), citiesNum);
            needJob = false;
        }

        /* check if we got new requests, handle it and renew listening */
        if (test_and_handle_bound_update(&boundRequest, minPathLen,
                    &minPathLenTmp))
            listen_bound_update_async(&boundRequest, &minPathLenTmp);

        if (test_and_handle_job_request(&jobRequest, nextState, stateTypeName)) {
            listen_job_request_async(&jobRequest);
            needJob = true;
        }

        /* free memory */
        if (needJob)
            free_state(nextState);
    }

    /* wait for all tasks to receive the end messages before exiting */
    MPI_Barrier(MPI_COMM_WORLD);
    //printf("cpu %d : barrier PASSED\n", 0);
    //printf("cpu %d : minPathLen = %d\n", 0, *minPathLen);
    
    /* gather the result */
    int *tmpShortestPath = allocate_empty_array_of_int(citiesNum);
    int *allShortestPaths =
        allocate_empty_array_of_int(numTasks * citiesNum * sizeof(int));
    MPI_Gather(tmpShortestPath, citiesNum, MPI_INT, allShortestPaths, citiesNum,
            MPI_INT, /*root=*/0, MPI_COMM_WORLD);
    //printf("cpu %d : allShortestPaths = ", 0);
    //print_array_of_int(allShortestPaths, citiesNum * numTasks);

    /* all the tasks have the same minPathLen so all shortestPaths should have
     * the same len.
     * tasks that didn't find a path with the require minPathLen have returne
     * [NOT_SET, ..., NOT_SET] to the root task.
     * we will look for the first non-empy shortestPath received */
    int *start = allShortestPaths;
    int *end = allShortestPaths + numTasks * citiesNum * sizeof(int);
    for (int *it = start ; it<end ; it += citiesNum) {
        if (it[0] != NOT_SET) {
            //printf("cpu %d shortestPathLocal len = %d\n",
            //        0, get_path_len(it, citiesNum, agencyMatrix));
            copy_array_of_int(it, shortestPath, citiesNum);
            //print_array_of_int(shortestPath, citiesNum);
            //printf("\n");
            break;

        }
    }
    assert(get_path_len(shortestPath, citiesNum, agencyMatrix) == *minPathLen);

    /* free the memory allocated */
    free_array_of_int(allShortestPaths);
    free_array_of_int(tmpShortestPath);
    free_array_of_int(nextPrefix);
    
    return *minPathLen;
}

/* receive the data from the root task, compute a sub problem and return the
 * best local result to root task */
void otherExec(int citiesNum, int **agencyMatrix, MPI_Datatype stateTypeName,
        int *shortestPath) {

    int rank;
    MPI_Status status;
    MPI_Request broadcastRequest, boundRequest;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* allocate a state to receive data */
    int *emptyIntArr = allocate_empty_array_of_int(citiesNum);
    State *state = create_state(NOT_SET, citiesNum, emptyIntArr, agencyMatrix);

    int *minPathLenLocal = malloc(sizeof(int)); // set on heap because it is 
    *minPathLenLocal = INF;                     // asyncronusly sent
    int minPathLenTmp, newBound;
    int *shortestPathLocal = allocate_empty_array_of_int(citiesNum);
    int *shortestPathTmp = allocate_empty_array_of_int(citiesNum);

    /* listen to bound update */
    MPI_Ibcast(&newBound, 1, MPI_INT, /*root=*/0, MPI_COMM_WORLD,
            &broadcastRequest);

    /* start main worker routine */
    bool finished = false;
    do {
        
        /* send a job request */
        int jobTmp;
        MPI_Ssend(&jobTmp, 1, MPI_INT, /*dst=*/0, JOB_REQUEST_TAG, MPI_COMM_WORLD);

        /* receive a new job */
        MPI_Recv(state, 1, stateTypeName, /*src=*/0, JOB_TAG, MPI_COMM_WORLD,
                &status);

        /* if the new state is a terminate-state (an empty state) then finish */
        if (state->vertex != NOT_SET) {
            minPathLenTmp = INF;
            cpu_main(state, citiesNum, agencyMatrix, &minPathLenTmp,
                    shortestPathTmp);
        } else
            finished = true;

        /* test new bound */
        int boundArrived = false;
        int rc = MPI_Test(&broadcastRequest, &boundArrived, &status);
        //int rc = MPI_Test(&boundRequest, &boundArrived, &status);
        assert(rc == MPI_SUCCESS);
        if (boundArrived) {
            //printf("cpu %d : new bound %d has arrived in broadcast from root\n", rank, newBound);
            if (newBound < *minPathLenLocal) {
                *minPathLenLocal = newBound;
                //printf("cpu %d : update minPathLenLocal = %d\n", rank, newBound);
                free_array_of_int(shortestPathLocal);
                shortestPathLocal = allocate_empty_array_of_int(citiesNum);
                //printf("cpu %d : shortestPathLocal was erased due to a new bound = %d\n",
                //        rank, newBound);
            }
            /* renew listening to bound update */
            MPI_Ibcast(&newBound, 1, MPI_INT, /*root=*/0, MPI_COMM_WORLD,
                    &broadcastRequest);
            //MPI_Ibcast(&minPathLenTmp, 1, MPI_INT, /*root=*/0, MPI_COMM_WORLD,
            //        &broadcastRequest);
        }

        if (state->vertex != NOT_SET) {
            /* if we found a better result than the curren one - notify the root */
            if (minPathLenTmp < *minPathLenLocal) {
                *minPathLenLocal = minPathLenTmp;
                copy_array_of_int(shortestPathTmp, shortestPathLocal, citiesNum);
                MPI_Issend(minPathLenLocal, 1, MPI_INT, /*dst=*/0,
                        BOUND_TAG, MPI_COMM_WORLD, &boundRequest);
                //printf("cpu %d send newBound = %d to root\n", rank, *minPathLenLocal);
            }
        }

    } while ( !finished );

    /* send the result to root task */
    MPI_Gather(shortestPathLocal, citiesNum, MPI_INT, NULL, citiesNum, MPI_INT,
            /*root=*/0, MPI_COMM_WORLD);
    //printf("cpu %d : sent shortestPathLocal to root\n", rank);

    /* wait for all tasks to terminate the computatioin */
    MPI_Barrier(MPI_COMM_WORLD);
    //printf("cpu %d : barrier PASSED\n", rank);

    /* free memory */
    free_state(state);
    free_array_of_int(shortestPathLocal);
    free_array_of_int(shortestPathTmp);
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
//    int citiesNum = 4;
//
//    int *prefix = allocate_empty_array_of_int(prefixLen);
//    increas_prefix(prefix, prefixLen, citiesNum);
//    int tmp[] = {NOT_SET, NOT_SET, NOT_SET, NOT_SET};
//    assert(are_equal_array_of_int(prefix, tmp, citiesNum));
//
//    for(int i=0 ; i<prefixLen ; i++) {
//        prefix[i] = i;
//    }
//    assert(prefix[0] == 0);
//    assert(prefix[1] == 1);
//    assert(prefix[2] == 2);
//    assert(prefix[3] == 3);
//
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
//    increas_prefix(prefix, prefixLen, citiesNum);
//    assert(prefix[0] == NOT_SET);
//    assert(prefix[1] == NOT_SET);
//    assert(prefix[2] == NOT_SET);
//    assert(prefix[3] == NOT_SET);
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
//    ////FIXME: do i need this?
//    ///* init_array_of_states test */
//    //int numTasks = 8;
//    //int citiesNum = 6;
//    //int xCoord[] = {0, 1, 2, 3, 4, 5};
//    //int yCoord[] = {0, 0, 0, 0, 0, 0};
//    //int **agencyMatrix = create_agency_matrix(xCoord, yCoord, citiesNum);
//    //void *statesArr = allocate_array_of_states(numTasks, citiesNum, agencyMatrix);
//    //int *nextPrefix = init_array_of_states(statesArr, numTasks, citiesNum,
//    //        /*prefixLen=*/4, agencyMatrix);
//    //int requestedRes[] = {0, 1, 4, 3};
//    //assert(are_equal_array_of_int(nextPrefix, requestedRes, /*prefixLen=*/4));
//    //free_array_of_int(nextPrefix);
//
//    //int *shortestPathUntilNow = allocate_empty_array_of_int(citiesNum);
//    //shortestPathUntilNow[0];
//    //State *state = create_state(0, citiesNum, shortestPathUntilNow, agencyMatrix);
//    //
//    //copy_state_from_array_of_states(statesArr, 0, citiesNum, state);
//    //assert(state->vertex == NOT_SET);
//    //assert(state->cost == 0);
//    //assert(state->shortestPathUntilNow[0] == NOT_SET);
//    //assert(state->shortestPathUntilNow[1] == NOT_SET);
//    //assert(state->shortestPathUntilNow[2] == NOT_SET);
//    //assert(state->shortestPathUntilNow[3] == NOT_SET);
//
//    //copy_state_from_array_of_states(statesArr, 1, citiesNum, state);
//    //assert(state->vertex == 3);
//    //assert(state->cost == 9);
//    //assert(state->shortestPathUntilNow[0] == 0);
//    //assert(state->shortestPathUntilNow[1] == 1);
//    //assert(state->shortestPathUntilNow[2] == 2);
//    //assert(state->shortestPathUntilNow[3] == 3);
//
//    //copy_state_from_array_of_states(statesArr, 2, citiesNum, state);
//    //assert(state->vertex == 4);
//    //assert(state->cost == 11);
//    //assert(state->shortestPathUntilNow[0] == 0);
//    //assert(state->shortestPathUntilNow[1] == 1);
//    //assert(state->shortestPathUntilNow[2] == 2);
//    //assert(state->shortestPathUntilNow[3] == 4);
//    //
//    //copy_state_from_array_of_states(statesArr, 7, citiesNum, state);
//    //assert(state->vertex == 2);
//    //assert(state->cost == 15);
//    //assert(state->shortestPathUntilNow[0] == 0);
//    //assert(state->shortestPathUntilNow[1] == 1);
//    //assert(state->shortestPathUntilNow[2] == 4);
//    //assert(state->shortestPathUntilNow[3] == 2);
//
//    //free_agency_matrix(agencyMatrix, citiesNum);
//
//    //citiesNum = 4;
//    //numTasks = 7;
//    //agencyMatrix = create_agency_matrix(xCoord, yCoord, citiesNum);
//    //nextPrefix = init_array_of_states(statesArr, numTasks, citiesNum,
//    //        /*prefixLen=*/4, agencyMatrix);
//    //int requestedRes2[] = {1, 0, 2, 3};
//    //assert(are_equal_array_of_int(nextPrefix, requestedRes2, /*prefixLen=*/4));
//    //free_array_of_int(nextPrefix);
//
//    //copy_state_from_array_of_states(statesArr, 1, citiesNum, state);
//    //assert(state->vertex == 3);
//    //assert(state->cost == 9);
//    //assert(state->shortestPathUntilNow[0] == 0);
//    //assert(state->shortestPathUntilNow[1] == 1);
//    //assert(state->shortestPathUntilNow[2] == 2);
//    //assert(state->shortestPathUntilNow[3] == 3);
//
//    //copy_state_from_array_of_states(statesArr, 2, citiesNum, state);
//    //assert(state->vertex == 2);
//    //assert(state->cost == 11);
//    //assert(state->shortestPathUntilNow[0] == 0);
//    //assert(state->shortestPathUntilNow[1] == 1);
//    //assert(state->shortestPathUntilNow[2] == 3);
//    //assert(state->shortestPathUntilNow[3] == 2);
//
//    //copy_state_from_array_of_states(statesArr, 3, citiesNum, state);
//    //assert(state->vertex == 3);
//    //assert(state->cost == 13);
//    //assert(state->shortestPathUntilNow[0] == 0);
//    //assert(state->shortestPathUntilNow[1] == 2);
//    //assert(state->shortestPathUntilNow[2] == 1);
//    //assert(state->shortestPathUntilNow[3] == 3);
//
//    //copy_state_from_array_of_states(statesArr, 4, citiesNum, state);
//    //assert(state->vertex == 1);
//    //assert(state->cost == 13);
//    //assert(state->shortestPathUntilNow[0] == 0);
//    //assert(state->shortestPathUntilNow[1] == 2);
//    //assert(state->shortestPathUntilNow[2] == 3);
//    //assert(state->shortestPathUntilNow[3] == 1);
//
//    //copy_state_from_array_of_states(statesArr, 5, citiesNum, state);
//    //assert(state->vertex == 2);
//    //assert(state->cost == 15);
//    //assert(state->shortestPathUntilNow[0] == 0);
//    //assert(state->shortestPathUntilNow[1] == 3);
//    //assert(state->shortestPathUntilNow[2] == 1);
//    //assert(state->shortestPathUntilNow[3] == 2);
//
//    //copy_state_from_array_of_states(statesArr, 6, citiesNum, state);
//    //assert(state->vertex == 1);
//    //assert(state->cost == 13);
//    //assert(state->shortestPathUntilNow[0] == 0);
//    //assert(state->shortestPathUntilNow[1] == 3);
//    //assert(state->shortestPathUntilNow[2] == 2);
//    //assert(state->shortestPathUntilNow[3] == 1);
//
//    //free_state(state);
//    //free_array_of_states(statesArr);
//    //free_agency_matrix(agencyMatrix, citiesNum);
//
//
//
//
//
//
//
//    //FIXME: do i need it ?
//    ///* init_array_of_prefixes test */
//    //int numTasks = 8;
//    //prefixLen = 4;
//    //citiesNum = 6;
//    //int xCoord[] = {0, 1, 2, 3, 4, 5};
//    //int yCoord[] = {0, 0, 0, 0, 0, 0};
//    //int *prefixesArr = allocate_array_of_prefixes(numTasks, citiesNum);
//
//    //int *nextPrefix = init_array_of_prefixes(prefixesArr, numTasks, citiesNum,
//    //        prefixLen);
//    //int requestedNextPrefix0[] = {0, 1, 4, 3};
//    //assert(are_equal_array_of_int(nextPrefix, requestedNextPrefix0, prefixLen));
//    //free_array_of_int(nextPrefix);
//
//    //int rs0[] = {NOT_SET, NOT_SET, NOT_SET, NOT_SET};
//    //assert(are_equal_array_of_int(prefixesArr, rs0, prefixLen));
//
//    //int rs1[] = {0, 1, 2, 3};
//    //assert(are_equal_array_of_int(prefixesArr + prefixLen, rs1, prefixLen));
//
//    //int rs2[] = {0, 1, 2, 4};
//    //assert(are_equal_array_of_int(prefixesArr + 2*prefixLen, rs2, prefixLen));
//
//    //int rs7[] = {0, 1, 4, 2};
//    //assert(are_equal_array_of_int(prefixesArr + 7*prefixLen, rs7, prefixLen));
//
//
//    //citiesNum = 4;
//    //numTasks = 7;
//    //nextPrefix = init_array_of_prefixes(prefixesArr, numTasks, citiesNum,
//    //        prefixLen);
//    //int requestedNextPrefix1[] = {1, 0, 2, 3};
//    //assert(are_equal_array_of_int(nextPrefix, requestedNextPrefix1, prefixLen));
//    //free_array_of_int(nextPrefix);
//
//    //int rs00[] = {NOT_SET, NOT_SET, NOT_SET, NOT_SET};
//    //assert(are_equal_array_of_int(prefixesArr, rs00, prefixLen));
//
//    //int rs01[] = {0, 1, 2, 3};
//    //assert(are_equal_array_of_int(prefixesArr + prefixLen, rs01, prefixLen));
//
//    //int rs02[] = {0, 1, 3, 2};
//    //assert(are_equal_array_of_int(prefixesArr + 2*prefixLen, rs02, prefixLen));
//
//    //int rs03[] = {0, 2, 1, 3};
//    //assert(are_equal_array_of_int(prefixesArr + 3*prefixLen, rs03, prefixLen));
//
//    //int rs04[] = {0, 2, 3, 1};
//    //assert(are_equal_array_of_int(prefixesArr + 4*prefixLen, rs04, prefixLen));
//
//    //int rs05[] = {0, 3, 1, 2};
//    //assert(are_equal_array_of_int(prefixesArr + 5*prefixLen, rs05, prefixLen));
//
//    //int rs06[] = {0, 3, 2, 1};
//    //assert(are_equal_array_of_int(prefixesArr + 6*prefixLen, rs06, prefixLen));
//
//    //free_array_of_prefixes(prefixesArr);
//}













