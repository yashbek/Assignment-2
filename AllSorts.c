#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <pthread.h>

#define NARRAY 1000 // Array size
#define NBUCKET 10  // Number of buckets
#define INTERVAL 100  // Each bucket capacity

int array1[NARRAY];
int array2[NARRAY];
int array3[NARRAY];

struct point {
  int data;
  struct point *next;
};

void BucketSort(int arr[]);
struct point *InsertionSort(struct point *Arr);
void print(int arr[]);
void printBuckets(struct point *Arr);
int isSorted(int* A, int size);

void BucketSort(int arr[]) {
  int i, j;
  struct point **buckets;

  buckets = (struct point **)malloc(sizeof(struct point *) * NBUCKET);

  for (i = 0; i < NBUCKET; ++i) {
    buckets[i] = NULL;
  }

  for (i = 0; i < NARRAY; ++i) {
    struct point *current;
    int index = arr[i]/INTERVAL;
    current = (struct point *)malloc(sizeof(struct point));
    current->data = arr[i];
    current->next = buckets[index];
    buckets[index] = current;
  }

  for (i = 0; i < NBUCKET; ++i) {
    buckets[i] = InsertionSort(buckets[i]);
  }

  for (j = 0, i = 0; i < NBUCKET; ++i) {
    struct point *node;
    node = buckets[i];
    while (node) {
      arr[j++] = node->data;
      node = node->next;
    }
  }
  return;
}

void OMPBucketSort(int arr[]) {
  int i, j;
  struct point **buckets;

  buckets = (struct point **)malloc(sizeof(struct point *) * NBUCKET);

  for (i = 0; i < NBUCKET; ++i) {
    buckets[i] = NULL;
  }
  
  #pragma omp parallel
  {
    #pragma omp for
    for (i = 0; i < NARRAY; ++i) {
        struct point *current;
        int index = arr[i]/INTERVAL;
        current = (struct point *)malloc(sizeof(struct point));
        current->data = arr[i];
    
        #pragma omp critical
        {
            current->next = buckets[index];
            buckets[index] = current;
        }
        
    }

    #pragma omp for
        for (i = 0; i < NBUCKET; ++i) {
            buckets[i] = InsertionSort(buckets[i]);
        }        
  }
  for (j = 0, i = 0; i < NBUCKET; ++i) {
            struct point *node;
            node = buckets[i];
            while (node) {
                arr[j++] = node->data;
                node = node->next;
            }
        }
  return;
}

struct point **gBuckets;
pthread_mutex_t ifil;

void* run(void* id){
    long ID = (long)id;
    struct point *current;
    for(int i = 0; i<NARRAY/INTERVAL; i++){
        int index = array3[INTERVAL*ID + i]/INTERVAL;
        current = (struct point *)malloc(sizeof(struct point));
        current->data = array3[i];
        pthread_mutex_lock(&ifil);
        current->next = gBuckets[index];
        gBuckets[index] = current;
        pthread_mutex_unlock(&ifil);
    }
    gBuckets[ID] = InsertionSort(gBuckets[ID]);
}

void PTBucketSort() {
  int i, j = 0;
  
  gBuckets = (struct point **)malloc(sizeof(struct point *) * NBUCKET);
  

  for (i = 0; i < NBUCKET; ++i) {
    gBuckets[i] = NULL;
  }
  pthread_t th[NBUCKET];
  for (long k = 0; k < NBUCKET; k++){
    pthread_create(&th[k], NULL, run, (void *)k);
  }
  for (long k = 0; k < NBUCKET; k++){
    pthread_join(th[k], NULL);
  }
    for ( i = 0; i < NBUCKET; ++i) {
        struct point *node;
        node = gBuckets[i];
        while (node) {
            array3[j++] = node->data;
            node = node->next;
        }
    }
  return;
}

struct point *InsertionSort(struct point *Arr) {
  struct point *k, *points;
  if (Arr == 0 || Arr->next == 0) {
    return Arr;
  }

  points = Arr;
  k = Arr->next;
  points->next = 0;
  while (k != 0) {
    struct point *ptr;
    if (points->data > k->data) {
      struct point *tmp;
      tmp = k;
      k = k->next;
      tmp->next = points;
      points = tmp;
      continue;
    }

    for (ptr = points; ptr->next != 0; ptr = ptr->next) {
      if (ptr->next->data > k->data)
        break;
    }

    if (ptr->next != 0) {
      struct point *tmp;
      tmp = k;
      k = k->next;
      tmp->next = ptr->next;
      ptr->next = tmp;
      continue;
    } else {
      ptr->next = k;
      k = k->next;
      ptr->next->next = 0;
      continue;
    }
  }
  return points;
}

int isSorted(int* A, int n) {
  for (int i = 1; i < n; i++){
    if(A[i-1]>A[i]){
        return 0;
    }
  }
  return 1;
}

void print(int ar[]) {
  int i;
  for (i = 0; i < NARRAY; ++i) {
    printf("%d ", ar[i]);
  }
  printf("\n");
}

// Driver code
int main(void) {
  srand(time(NULL)); 
  for(int i = 0; i < NARRAY; i++)
  {
    int val = rand() % (NARRAY);
    array1[i] = val;
    array2[i] = val;
    array3[i] = val;
  }
  printf("-------------\n");
  clock_t seq = clock();

  BucketSort(array1);

  double seqtime = clock() - seq;
  printf("seq took %f seconds",seqtime/CLOCKS_PER_SEC);
  int seqFlag = isSorted(array1, NARRAY);
  if (seqFlag == 1){
    printf("\nSorted\n");
  }

  printf("-------------\n");
  clock_t omp = clock();

  OMPBucketSort(array2);
  double omptime = clock() - omp;
  printf("omp took %f seconds", omptime/CLOCKS_PER_SEC);
  int ompFlag = isSorted(array2, NARRAY);
//   print(array2);
  if (ompFlag == 1){
    printf("\nSorted\n");
  }
  printf("-------------\n");
  clock_t pt = clock();

  PTBucketSort(); //operates on array3 by default
  double pttime = clock() - pt;
  printf("pthreads took %f seconds", pttime/CLOCKS_PER_SEC);
  int ptFlag = isSorted(array2, NARRAY);
//   print(array2);
  if (ptFlag == 1){
    printf("\nSorted\n");
  }
  
  return 0;
}