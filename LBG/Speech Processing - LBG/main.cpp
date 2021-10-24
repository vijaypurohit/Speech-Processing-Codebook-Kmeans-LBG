// Speech Processing - LBG.cpp : Defines the entry point for the console application.
/******************************************************************************************************************************************
	Speech Processing CS 566: Assignment 04 (LBG Algorithm: Linde, Buzo, Gray / Modified Kmeans)
	Roll No: 214101058 MTech CSE'23 IITG
	Input: 
		*	Universe File (.csv) consisting of cepstral coefficients vectors. 
				Folder: filePathUniverse[] = "input_universe/"
				File Name:	fileNameUniverse[] = "Universe.csv"
	Output:  
		*	Output File consising of details of each iterations and Final CodeBook.
				Folder:		filePathConsole[] = "output/"
				File Name:	fileNameConsole[] = "lbg_output.txt"	
*********************************************************************************************************************************************/

#include "stdafx.h"
#pragma warning (disable : 4996)		//to disable fopen() warnings
#include <stdio.h>
#include <stdlib.h>		//atoi, atof
#include <string.h>		//strtok
#include <conio.h>		//getch,
#include <time.h>		//time(0)
#include <math.h>       // abs
/******************--------Common Settings--------******************/
#define p 12						 //	Number of Cepstral Coefficients, dimensionality of vector x
#define codeBkSize 8		//change // CodeBook Y Size = K		
#define universeSize 6340   //change // number of the rows in the universe file
#define delta 0.00001				 // 0.00001 or 0.000001 abs(old_distortion - new_dist) > delta. 
#define epsilon 0.03		//change // value of epsilon for Yi = Yi(1+-epsilon)
/******************--------Variable Decalaration--------******************/
// WEIGHTS
const double w_tkh[]={1.0, 3.0, 7.0, 13.0, 19.0, 22.0, 25.0, 33.0, 42.0, 50.0, 56.0, 61.0};		//tokhura weigths provided.
double tok_distances[codeBkSize]={0};			//calculating tokhura distance of vector x with each vector y in codebook Y

// CODE BOOK
long ceps_count;		//count the number of vector x i.e ceps coefficients in file.
double old_distortion=0.0, current_distortion=1000.0, total_distortion=0;  //old_distortion for distortion of iteration m-1, current_distortion is for iteration m, total_distortion is for entire universe
double min_distortion = 1000000000.0;  // distortion value of the CodeVector with minimum distortion
long int iterations_m;			// total number of iterations for Kmeans

double Universe_X[universeSize][p];
double CodeBook_Y[codeBkSize][p];
long int random_index[codeBkSize]; // to store random number index 

// File Stream
FILE *fp_uni ;			//to read from Universe		
const char fileNameUniverse[] = "Universe.csv";								
const char filePathUniverse[] = "input_universe/";			
char completePathUniverse[300];	
FILE *fp_console ;		//to write in output file	
const char fileNameConsole[] = "lbg_output.txt";								
const char filePathConsole[] = "output/";			
char completePathConsole[300];	

/**************************************************************************************
	To Display Common Settings used in our System.
**************************************************************************************/
void DisplayCommonSettings(){
	// General Information to Display
		printf("****-------- WELCOME TO VECTOR QUANTIZATION (LBG) --------****\n");		
		printf("-Common Settings are : -\n");	
		printf(" P (#of Cepstral Coefficients)= : %d\n", p);
		printf(" CodeBook Size (Y) : %d\n", codeBkSize);	
		printf(" Universe Size (X) : %d\n", universeSize);
		printf(" Threshold Delta : %lf\n", delta);
		printf(" Epsilon Value : %lf\n", epsilon);

		printf(" Tokhura Weights : ");
		for(int i=0; i<p; i++){
			printf("%0.1f(%d) ", w_tkh[i],i+1);
		}
		printf("\n");
		printf("----------------------------------------------------------------\n\n");		
		
		//printing in file
		fprintf(fp_console,"****-------- WELCOME TO VECTOR QUANTIZATION (LBG) --------****\n");		
		fprintf(fp_console,"-Common Settings are : -\n");	
		fprintf(fp_console," P (#of Cepstral Coefficients)= : %d\n", p);
		fprintf(fp_console," CodeBook Size (Y) : %d\n", codeBkSize);	
		fprintf(fp_console," Universe Size (X) : %d\n", universeSize);	
		fprintf(fp_console," Threshold Delta : %lf\n", delta);
		fprintf(fp_console," Epsilon Value : %lf\n", epsilon);

		fprintf(fp_console," Tokhura Weights : ");
		for(int i=0; i<p; i++){
			fprintf(fp_console,"%0.1f(%d) ", w_tkh[i],i+1);
		}
		fprintf(fp_console,"\n");
		fprintf(fp_console,"----------------------------------------------------------------\n\n");	
}

/**************************************************************************************
	Display the entire CodeBook Y and its all vector y with dimension k(=p).
**************************************************************************************/
void displayCodeBook(int s, int m){

	if(s==1){ // printing in file and Universe X
		printf("Universe\t  c[1]\t  c[2]\t  c[3]\t  c[4]\t  c[5]\t  c[6]\t  c[7]\t  c[8]\t  c[9]\t  c[10]\t  c[11]\t  c[12]\n");
		fprintf(fp_console, "Universe    c[1]	    c[2]	   c[3]		   c[4]		   c[5]		   c[6]		   c[7]		   c[8]		    c[9]	   c[10]	  c[11]		   c[12]\n");
		
		for (int y = 0; y < m; ++y)
		{
				printf("X[%ld]  ",random_index[y]); fprintf(fp_console, "X[%ld]",random_index[y]);
				for (int k = 0; k < p; ++k){
					printf("  %lf",CodeBook_Y[y][k]);  fprintf(fp_console, "\t %lf",CodeBook_Y[y][k]);    
				}
				fprintf(fp_console, "\n"); 
		}//end of y for-loop
	}
	else if(s==2){ // printing in file and Cluster Size
		fprintf(fp_console, "Cluster Size    c[1]	    c[2]	   c[3]		   c[4]		   c[5]		   c[6]		   c[7]		   c[8]		    c[9]	   c[10]	  c[11]		   c[12]\n");
		for (int y = 0; y < m; ++y)
		{
				fprintf(fp_console, "[%ld]",random_index[y]);
				for (int k = 0; k < p; ++k){
					fprintf(fp_console, "\t %lf",CodeBook_Y[y][k]);  
				}
				fprintf(fp_console, "\n"); 
		}//end of y for-loop
	}
	else if(s==11){		// printing in file and show on console and Cluster Size
		printf("Cluster Size\t  c[1]\t  c[2]\t  c[3]\t  c[4]\t  c[5]\t  c[6]\t  c[7]\t  c[8]\t  c[9]\t  c[10]\t  c[11]\t  c[12]\n");
				fprintf(fp_console, "Cluster Size    c[1]	    c[2]	   c[3]		   c[4]		   c[5]		   c[6]		   c[7]		   c[8]		    c[9]	   c[10]	  c[11]		   c[12]\n");
		for (int y = 0; y < m; ++y)
		{
			printf("[%ld]",random_index[y]);		fprintf(fp_console, "[%ld]",random_index[y]);
			for (int k = 0; k < p; ++k){
				printf("  %lf",CodeBook_Y[y][k]);	fprintf(fp_console, "\t %lf",CodeBook_Y[y][k]);  
			}
			printf("\n");							fprintf(fp_console, "\n"); 
		}//end of y for-loop
	}
	else if(s==12){ // printing in file  and NO Cluster Size
		fprintf(fp_console, "    c[1]	    c[2]	   c[3]		   c[4]		   c[5]		   c[6]		   c[7]		   c[8]		    c[9]	   c[10]	  c[11]		   c[12]\n");
		for (int y = 0; y < m; ++y)
		{
			for (int k = 0; k < p; ++k){
				fprintf(fp_console, " %lf\t",CodeBook_Y[y][k]);  
			}
			printf("\n"); 
			fprintf(fp_console, "\n"); 
		}//end of y for-loop
	}
	
}

/**************************************************************************************
	Read Each Vector x of dimension k(=p) from Universe File (X) into array Universe_X
	Input: Universe File
**************************************************************************************/
void readCepsFromFile(){

	char *lineToRead; // reading line by line from file.
	const int rowSize = 1024;  //rowSize stores the size of one row of the .csv file //Maximum number of characters to be copied into str  (including the terminating null-character)
	char line_str[rowSize];   //Pointer to an array of chars where the string read is copied.

	int col = 0;
	ceps_count=0; //number of rows

	while(fgets(line_str,rowSize,fp_uni))
	{
	
		lineToRead = strtok(line_str, ","); //break the row by delimiter "," into chunks one by one.
		col=0;
		while (lineToRead != NULL) //till there are more numbers in line string
		{
			Universe_X[ceps_count][col] = atof(lineToRead); //store the number in universe X
			++col;
			lineToRead = strtok(NULL,","); //proceed for the next number
		}
		ceps_count++;
	}

	
	printf("Number of Cepstral Coefficients Vectors read from file are: %d\n", ceps_count);
	fprintf(fp_console, "Number of Cepstral Coefficients Vectors read from file are: %d\n", ceps_count);
}

/**************************************************************************************
	Kmeans S1: Initialization: 
	  Randomly choosing a set of vector x to store into CodeBook Y
**************************************************************************************/
void s1_initialization(){
	int random_num_index=0;
	 
	srand(time(0)); // Current Time as Seed for Random Generator
	
	for (int y = 0; y < codeBkSize; ++y) 
	{
		random_num_index = rand() % universeSize;
		random_index[y]=random_num_index;
		//printf("\t  %d",random_num_index+1);
		for (int k = 0; k < p; ++k)
			CodeBook_Y[y][k] = Universe_X[random_num_index][k]; 
	}

}

/**************************************************************************************
	Kmeans S2: Classification: 
	  Classify each vector x of universe into their cluster according to Nearest 
	  Neighbourhood reduction rule. (Tokhura Distance)
	Kmeans S3: Code Vector Updation
	  Updating the code vector of every cluster by computing the centroid of each cluster.
	Input: Current Size of the CodeBook.
**************************************************************************************/
void s2_3_classification(int curCBsize)
{
	double temp;
	long int ClusterNumber[universeSize]={0};		// Store the Cluster Number, The vector x belong to.
	long int ClusterSize[codeBkSize]={0};			// Size of Each Cluster, i.e. Number of x Vectors it contains
	long double ClusterSum[codeBkSize][p]={0};		// Sum of Each Cluster with vectors present in it,  
	long minIndex_y=-1;								// Store the index of vector y which has minimum distance with vector x
	long maxIndex_cluster=0;							// Index of Cluster having Maximum size. 
	
	// classifying each vector x in the universe X to their cluster y.
	for(int x=0; x<universeSize ; ++x)		
	{
		min_distortion = 1000000000.0;		//for each iteration

		for (int y = 0; y < curCBsize; ++y)		// for each vector y in the CodeBook Y
		{
			tok_distances[y] = 0;
			for (int k = 0; k < p; ++k)		// for each p value
			{
				temp=Universe_X[x][k]-CodeBook_Y[y][k];
				tok_distances[y] += w_tkh[k]*(temp)*(temp); 
			}

			if (tok_distances[y] < min_distortion) // Finding min distance in order to put vector x into that cluster y
			{
				min_distortion = tok_distances[y];
				minIndex_y = y;
			}

		} // y-loop over

		// Vector x belong to vector y having min distance among all y in codebook Y.
		ClusterNumber[x] = minIndex_y;

		// Increase the respective Cluster size as we added a new vector in it.
		++ClusterSize[minIndex_y];

		// Saving index of cluster having maximum size among all.
		if (ClusterSize[minIndex_y] > ClusterSize[maxIndex_cluster])
			maxIndex_cluster = minIndex_y;

		// adding current vector x value to its respective cluster sum
		for (int k = 0; k < p; ++k)
			ClusterSum[minIndex_y][k] += Universe_X[x][k];

		// adding the current distortion to the total distortion of entire universe.
		total_distortion += min_distortion;

		// UnComment to Print Cluster Number of vector x in File
		/*
			fprintf(fp_console, "X[%d]#: C[%ld]\n",x,ClusterNumber[x]);
		*/

		// UnComment to Print Cluster Size after adding each vector.
		/*
			for (int y = 0; y < curCBsize; ++y){
				//printf("C[%d]: %d ",y, ClusterSize[y]);
				fprintf(fp_console, "C[%d]size: %ld ",y, ClusterSize[y]);
				random_index[y]=ClusterSize[y];
			}	
			//printf("\n");
			fprintf(fp_console, "\n");
		 */

	} // x for-loop over

	/**************** S3: Code Vector Updation ****************/
	for (int y = 0; y < curCBsize; ++y){
		for (int k = 0; k < p; ++k)
		{
			CodeBook_Y[y][k] = ClusterSum[y][k]/ClusterSize[y];
			
		}
		random_index[y]=ClusterSize[y];
	}
			
	old_distortion = current_distortion;				//store the previous distortion
	current_distortion = total_distortion/universeSize; //updating current distortion

}

/**************************************************************************************
	Kmeans Algorithm or Generalized LLoyd's Algorithm
**************************************************************************************/
void kmeans_algo(int curCBsize){

	/**************** S1: Initialization ****************/
	//s1_initialization();

	//printf("\n***************------- Initial Codebook -------*************\n"); fprintf(fp_console, "\n***************------- Initial Codebook -------*************\n"); 
		//displayCodeBook(1, curCBsize);
	
	iterations_m=0;
	old_distortion=0;
	current_distortion=1000;

	/**************** S4: Termination Loop ****************/
	while(abs(current_distortion-old_distortion) >= delta){ 
		++iterations_m ;
		total_distortion=0;

		/**************** S2: Classification Also include S3: Code Vector Updating ****************/
		s2_3_classification(curCBsize);

		fprintf(fp_console, "\n\t --> Iteration: %ld \t Codebook\n",iterations_m); 
		  displayCodeBook(2, curCBsize);
		 fprintf(fp_console,"\t --> Iteration: %ld \t Current Distortion: %lf\n\n",iterations_m, current_distortion);
	
	}//end of while S4: Termination Loop

	//printf("\n****************------- Kmeans Final Codebook -------**************\n");
	//	fprintf(fp_console, "\n****************------- Kmeans Final Codebook -------**************\n"); 
	//	displayCodeBook(11, curCBsize);

	printf("\nKmeans: \nTotal iterations = %ld\nDistortion difference = %lf\nCurrent Distortion = %lf\nOld Distortion = %lf\n",iterations_m,abs(current_distortion-old_distortion),current_distortion, old_distortion);
		fprintf(fp_console, "\nKmeans: \nTotal iterations = %ld\nDistortion difference = %lf\nCurrent Distortion = %lf\nOld Distortion = %lf\n",iterations_m,abs(current_distortion-old_distortion),current_distortion, old_distortion); 

}

//---------------------------------------------------------------------------------------------------------

/**************************************************************************************
	LBG S1: Design a 1 Vector CodeBook: 
	  Taking the Centroid of the entire Universe.
**************************************************************************************/
void LBG_s1_initialization(){

	 double overall_sum[p]={0};

	/**************** Finding Centroid ****************/
	// Summing over all vectors of universe.
	for (int x = 0; x < universeSize; ++x) 
	{
		for (int k = 0; k < p; ++k)
			overall_sum[k] += Universe_X[x][k]; 
	}

	// Taking the average
	for (int k = 0; k < p; ++k)
	{
			overall_sum[k] /= universeSize;
			CodeBook_Y[0][k] = overall_sum[k];
	}

	random_index[0]=universeSize;

	//for (int k = 0; k < p; ++k)
	//	printf("%lf ",overall_sum[k]);
	//printf("\n");
	
}

/**************************************************************************************
	LBG Algorithm: Linde, Buzo, Gray / Modified Kmeans
**************************************************************************************/
void lbg_algo(){
	int m = 1;			// Current size of the CodeBook.
	int old_m = m;		// CodeBook Size before Splitting.

	/**************** S1: Design a 1 Vector CodeBook ****************/
	LBG_s1_initialization();

	printf("\n***************------- Initial Codebook (m=%d) -------*************\n", m); fprintf(fp_console, "\n***************------- Initial Codebook (m=%d) -------*************\n", m); 
		displayCodeBook(11, m);

		/**************** S4: Termination Loop ****************/
	while(codeBkSize != m){		// loop until we get our desired size of the CodeBook i.e. codeBkSize
		old_m = m;
		/**************** S2: Splitting the CodeBook, Double the size of CodeBook ****************/
		for (int y = 0; y < old_m; ++y)
		{
			for (int k = 0; k < p; ++k)
			{
				CodeBook_Y[old_m+y][k]= CodeBook_Y[y][k]*(1+epsilon);
			}
			for (int k = 0; k < p; ++k)
			{
				CodeBook_Y[y][k]= CodeBook_Y[y][k]*(1-epsilon);
			}
		}

		m = 2*m; //new size of the codebook.

		fprintf(fp_console, "\n ---------------------- >\t CodeBook After Split and Before Applying Kmeans. \t Codebook Size: %d \t <---------------------- \n",m); 
		  displayCodeBook(12, m);

		/**************** S3: Applying Kmeans to The non-optimal CodeBook ****************/
		  kmeans_algo(m);

		printf("\n---------------------- >\t CodeBook After Applying Kmeans. \t Codebook Size: %d \t <---------------------- \n",m);
		fprintf(fp_console, "\n---------------------- >\t CodeBook After Applying Kmeans. \t Codebook Size: %d \t <---------------------- \n",m); 
		  displayCodeBook(11, m);
		
		fprintf(fp_console, "\n\t\t\t---------------------- ><---------------------- \n\n"); 
		 
	}//end of while S4: Termination Loop


	printf("\n****************------- Final Codebook (m=%d) -------*************\n", m);
		fprintf(fp_console, "\n****************------- Final Codebook (m=%d) -------*************\n", m); 
		displayCodeBook(11, m);
}

int _tmain(int argc, _TCHAR* argv[])
{
	/**************** Creating necessary file Path for data. ****************/
	sprintf(completePathUniverse, "%s%s", filePathUniverse, fileNameUniverse); //completePathUniverse = {filePathUniverse} + {Universe.csv}
	sprintf(completePathConsole, "%s%s", filePathConsole, fileNameConsole); 
	/**************** Opening respective files. ****************/
	fp_uni = fopen(completePathUniverse, "r");				//to read input universe file
	fp_console = fopen(completePathConsole, "w");				//to read input universe file
		if(fp_uni == NULL || fp_console == NULL){ 
			perror("\nError: ");
			printf("\n File Names are Input Universe, Console Output: %s \n %s", completePathUniverse, completePathConsole);
			getch();
			return EXIT_FAILURE;
		}
	
	/**************** Common Information to Display ****************/
	DisplayCommonSettings();

	/**************** Reading Ceps Vector x from Universe X ****************/
	readCepsFromFile();
		fclose(fp_uni);	// closing file pointer of reading from universe.

	/**************** Applying LBG Algorithm ****************/
	printf("\n\t---------------------- Starting LBG Algo ----------------------\n"); fprintf(fp_console, "\n\t---------------------- Starting LBG Algo ----------------------\n"); 
	lbg_algo();
	printf("\n\t---------------------- Ending LBG Algo ----------------------\n"); fprintf(fp_console, "\n\t---------------------- Ending LBG Algo ----------------------\n");
	
	printf("\n Detailed Output File : %s", completePathConsole);
	
	fclose(fp_console);
	getch();
	return 0;
}
