# include <stdio.h>
# include <pthread.h>
# include <stdlib.h>
# include <math.h>

// the number of rows & columns the 2D array has
#define n 4

// the 2D array to be sorted
int a[n][n];
FILE *myFile;

// number of times the sorting function has been called so far
int phase = 0;

// max number of times to run through the sorting function before the array has been sorted
int max_phases;

// the condition varaible
int phase_synch_count = 0;


pthread_t threads[n];
pthread_mutex_t count_mutex;
pthread_cond_t count_threshold_cv;


//Go through the array, printing each value
int printArray(int a[][n])	{
	for (int i = 0; i < n; i++)	{
		printf("\n");
		for (int j = 0; j < n; j++)	{
			printf("%d ", a[i][j]);
		}
	}
	printf("\n\n");
}

// Sort the row from left to right (i.e. smallest to largest)
int bubblesortRowOdd(int num)	{
	
	int temp;
	int i,j;
	
	for (i = 0; i < n-1; i++)	{
		for (j = 0; j < n-i-1; j++)	{
			if (a[num][j] > a[num][j+1])	{
				temp = a[num][j+1];
				a[num][j+1] = a[num][j];
				a[num][j] = temp;					
			}
		}
	}
}
// Sort the row from right to left (i.e. largest to smallest)
int bubblesortRowEven(int num)	{

	int temp;
	int i,j;
	
	for (i = 0; i < n-1; i++)	{
		for (j = n - 1; j > i; j--)	{
			if (a[num][j] > a[num][j-1])	{
			temp = a[num][j-1];
			a[num][j-1] = a[num][j];
			a[num][j] = temp;					
			}
		}
	}
}
// sort the columns from top to bottom (i.e. smallest to largest)
int bubblesortCol(int num)	{
	
	int temp;
	int i,j;
	
	for (i = 0; i < n-1; i++)	{
		for (j = 0; j < n-i-1; j++)	{
			if (a[j][num] > a[j+1][num])	{
				temp = a[j+1][num];
				a[j+1][num] = a[j][num];
				a[j][num] = temp;					
			}
		}
	}
}

// the function called and run by the threads declared in main()
// This is responsible for sorting the array
void *shearsort(void *threadid)	{
	
	long thread_id;
    thread_id = (long)threadid;
	
	while (phase < max_phases)	{
		//lock the mutix before uenterign the critical section of code (i.e. the shared resources)
		pthread_mutex_lock(&count_mutex);
		
		// critical section
	 
		// operate on columns
		if ((phase + 1) % 2 == 0) {
			printf("Working on Col | thread #%ld!\n", thread_id);
			bubblesortCol(thread_id);
			
		// operate on rows
		} else {
			// operate from right to left
			if ((thread_id + 1) % 2 == 0)	{
				printf("Working on RowEven | thread #%ld!\n", thread_id);
				bubblesortRowEven(thread_id);
			}else{
			// operate from left to right
				printf("Working on RowOdd | thread #%ld!\n", thread_id);
				bubblesortRowOdd(thread_id);
			}
		}

		// used to signify when a thread is done
		phase_synch_count++;
		
		// the last thread to finish calls this statement
		if (phase_synch_count == n)	{
			printArray(a);
			printf("Finished Phase #%d | Starting next phase | thread #%ld!\n\n", phase, thread_id);
			phase++;
			phase_synch_count = 0;
			// wake up all the threads that are blocked
			pthread_cond_broadcast(&count_threshold_cv);
			
		}else{
			printf("Now waiting | thread #%ld!\n", thread_id);
			// block all threads
			// when pthread_cond_broadcast is called, they become unblocked (one at a time)
			pthread_cond_wait(&count_threshold_cv, &count_mutex);
		}
			
		pthread_mutex_unlock(&count_mutex);
	}
	
	printf("All Done | thread #%ld!\n", thread_id);
	pthread_exit(NULL);
}


int main()	{
	int rc, holder;
	long t;
	
	// number of phases to sort array of size N (N = n x n) is log_2(N)+1
	max_phases = (log(n*n) / log(2)) + 1;
	
	//declare which file will be opened
	myFile = fopen("input.txt", "r");
		
	// Go through the file, copying the integers into the array
	for (int i = 0; i < n; i++)	{
		for (int j = 0; j < n; j++)	{
			fscanf(myFile, "%d", &holder);
			a[i][j] = holder;
		}
	}
	//print the original, unsorted array
	printf("Original Array\n\n");
	printArray(a);
	
	
	/* Initialize mutex and condition variable objects */
   pthread_mutex_init(&count_mutex, NULL);
   pthread_cond_init (&count_threshold_cv, NULL);


	// Create n Threads with ID = t
	for(t = 0; t < n; t++){
		
       printf("In main: creating thread %ld\n", t);
	   
	   // pthread_create (thread,attr,start_routine,arg)
       rc = pthread_create(&threads[t], NULL, shearsort, (void *)t);
	   
	   // Handle any errors with creating the threads
       if (rc){
          printf("ERROR; return code from pthread_create() is %d\n", rc);
          exit(-1);
       }
	}
	
	/* Clean up and exit */
	pthread_mutex_destroy(&count_mutex);
	pthread_cond_destroy(&count_threshold_cv);
	pthread_exit(NULL);
}