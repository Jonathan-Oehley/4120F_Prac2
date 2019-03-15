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

//Structure for dividing work for pthreads
struct allocation
{
    int x_start, y_start, x_end, y_end, ID;
    pthread_t Thread;
};


//This is a select filter that only sorts halfway to optimise speeds
int select_med(int start[], int length){
    int i,j,lownumindex;
    for(j=0;j<ceil(length/2)+1;j++){
        lownumindex = j;
        for(i=j+1;i<length;i++){
            if (start[i]<start[lownumindex]){
                lownumindex = i;
            }
        }
        std::swap(start[lownumindex],start[j]);
    }
    if (length%2==0){
        return (int)(start[length/2]+start[length/2-1])/2;
    }
    else{
        return start[(int)length/2];
    }
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


int qsort_med(int start[], int length){
    std::sort(start, start+length);
    if (length%2==0){
        return (int)(start[length/2]+start[length/2-1])/2;
    }
    else{
        return start[(int)length/2];
    }
}

// Filter function
void median_filter(int y_start, int x_start, int y_end, int x_end, int (*med_sort)(int[], int)){

 // Note that for loops run over the pixel indices, not the RGB values
 int y_u,y_d,x_l, x_r; // Bounds on the columns/rows when constructing the data array for sorting
 int y_length,x_length,yi,xi; //Bounds and indices for area to copy

 for(int y = y_start; y < y_end; y++){
    // Check to see if we are close to top/bottom of image
    if (y<4){
        y_u = y;
    }
    else y_u = 4;
    if (y>(Input.Height-5)) {
        y_d = (Input.Height-1)-y;
    }
    else y_d = 4;

    for(int x = x_start; x < x_end; x++){
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
                //Pixel points to R value of the pixel, +1 for G, +2 for B
                //Note the time 3 on the x index as each pixel has 3 components
                r_arr[yi*x_length+xi] = Input.Rows[y-y_u+yi][3*(x-x_l+xi)];
                g_arr[yi*x_length+xi] = Input.Rows[y-y_u+yi][3*(x-x_l+xi)+1];
                b_arr[yi*x_length+xi] = Input.Rows[y-y_u+yi][3*(x-x_l+xi)+2];
            }
        }
    Output.Rows[y][3*x] = med_sort(r_arr,y_length*x_length);
    Output.Rows[y][3*x+1] = med_sort(g_arr,y_length*x_length);
    Output.Rows[y][3*x+2] = med_sort(b_arr,y_length*x_length);
//    delete [] r_arr;
//    delete [] g_arr;
//    delete [] b_arr;
    }
 }

}

// This is each thread's "main" function.  It receives a unique ID
void* Thread_Main(void* Parameter){
 allocation params = *((allocation*)Parameter);

 median_filter(params.y_start,params.x_start,params.y_end,params.x_end,qsort_med);

 return 0;
}
//------------------------------------------------------------------------------


int main(int argc, char** argv){
 int j;

 // Initialise everything that requires initialisation
 tic();
 pthread_mutex_init(&Mutex, 0);

 // Read the input image
 if(!Input.Read("Data/greatwall.jpg")){
  printf("Cannot read image\n");
  return -1;
 }

 // Allocated RAM for the output image
 if(!Output.Allocate(Input.Width, Input.Height, Input.Components)) return -2;

 // Implement Median Filter
 printf("Starting filter bubble\n");
 tic();
 median_filter(0,0,Input.Height,Input.Width,bubble_med);
 printf("Time for golden standard filter = %lg s\n\n", (double)toc());

  // Write the output image
 if(!Output.Write("Data/Output_GoldStandard.jpg")){
  printf("Cannot write image\n");
  return -3;
 }

 int Thread_Count = 32;
 // Spawn threads...
 allocation Threads [Thread_Count]; // Structure to keep the tread information

// Splitting by row:
 int thread_split_y = floor(Input.Height/Thread_Count);

 //split by column
 //int thread_split_x = floor(Input.Width/Thread_Count);

 for(j = 0; j < Thread_Count-1; j++){ //create threads and allocate them a workload

    //Comment out one of the below Blocks and the corresponding block below

//    Threads[j].x_start = thread_split_x*j;
//    Threads[j].x_end = thread_split_x*(j-1)-1;
//    Threads[j].y_start = 0;
//    Threads[j].y_end = Input.Height-1;

    Threads[j].x_start = 0;
    Threads[j].x_end = Input.Width-1;
    Threads[j].y_start = thread_split_y*j;
    Threads[j].y_end = thread_split_y*(j+1)-1;

    Threads[j].ID = j;
    pthread_create(&Threads[j].Thread, 0, Thread_Main, (void*)&Threads[j]);
 }

// Threads[Thread_Count-1].x_start = thread_split_x*(Thread_Count-1);
// Threads[Thread_Count-1].y_start = 0;

 Threads[Thread_Count-1].x_start = 0;
 Threads[Thread_Count-1].y_start = thread_split_y*(Thread_Count-1);

 Threads[Thread_Count-1].x_end = Input.Width-1;
 Threads[Thread_Count-1].y_end = Input.Height-1;

 Threads[Thread_Count-1].ID = Thread_Count-1;
 pthread_create(&Threads[Thread_Count-1].Thread, 0, Thread_Main, (void*)&Threads[Thread_Count-1]);

 // Printing stuff is a critical section...
 pthread_mutex_lock(&Mutex);
  printf("Threads created.\n");
 pthread_mutex_unlock(&Mutex);

 tic();
 // Wait for threads to finish
 for(j = 0; j < Thread_Count; j++){
  if(pthread_join(Threads[j].Thread, 0)){
   pthread_mutex_lock(&Mutex);
    printf("Problem joining thread %d\n", j);
   pthread_mutex_unlock(&Mutex);
  }
 }

// No more active threads, so no more critical sections required
 printf("All threads have quit\n");
 printf("Time taken for threads to run = %lg s\n", toc());

   // Write the output image
 if(!Output.Write("Data/Output_PThreads.jpg")){
  printf("Cannot write image\n");
  return -3;
 }

 // Clean-up
 pthread_mutex_destroy(&Mutex);
 return 0;
}
//------------------------------------------------------------------------------
