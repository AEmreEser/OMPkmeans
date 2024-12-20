#include <iostream>
#include <fstream>
#include <sstream>
#include <climits>      /* INT_MAX*/
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <cmath>
#include <omp.h>
using namespace std;

#define MAX_ITER 500

double inline dist(double x1, double y1, double x2, double y2) {
  return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);  // sqrt is omitted as it reduces performance.
}

/* Selecting k distinct, random centroids from n points. */
void randomCenters(int *&x, int *&y, int n, int k, double *&cx, double *&cy) {
  int *centroids = new int[k];

#ifdef RANDOM
  srand (time(NULL)); //normal code
  int added = 0;
  
  while(added != k) {
    bool exists = false;
    int temp = rand() % n;
    for(int i = 0; i < added; i++) {
      if(centroids[i] == temp) {
        exists = true;
      }
    }
    if(!exists) {
      cx[added] = x[temp];
      cy[added] = y[temp];
      centroids[added++] = temp;
    }
  }
#else //deterministic init
  for(int i = 0; i < k; i++) {
     cx[i] = x[i];
     cy[i] = y[i];
     centroids[i] = i;
  }
#endif
delete[] centroids;
}

bool assign(int *&x, int *&y, int *&c, double *&cx, double *&cy, int k, int n) {
  bool end = true;
  int * changed = new int[n]();

  omp_set_num_threads(omp_get_max_threads());
  #pragma omp parallel for // reduction(&:end) // Parallelize the assignment step
  for(int i = 0; i < n; i++) {
    int cluster;
    int minDist = INT_MAX;
    // Assign to closest center
    for(int j = 0; j < k; j++) {
      double distance = dist(cx[j], cy[j], x[i], y[i]);      
      if(distance < minDist) {
        minDist = distance;
        cluster = j;
      }
    }
    if(cluster != c[i]) { // even if one point changes, we will run for one more iteration.
      // end = false;
      changed[i] = 1;
      c[i] = cluster; // assign the point to the cluster with minDist
    }
    // REDUCTION FOR SOME REASON FAILS MISERABLY
    // even if one point changes, we will run for one more iteration.
    // end &= (cluster != c[i]);
    // c[i] = cluster; // assign the point to the cluster with minDist
  }

  int sum = 0;

  #pragma omp parallel for reduction(+: sum)
  for (int i = 0; i < n; i++){
    sum += changed[i];
  }

  delete[] changed;
  return (sum == 0); // if sum == 0 -> no changes --> end == true
  // return end;
}

void init(int *&x, int *&y, int *&c, double *&cx, double *&cy, int k, int n) {
  #ifdef ALIGNED_ALLOC
    cx = (double *) aligned_alloc(64, k * sizeof(double));
    cy = (double *) aligned_alloc(64, k * sizeof(double));
  #else
    cx = new double[k];
    cy = new double[k];
  #endif
  
  randomCenters(x, y, n, k, cx, cy);

  /* Initialize the cluster information for each point. */
  #ifdef ALIGNED_ALLOC
    c = (int *) aligned_alloc(64, n * sizeof(int)); // l1,2,3 cache line size == 64 bytes // preventing false sharing
  #else
    c = new int[n];
  #endif
  
  assign(x,y,c,cx,cy,k,n); // Assign each point to closest center.
}

void update(int *&x, int *&y, int *&c, double *&cx, double *&cy, int k, int n) {
  omp_set_num_threads(k); // max degree of concurrency here
  double sumxs = new double[k];
  double sumys = new double[k];
  int counts = new int[k];

  // THERE IS SMT STUPID GOING ON HERE!!!!
  #pragma omp parallel for
  for(int j = 0; j < n; j++) {
    double locsumx = 0, locsumy = 0;
    int loccount = 0;

    #pragma omp parallel for reduction(+:locsumx,locsumy,loccount)
    for(int i = 0; i < k; i++) {
      if(c[j] == i) {
        locsumx += x[j];
        locsumy += y[j];
        loccount+= 1;
      }
    } // for i

    // if(count > 0) {
    //   cx[i] = sumx / count;
    //   cy[i] = sumy / count;
    // }
  } // for j

  #pragma omp parallel for
  for(int i = 0; i < k; i++) {
    if(count > 0) {
      cx[i] = sumxs[i] / counts[i];
      cy[i] = sumys[i] / counts[i];
    } // if 
  } // for i 
}

int readfile(string fname, int *&x, int *&y) {
  ifstream f;
  f.open(fname.c_str());
  string line;
  getline(f,line);
  int n = atoi(line.c_str());

  #ifdef ALIGNED_ALLOC
    x = (int*) aligned_alloc(64, n * sizeof(int));
    y = (int*) aligned_alloc(64, n * sizeof(int));
  #else
    x = new int[n];
    y = new int[n];
  #endif
  
  int tempx, tempy;
  for(int i = 0; i < n; i++) {
    getline(f,line);
    stringstream ss(line);
   
    ss >> tempx>> tempy;
    x[i] = tempx;
    y[i] = tempy;
  }
  return n;
}

void print(int *&x, int *&y, int *&c, double *&cx, double *&cy, int k, int n) {
  for(int i = 0; i < k; i++) {
    printf("**Cluster %d **",i);
    printf("**Center :(%f,%f)\n",cx[i],cy[i]);
    for(int j = 0; j < n; j++) {
      if(c[j] == i)
        printf("(%d,%d) ",x[j],y[j]);
    }
    printf("\n");
  }
}

void kmeans(int *&x, int *&y, int *&c, double *&cx, double *&cy, int k, int n) {
  bool end = false; 
  int iter = 0;
  while(!end && iter != MAX_ITER) {
    update(x,y,c,cx,cy,k,n);  // Update the centers
    end = assign(x,y,c,cx,cy,k,n);  // Reassign points to clusters
    iter++;
    if(end) {
      printf("end at iter :%d\n",iter);
    }
  }
  printf("Total %d iterations.\n",iter);
}
  
void usage() {
  printf("./test <filename> <k>\n");
  exit(-1);
}

int main(int argc, char *argv[]) {
  
  if(argc - 1 != 2) {
    usage();
  }

  string fname = argv[1];
  int    k     = atoi(argv[2]);
  
  int    *x;  // array of x coordinates
  int    *y;  // array of y coordinates
  double *cx; // array of x coordinates of centers
  double *cy;  // array of y coordinates of centers
  int    *c;  // array of cluster info
  
  int n = readfile(fname,x,y);
  // std::cout << "Max Threads OMP: " << omp_get_max_threads() << std::endl; // 8 on my craptop
  
  // Measure time for initialization
  double init_start = omp_get_wtime();
  init(x, y, c, cx, cy, k, n);
  double init_end = omp_get_wtime();
  printf("Initialization Time: %f seconds\n", init_end - init_start);

  // Measure time for k-means clustering
  double kmeans_start = omp_get_wtime();
  kmeans(x, y, c, cx, cy, k, n);
  double kmeans_end = omp_get_wtime();
  printf("K-Means Execution Time: %f seconds\n", kmeans_end - kmeans_start);
  
  
  // Evaluate clustering quality
  double totalSSD = 0.0;
  for (int i = 0; i < n; i++) {
    int cluster = c[i];
    totalSSD += dist(x[i], y[i], cx[cluster], cy[cluster]);
  }
  printf("Sqrt of Sum of Squared Distances (SSD): %f\n", sqrt(totalSSD));
  
  // Uncomment to print results
  #ifdef PRINT
  print(x,y,c,cx,cy,k,n);
  #endif

  delete[] x;
  delete[] y;
  delete[] cx;
  delete[] cy;
  delete[] c;

  return 0;
}
