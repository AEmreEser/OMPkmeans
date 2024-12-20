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

double dist(double x1, double y1, double x2, double y2) {
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
}

bool assign(int *&x, int *&y, int *&c, double *&cx, double *&cy, int k, int n) {
  bool end = true;
  for(int i = 0; i < n; i++) {
    int cluster;
    int minDist = INT_MAX;
    //Assign to closest center
    for(int j = 0; j < k; j++) {
      double distance = dist(cx[j], cy[j], x[i], y[i]);      
      if(distance < minDist) {
	minDist = distance;
	cluster = j;
      }
    }
    if(cluster != c[i]) { // even if one point changes, we will run for one more iteration.
      end = false;
    }
    c[i] = cluster; // assign the point to the cluster with minDist
    
  }
  return end;
}


/* x and y are arrays of coordinates
   c is an integer array representing which cluster each point belongs to.
   cx is an array for x coordinates of center points
   cy is an array for y coordinates of center points
   k is the k value for k-means
   n is the number of elements      */
void init(int *&x, int *&y, int *&c, double *&cx, double *&cy, int k, int n) {
/* Initialize the centers and assign each point to the closest one */
  cx = new double[k];
  cy = new double[k];
  
  randomCenters(x, y, n, k, cx, cy);

  /* Initialize the cluster information for each point. */

  c = new int[n];
  int cluster = 0;
  
  assign(x,y,c,cx,cy,k,n); // Assign each point to closest center.
}

void update(int *&x, int *&y, int *&c, double *&cx, double *&cy, int k, int n) {
    for(int i = 0; i < k; i++) {
        double sumx = 0, sumy = 0;
        int count = 0;
        for(int j = 0; j < n; j++) {
            if(c[j] == i) {
                sumx += x[j];
                sumy += y[j];
                count ++;
            }
        }
        cx[i] = sumx / count;
        cy[i] = sumy / count;
    }
}

int readfile(string fname, int *&x, int *&y) {
  ifstream f;
  f.open(fname.c_str());
  string line;
  getline(f,line);
  int n = atoi(line.c_str());

  x = new int[n];
  y = new int[n];
  
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
  for(int i = 0; i < k; i++)
  {
    printf("**Cluster %d **",i);
    printf("**Center :(%f,%f)\n",cx[i],cy[i]);
    for(int j = 0; j < n; j++)
    {
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
    update(x,y,c,cx,cy,k,n);
    end = assign(x,y,c,cx,cy,k,n);
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
  
  #ifdef PRINT
  print(x,y,c,cx,cy,k,n);
  #endif
  
  return 0;
}