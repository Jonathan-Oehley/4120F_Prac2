//==============================================================================
// Copyright (C) John-Philip Taylor
// tyljoh010@myuct.ac.za
//
// This file is part of the EEE4084F Course
//
// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>
//==============================================================================

#include "Prac2.h"
//------------------------------------------------------------------------------

// This is each thread's "main" function.  It receives a unique ID
void* Thread_Main(void* Parameter){
 int ID = *((int*)Parameter);

 pthread_mutex_lock(&Mutex);
  //printf("Hello from thread %d\n", ID);
 pthread_mutex_unlock(&Mutex);

 for(int j = 0; j < 1e6; j++); // Do some heavy calculation

 pthread_mutex_lock(&Mutex);
  //printf("Thread %d: I QUIT!\n", ID);
 pthread_mutex_unlock(&Mutex);

 return 0;
}
//------------------------------------------------------------------------------

int bubble_med(int start[], int length){
    int i,j;
    for(j=0;j<length-1;j++){
        for(i=0;i<length-1-j;i++){
            if (start[i]>start[i+1]) std::swap(start[i],start[i+1]);
        }
    }
    if (length%2==0){
        return (int)(start[length/2]+start[length/2-1])/2;
    }
    else{
        return start[(int)length/2];
    }
}


int main(int argc, char** argv){
 int j;

 // Initialise everything that requires initialisation
 tic();
 pthread_mutex_init(&Mutex, 0);

 // Read the input image
 if(!Input.Read("Data/fly.jpg")){
  printf("Cannot read image\n");
  return -1;
 }

 // Allocated RAM for the output image
 if(!Output.Allocate(Input.Width, Input.Height, Input.Components)) return -2;

 // This is example code of how to copy image files ----------------------------
 // printf("Start of example code...\n");
 //for(j = 0; j < 10; j++){
 // tic();
 //int x, y;
 // for(y = 0; y < Input.Height; y++){
 //  for(x = 0; x < Input.Width*Input.Components; x++){
 //   Output.Rows[y][x] = Input.Rows[y][x];
 //  }
 // }
 // printf("Time = %lg ms\n", (double)toc()/1e-3);
 //}
 //printf("End of example code...\n\n");
 // End of example -------------------------------------------------------------

 // Copy Image File
 tic();
 int x, y;
 for(y = 0; y < Input.Height; y++){
  for(x = 0; x < Input.Width*Input.Components; x++){
   Output.Rows[y][x] = Input.Rows[y][x];
  }
 }

 // Testing
 // printf("Output components # is: %d (3 if RGB)\n", Output.Components);
 // Output.Rows[0][0] = 156;
 // printf("Output [0][0] R,G,B is: %d,%d,%d\n", Output.Rows[0][0],Output.Rows[0][1],Output.Rows[0][2]);

 int test_arr1 [9] = {1,2,3,4,5,6,7,8,9};
// for (int i=0;i<9;i++){
//    printf("%d ",test_arr1[i]);
// }
 int test_arr2 [6] = {1,2,3,4,5,6};
// printf("Median 1 is %d\n",bubble_med(test_arr1,7));
// printf("Median 2 is %d",bubble_med(test_arr2,6));

 printf("\n\n");

 // Implement Median Filter
 printf("Starting filter\n");
 printf("Input.Width is %d\n",Input.Width);
 tic();
 int y_u,y_d,x_l, x_r, y_length,x_length,yi,xi; // Bounds on the columns/rows when constructing the data array for sorting
 for(y = 0; y < Input.Height; y++){
    // Check to see if we are close to top/bottom of image
    if (y<4){
        y_u = y;
        //printf("%d",y_u);
    }
    else y_u = 4;
    if (y>(Input.Height-5)) {
        y_d = (Input.Height-1)-y;
        //printf("%d",y_d);
    }
    else y_d = 4;

    for(x = 0; x < Input.Width; x++){
        //if (y==0) printf("%d %d %d\n",x*3,x*3+1,x*3+2);
        // Check to see if we are close to left/right of image
        if (x<4){
            x_l = x;
            //printf("%d",x_l);
        }
        else x_l = 4;
        if (x>(Input.Width-5)) {
            x_r = (Input.Width-1)-x;
            //printf("%d",x_r);
        }
        else x_r = 4;

        // Copy area of image to find median of
        y_length = 1+y_u + y_d;                 // Height and width of area
        x_length = 1+x_r + x_l;
        int r_arr[y_length*x_length] = {};       // RGB 1D arrays
        int g_arr[y_length*x_length] = {};
        int b_arr[y_length*x_length] = {};

        for (yi=0;yi<y_length;yi++){        // Copy data
            for (xi=0;xi<x_length;xi++){
                r_arr[yi*x_length+xi] = Input.Rows[y-y_u+yi][3*x-x_l+xi*3];
                g_arr[yi*x_length+xi] = Input.Rows[y-y_u+yi][3*x-x_l+xi*3+1];
                b_arr[yi*x_length+xi] = Input.Rows[y-y_u+yi][3*x-x_l+xi*3+2];
            }
        }
    Output.Rows[y][3*x] = bubble_med(r_arr,y_length*x_length);
    Output.Rows[y][3*x+1] = bubble_med(g_arr,y_length*x_length);
    Output.Rows[y][3*x+2] = bubble_med(b_arr,y_length*x_length);
    }
 }
 printf("Time for median = %lg s\n\n", (double)toc());

 // Spawn threads...
 int       Thread_ID[Thread_Count]; // Structure to keep the tread ID
 pthread_t Thread   [Thread_Count]; // pThreads structure for thread admin

 for(j = 0; j < Thread_Count; j++){
  Thread_ID[j] = j;
  pthread_create(Thread+j, 0, Thread_Main, Thread_ID+j);
 }

 // Printing stuff is a critical section...
 pthread_mutex_lock(&Mutex);
  //printf("Threads created :-)\n");
 pthread_mutex_unlock(&Mutex);

 tic();
 // Wait for threads to finish
 for(j = 0; j < Thread_Count; j++){
  if(pthread_join(Thread[j], 0)){
   pthread_mutex_lock(&Mutex);
    printf("Problem joining thread %d\n", j);
   pthread_mutex_unlock(&Mutex);
  }
 }

 // No more active threads, so no more critical sections required
 printf("All threads have quit\n");
 printf("Time taken for threads to run = %lg ms\n", toc()/1e-3);

 // Write the output image
 if(!Output.Write("Data/Output.jpg")){
  printf("Cannot write image\n");
  return -3;
 }

 // Clean-up
 pthread_mutex_destroy(&Mutex);
 return 0;
}
//------------------------------------------------------------------------------
