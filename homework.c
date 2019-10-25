

#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#include<mpi.h>

typedef struct {
     unsigned char red,green,blue;
} pixel;

typedef struct {
	unsigned char value;
} pixel_gray;

typedef struct {
	int height,width;
	char type;
	unsigned char maxVal;
	pixel_gray **matrix_gray;
	pixel **matrix;
}image;



												


void readInput(const char * fileName, image *img) {


	char buff[16];

    FILE *fp;

    fp = fopen(fileName, "rb");

    fgets(buff, sizeof(buff), fp);
         

	img->type = buff[1];

 
  	fscanf(fp, "%d %d", &img->width, &img->height);
  

    fscanf(fp, "%hhu", &img->maxVal); 
  

	//if the image is pnm
	if(img->type == '6'){
  
    	while (fgetc(fp) != '\n');
    		
			
	int i;

		img->matrix = (pixel **)malloc(img->height * sizeof(pixel*));
	


		for(i=0; i<img->height; i++) {
			img->matrix[i] = (pixel *)malloc(img->width * sizeof(pixel));
			fread(img->matrix[i], sizeof(pixel), img->width, fp);
		}

	//if the image is pgm
	} else if(img->type == '5'){
		

		while (fgetc(fp) != '\n') ;
    		
    		
		img->matrix_gray = (pixel_gray **)malloc(img->height * sizeof(pixel_gray*));
	
		int i;

		for(i=0; i<img->height; i++) {
			img->matrix_gray[i] = (pixel_gray *)malloc(img->width * sizeof(pixel_gray));
			fread(img->matrix_gray[i], sizeof(pixel_gray), img->width, fp);
		}


	}

    fclose(fp);

	
    
}

void writeData(const char * fileName, image *img) {



	FILE *fp;
	

   
    fp = fopen(fileName, "wb");


    fprintf(fp, "P%c\n",img->type);


    fprintf(fp, "%d %d\n",img->width,img->height);


    fprintf(fp, "%u\n",img->maxVal);


    int i;

	if(img->type == '6'){

    		
		for(i=0; i<img->height; i++) {
  			fwrite(img->matrix[i], sizeof(pixel),img->width, fp);
		}


	} else if(img->type == '5'){

    		
		for(i=0; i<img->height; i++) {
  			fwrite(img->matrix_gray[i], sizeof(pixel_gray),img->width, fp);
		}


	}


    fclose(fp);

}


void updateMatrix(float kernel[3][3],char filter[]){


		if(strcmp(filter,"smooth") == 0){

				kernel[0][0] = (float)1/9;
				kernel[0][1] = (float)1/9;
				kernel[0][2] = (float)1/9;

				kernel[1][0] = (float)1/9;
				kernel[1][1] = (float)1/9;
				kernel[1][2] = (float)1/9;

				kernel[2][0] = (float)1/9;
				kernel[2][1] = (float)1/9;
				kernel[2][2] = (float)1/9;
											
		}

		if(strcmp(filter,"blur") == 0){

				kernel[0][0] = (float)1/16;
				kernel[0][1] = (float)2/16;
				kernel[0][2] = (float)1/16;

				kernel[1][0] = (float)2/16;
				kernel[1][1] = (float)4/16;
				kernel[1][2] = (float)2/16;

				kernel[2][0] = (float)1/16;
				kernel[2][1] = (float)2/16;
				kernel[2][2] = (float)1/16;
		
		}

		if(strcmp(filter,"sharpen") == 0){

				kernel[0][0] = (float)0/3;
				kernel[0][1] = (float)-2/3;
				kernel[0][2] = (float)0/3;

				kernel[1][0] = (float)-2/3;
				kernel[1][1] = (float)11/3;
				kernel[1][2] = (float)-2/3;

				kernel[2][0] = (float)0/3;
				kernel[2][1] = (float)-2/3;
				kernel[2][2] = (float)0/3;
										
		}

		if(strcmp(filter,"mean") == 0){

				kernel[0][0] = (float)-1;
				kernel[0][1] = (float)-1;
				kernel[0][2] = (float)-1;

				kernel[1][0] = (float)-1;
				kernel[1][1] = (float)9;
				kernel[1][2] = (float)-1;

				kernel[2][0] = (float)-1;
				kernel[2][1] = (float)-1;
				kernel[2][2] = (float)-1;
												
		}

		if(strcmp(filter,"emboss") == 0){

				kernel[0][0] = (float)0;
				kernel[0][1] = (float)1;
				kernel[0][2] = (float)0;

				kernel[1][0] = (float)0;
				kernel[1][1] = (float)0;
				kernel[1][2] = (float)0;

				kernel[2][0] = (float)0;
				kernel[2][1] = (float)-1;
				kernel[2][2] = (float)0;	

		}


}



int main(int argc, char * argv[]) {



	int numberOfFilters;
	char **filters; //get the filters from the parameters



	image input;

	
	numberOfFilters = argc - 3;


	filters = (char **)malloc(numberOfFilters * sizeof(char*));
	int i;
	for(i = 3; i < argc; i ++){
		filters[i-3] = argv[i];
		
	}


	
	int numberOfLinesEach;
	
	

    int rank;
    int nProcesses;
 
    MPI_Init(&argc, &argv);
    
 
 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
   

    //the first process
	if(rank == 0){
		readInput(argv[1], &input);


		

		int j;
		for(j = rank+1 ; j < nProcesses ; j++){
			MPI_Send(&input.type,1,MPI_CHAR,j,4,MPI_COMM_WORLD); //i send to the rest the type of the image
			MPI_Send(&input.width,1,MPI_INT,j,4,MPI_COMM_WORLD); //i send to the rest the width of the image
		}	

		
			
			
			numberOfLinesEach = input.height / nProcesses;
			

			if(input.type == '6'){ //color

				

				for(i = 0; i < numberOfFilters; i ++){
					if(nProcesses == 1){ //only one process, it does all the work

							pixel **local_matrix_aux = (pixel **)malloc(input.height * sizeof(pixel*));
							for(int h = 0; h < input.height; h ++){

								local_matrix_aux[h] = (pixel *)malloc(input.width * sizeof(pixel));
								for(int g = 0 ; g < input.width ; g++){

									local_matrix_aux[h][g] = input.matrix[h][g]; //i use a matrix aux to keep the old values
								}


							}

							// i start from 1 and end before height-1 or width-1 to leave the borders of the image unchanged
							for(int k = 1 ; k <  input.height -1 ; k++) {
							
						
								for(int l = 1 ; l < input.width - 1 ; l ++) {
									
									float sumred = 0, sumgreen =0, sumblue =0;


									float kernel[3][3];

									updateMatrix(kernel,filters[i]);


									int ki = 0;
									int kj = 0;

									// i compute the new pixel [k][l] 
									for(int x = k-1; x <= k+1; x++){

										for(int y = l-1; y<= l+1; y++){

											sumred +=  (float)(kernel[ki][kj] * (float)local_matrix_aux[x][y].red) ;
											sumgreen += (float)(kernel[ki][kj] * (float)local_matrix_aux[x][y].green);
											sumblue +=  (float)(kernel[ki][kj] * (float)local_matrix_aux[x][y].blue);



											kj++;
										}
										ki++;
										kj=0;
									}


									input.matrix[k][l].red = (unsigned char)sumred;	
									input.matrix[k][l].green = (unsigned char)sumgreen;
									input.matrix[k][l].blue = (unsigned char)sumblue;

								}
							}




					}
					

					if(nProcesses != 1){
						

						pixel **local_matrix = (pixel **)malloc((numberOfLinesEach + 1) * sizeof(pixel*));
						pixel **local_matrix_aux = (pixel **)malloc((numberOfLinesEach + 1) * sizeof(pixel*));
						int k,l;
						for(k = 0 ; k <  numberOfLinesEach + 1; k++){
							local_matrix[k] = (pixel *)malloc(input.width * sizeof(pixel));
							local_matrix_aux[k] = (pixel *)malloc(input.width * sizeof(pixel));
						
							for(l = 0 ; l < input.width ; l ++){
								local_matrix[k][l] = input.matrix[k][l];
								local_matrix_aux[k][l] = input.matrix[k][l];
								

							}
						}

						for(k = 1 ; k <  numberOfLinesEach ; k++) {
							
						
							for(l = 1 ; l < input.width - 1 ; l ++) {
								
								float sumred = 0, sumgreen =0, sumblue =0;


								float kernel[3][3];

								updateMatrix(kernel,filters[i]);


								int ki = 0;
								int kj = 0;

								for(int x = k-1; x <= k+1; x++){

									for(int y = l-1; y<= l+1; y++){

										sumred +=  (float)(kernel[ki][kj] * (float)local_matrix_aux[x][y].red) ;
										sumgreen += (float)(kernel[ki][kj] * (float)local_matrix_aux[x][y].green);
										sumblue +=  (float)(kernel[ki][kj] * (float)local_matrix_aux[x][y].blue);



										kj++;
									}
									ki++;
									kj=0;
								}


								local_matrix[k][l].red = (unsigned char)sumred;	
								local_matrix[k][l].green = (unsigned char)sumgreen;
								local_matrix[k][l].blue = (unsigned char)sumblue;

							}
						}

						

						
				
					


						int j;
						for(j = rank+1 ; j < nProcesses ; j++){

							if(j != nProcesses - 1){
								//for the rest of the processes i send the numberOfLinesEach + 2  because those 2 lines will be needed to compute the pixels
								

							

								int size_of_vector = (((j + 1) * numberOfLinesEach - j*numberOfLinesEach) + 2) * input.width * 3;

								MPI_Send(&size_of_vector,1 , MPI_INT,j,4,MPI_COMM_WORLD);

								int send_vector[size_of_vector];
								
								int count = 0;
								
								for(k = j * numberOfLinesEach -1; k < (j + 1) * numberOfLinesEach +1 ; k++){ //i split the image to give each process a part 
									
						
									for(l = 0 ; l < input.width ; l ++){
										send_vector[count] = (unsigned int)input.matrix[k][l].red;
										count++;

										send_vector[count] = (unsigned int)input.matrix[k][l].green;
										count++;

										send_vector[count] = (unsigned int)input.matrix[k][l].blue;
										count++;

									}
								}
									
								MPI_Send(&send_vector,size_of_vector , MPI_INT,j,4,MPI_COMM_WORLD);


							} else if(j == nProcesses - 1){

									

									int size_of_vector = ((input.height  - j*numberOfLinesEach) + 1) * input.width * 3;

									MPI_Send(&size_of_vector,1 , MPI_INT,j,4,MPI_COMM_WORLD);

									int send_vector[size_of_vector];
								

								int count = 0;
								for(k = j * numberOfLinesEach -1; k < input.height  ; k++){ //the last process gets the remains of the image until the end
									
						
									for(l = 0 ; l < input.width ; l ++){
										send_vector[count] = input.matrix[k][l].red;
										count++;

										send_vector[count] = input.matrix[k][l].green;
										count++;

										send_vector[count] = input.matrix[k][l].blue;
										count++;

									}
								}

						
							
								MPI_Send(&send_vector,size_of_vector , MPI_INT,j,4,MPI_COMM_WORLD);
									
								
							

							}
							
							


						}


						//assemble the matrix
						int lineCounter = 0;

						for(k = 0 ; k <  numberOfLinesEach ; k++) {
							
						
							for(l = 0; l < input.width  ; l ++) {
								input.matrix[k][l].red = local_matrix[k][l].red;
								input.matrix[k][l].green = local_matrix[k][l].green;
								input.matrix[k][l].blue = local_matrix[k][l].blue;

							}

						}

						lineCounter = numberOfLinesEach;

	
						for(j = rank+1 ; j < nProcesses ; j++){

							int dimension;
							MPI_Recv(&dimension,1,MPI_INT,j,MPI_ANY_TAG, MPI_COMM_WORLD,  NULL );

							int count = 0;

							int recv_vector[dimension];
							MPI_Recv(&recv_vector,dimension,MPI_INT,j,MPI_ANY_TAG, MPI_COMM_WORLD,  NULL ); //receive the other parts of the image 

							int end = lineCounter + numberOfLinesEach;
							if(j == nProcesses - 1){

								end = input.height;
							}
							for(k = lineCounter ; k < end ; k++) {
							
						
								for(l = 0; l < input.width  ; l ++) {

									input.matrix[k][l].red = (unsigned char)recv_vector[count];
									count++;

									input.matrix[k][l].green = (unsigned char)recv_vector[count];
									count++;

									input.matrix[k][l].blue = (unsigned char)recv_vector[count];
									count++;

								}

							}
							
							lineCounter = end;

						}

						

					}
				}
				
				
			} else if(input.type == '5'){ //grayscale
				for(i = 0; i < numberOfFilters; i ++){
					if(nProcesses == 1){

							pixel_gray **local_matrix_aux = (pixel_gray **)malloc(input.height * sizeof(pixel_gray*));
							for(int h = 0; h < input.height; h ++){

								local_matrix_aux[h] = (pixel_gray *)malloc(input.width * sizeof(pixel_gray));
								for(int g = 0 ; g < input.width ; g++){

									local_matrix_aux[h][g] = input.matrix_gray[h][g];
								}


							}


							for(int k = 1 ; k <  input.height -1 ; k++) {
							
						
								for(int l = 1 ; l < input.width - 1 ; l ++) {
									
									float sum=0;


									float kernel[3][3];

									updateMatrix(kernel,filters[i]);


									int ki = 0;
									int kj = 0;

									for(int x = k-1; x <= k+1; x++){

										for(int y = l-1; y<= l+1; y++){

											sum +=  (float)(kernel[ki][kj] * (float)local_matrix_aux[x][y].value) ;
										



											kj++;
										}
										ki++;
										kj=0;
									}


									input.matrix_gray[k][l].value = (unsigned char)sum;	
									

								}
							}




					}
					

					if(nProcesses != 1){
						

						pixel_gray **local_matrix = (pixel_gray **)malloc((numberOfLinesEach + 1) * sizeof(pixel_gray*));
						pixel_gray **local_matrix_aux = (pixel_gray **)malloc((numberOfLinesEach + 1) * sizeof(pixel_gray*));
						int k,l;
						for(k = 0 ; k <  numberOfLinesEach + 1; k++){
							local_matrix[k] = (pixel_gray *)malloc(input.width * sizeof(pixel_gray));
							local_matrix_aux[k] = (pixel_gray *)malloc(input.width * sizeof(pixel_gray));
						
							for(l = 0 ; l < input.width ; l ++){
								local_matrix[k][l] = input.matrix_gray[k][l];
								local_matrix_aux[k][l] = input.matrix_gray[k][l];
								

							}
						}

						for(k = 1 ; k <  numberOfLinesEach ; k++) {
							
						
							for(l = 1 ; l < input.width - 1 ; l ++) {
								
								float sum = 0;


								float kernel[3][3];

								updateMatrix(kernel,filters[i]);


								int ki = 0;
								int kj = 0;

								for(int x = k-1; x <= k+1; x++){

									for(int y = l-1; y<= l+1; y++){

										sum +=  (float)(kernel[ki][kj] * (float)local_matrix_aux[x][y].value) ;
										



										kj++;
									}
									ki++;
									kj=0;
								}


								local_matrix[k][l].value = (unsigned char)sum;	
								

							}
						}


						int j;
						for(j = rank+1 ; j < nProcesses ; j++){

							if(j != nProcesses - 1){


							

								int size_of_vector = (((j + 1) * numberOfLinesEach - j*numberOfLinesEach) + 2) * input.width;

								MPI_Send(&size_of_vector,1 , MPI_INT,j,4,MPI_COMM_WORLD);

								int send_vector[size_of_vector];
								
								int count = 0;
								
								for(k = j * numberOfLinesEach -1; k < (j + 1) * numberOfLinesEach +1 ; k++){
									
						
									for(l = 0 ; l < input.width ; l ++){
										send_vector[count] = (unsigned int)input.matrix_gray[k][l].value;
										count++;

										

									}
								}
									
								MPI_Send(&send_vector,size_of_vector , MPI_INT,j,4,MPI_COMM_WORLD);


							} else if(j == nProcesses - 1){

									

									int size_of_vector = ((input.height  - j*numberOfLinesEach) + 1) * input.width;

									MPI_Send(&size_of_vector,1 , MPI_INT,j,4,MPI_COMM_WORLD);

									int send_vector[size_of_vector];
								

								int count = 0;
								for(k = j * numberOfLinesEach -1; k < input.height  ; k++){
									
						
									for(l = 0 ; l < input.width ; l ++){
										send_vector[count] = input.matrix_gray[k][l].value;
										count++;

										

									}
								}

						
							
								MPI_Send(&send_vector,size_of_vector , MPI_INT,j,4,MPI_COMM_WORLD);
									
								
							

							}
							
							


						}


						//assemble the matrix
						int lineCounter = 0;

						for(k = 0 ; k <  numberOfLinesEach ; k++) {
							
						
							for(l = 0; l < input.width  ; l ++) {
								input.matrix_gray[k][l].value = local_matrix[k][l].value;
								

							}

						}

						lineCounter = numberOfLinesEach;


						for(j = rank+1 ; j < nProcesses ; j++){

							int dimension;
							MPI_Recv(&dimension,1,MPI_INT,j,MPI_ANY_TAG, MPI_COMM_WORLD,  NULL );

							int count = 0;

							int recv_vector[dimension];
							MPI_Recv(&recv_vector,dimension,MPI_INT,j,MPI_ANY_TAG, MPI_COMM_WORLD,  NULL );

							int end = lineCounter + numberOfLinesEach;
							if(j == nProcesses - 1){

								end = input.height;
							}
							for(k = lineCounter ; k < end ; k++) {
							
						
								for(l = 0; l < input.width  ; l ++) {

									input.matrix_gray[k][l].value = (unsigned char)recv_vector[count];
									count++;

									

								}

							}
							
							lineCounter = end;

						}

						

					}
				}
			}


		

		writeData(argv[2], &input);

	} else if(rank == nProcesses - 1){ //the last process

		

		char type;
		MPI_Recv(&type,1,MPI_CHAR,0,MPI_ANY_TAG,MPI_COMM_WORLD,NULL); // the type of the image (color or grayscale)

		int width;
		MPI_Recv(&width,1,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,NULL);
	

			if(type == '6'){ //color
				for(i = 0; i < numberOfFilters; i ++){
					


					int size_of_vector;
					MPI_Recv(&size_of_vector,1,MPI_INT,0,MPI_ANY_TAG, MPI_COMM_WORLD,  NULL );

				
					int recv_vector[size_of_vector];
					MPI_Recv(&recv_vector,size_of_vector,MPI_INT,0,MPI_ANY_TAG, MPI_COMM_WORLD,  NULL );

				
					int count = 0;
					pixel **local_matrix = (pixel **)malloc(size_of_vector / (width * 3) * sizeof(pixel*));
					pixel **local_matrix_aux = (pixel **)malloc(size_of_vector / (width * 3) * sizeof(pixel*));
						int k,l;
						for(k = 0 ; k <  size_of_vector / (width * 3) ; k++){
							local_matrix[k] = (pixel *)malloc(width * sizeof(pixel));
							local_matrix_aux[k] = (pixel *)malloc(width * sizeof(pixel));
						
							for(l = 0 ; l < width ; l ++){
								
								local_matrix[k][l].red = (unsigned char)recv_vector[count];
								local_matrix_aux[k][l].red = (unsigned char)recv_vector[count];
								count++;
								local_matrix[k][l].green = (unsigned char)recv_vector[count];
								local_matrix_aux[k][l].green = (unsigned char)recv_vector[count];
								count++;
								local_matrix[k][l].blue = (unsigned char)recv_vector[count];
								local_matrix_aux[k][l].blue = (unsigned char)recv_vector[count];
								count++;
								

							}
						}

					

					for(k = 1 ; k <  size_of_vector / (width * 3) -1; k++){
							
						
							for(l = 1 ; l < width -1; l ++){
								
								float sumred = 0, sumgreen =0, sumblue =0;

								float kernel[3][3];

								updateMatrix(kernel,filters[i]);


								int ki = 0;
								int kj = 0;

								for(int x = k-1; x <= k+1; x++){

									for(int y = l-1; y<= l+1; y++){

										sumred +=  (float)(kernel[ki][kj] * (float)local_matrix_aux[x][y].red) ;
										sumgreen += (float)(kernel[ki][kj] * (float)local_matrix_aux[x][y].green);
										sumblue +=  (float)(kernel[ki][kj] * (float)local_matrix_aux[x][y].blue);



										kj++;
									}
									ki++;
									kj=0;
								}


								local_matrix[k][l].red = (unsigned char)sumred;	
								local_matrix[k][l].green = (unsigned char)sumgreen;
								local_matrix[k][l].blue = (unsigned char)sumblue;

							}
						}




						count = 0;
						int send_recv_vector[size_of_vector - width * 3 ]; //without the extra line
						for(k = 1 ; k <  size_of_vector / (width * 3) ; k++){
							
						
							for(l = 0 ; l < width ; l ++){
								
								
								send_recv_vector[count] = (int) local_matrix[k][l].red;
								
								count++;
								
								send_recv_vector[count] = (int) local_matrix[k][l].green;
								
								count++;
								
								send_recv_vector[count] = (int) local_matrix[k][l].blue;
								
								count++;
								

							}
						}


						int dimension = size_of_vector - width * 3;
						MPI_Send(&dimension,1 , MPI_INT,0,4,MPI_COMM_WORLD);
						MPI_Send(&send_recv_vector,dimension , MPI_INT,0,4,MPI_COMM_WORLD);


				}
			} else if(type == '5'){ //grayscale

				for(i = 0; i < numberOfFilters; i ++){
					

					
				
				
					int size_of_vector;
					MPI_Recv(&size_of_vector,1,MPI_INT,0,MPI_ANY_TAG, MPI_COMM_WORLD,  NULL );

				

				
					int recv_vector[size_of_vector];
					MPI_Recv(&recv_vector,size_of_vector,MPI_INT,0,MPI_ANY_TAG, MPI_COMM_WORLD,  NULL );

				
					int count = 0;
					pixel_gray **local_matrix = (pixel_gray **)malloc(size_of_vector / width * sizeof(pixel_gray*));
					pixel_gray **local_matrix_aux = (pixel_gray **)malloc(size_of_vector / width * sizeof(pixel_gray*));
						int k,l;
						for(k = 0 ; k <  size_of_vector / width ; k++){
							local_matrix[k] = (pixel_gray *)malloc(width * sizeof(pixel_gray));
							local_matrix_aux[k] = (pixel_gray *)malloc(width * sizeof(pixel_gray));
						
							for(l = 0 ; l < width ; l ++){
								
								local_matrix[k][l].value = (unsigned char)recv_vector[count];
								local_matrix_aux[k][l].value = (unsigned char)recv_vector[count];
								count++;
								
								

							}
						}

					

					for(k = 1 ; k <  size_of_vector / width -1; k++){
							
						
							for(l = 1 ; l < width -1; l ++){
								
								float sum = 0;

								float kernel[3][3];

								updateMatrix(kernel,filters[i]);


								int ki = 0;
								int kj = 0;

								for(int x = k-1; x <= k+1; x++){

									for(int y = l-1; y<= l+1; y++){

										sum +=  (float)(kernel[ki][kj] * (float)local_matrix_aux[x][y].value) ;
									
										kj++;
									}
									ki++;
									kj=0;
								}


								local_matrix[k][l].value = (unsigned char)sum;	
								

							}
						}




						count = 0;
						int send_recv_vector[size_of_vector - width ]; //without the extra line
						for(k = 1 ; k <  size_of_vector / width ; k++){
							
						
							for(l = 0 ; l < width ; l ++){
								
								
								send_recv_vector[count] = (int) local_matrix[k][l].value;
								
								count++;
								
								
								

							}
						}


						int dimension = size_of_vector - width;
						MPI_Send(&dimension,1 , MPI_INT,0,4,MPI_COMM_WORLD);
						MPI_Send(&send_recv_vector,dimension , MPI_INT,0,4,MPI_COMM_WORLD);


				}

			}



		

	} else {

		
		
			char type;
			MPI_Recv(&type,1,MPI_CHAR,0,MPI_ANY_TAG,MPI_COMM_WORLD,NULL);

			int width;
			MPI_Recv(&width,1,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,NULL);


			if(type == '6'){ //color

				for(i = 0; i < numberOfFilters; i ++){
					


					int size_of_vector;
					MPI_Recv(&size_of_vector,1,MPI_INT,0,MPI_ANY_TAG, MPI_COMM_WORLD,  NULL );


				
					int recv_vector[size_of_vector];
					MPI_Recv(&recv_vector,size_of_vector,MPI_INT,0,MPI_ANY_TAG, MPI_COMM_WORLD,  NULL );


					int count = 0;
					pixel **local_matrix = (pixel **)malloc(size_of_vector / (width * 3) * sizeof(pixel*));
					pixel **local_matrix_aux = (pixel **)malloc(size_of_vector / (width * 3) * sizeof(pixel*));
						int k,l;
						for(k = 0 ; k <  size_of_vector / (width * 3) ; k++){
							local_matrix[k] = (pixel *)malloc(width * sizeof(pixel));
							local_matrix_aux[k] = (pixel *)malloc(width * sizeof(pixel));
						
							for(l = 0 ; l < width ; l ++){
								
								local_matrix[k][l].red = (unsigned char)recv_vector[count];
								local_matrix_aux[k][l].red = (unsigned char)recv_vector[count];
								count++;
								local_matrix[k][l].green = (unsigned char)recv_vector[count];
								local_matrix_aux[k][l].green = (unsigned char)recv_vector[count];
								count++;
								local_matrix[k][l].blue = (unsigned char)recv_vector[count];
								local_matrix_aux[k][l].blue = (unsigned char)recv_vector[count];
								count++;
								

							}
						}


						for(k = 1 ; k <  size_of_vector / (width * 3) -1; k++){
							
						
							for(l = 1 ; l < width -1; l ++){
								
								float sumred = 0, sumgreen =0, sumblue =0;

								float kernel[3][3];

								updateMatrix(kernel,filters[i]);


								int ki = 0;
								int kj = 0;

								for(int x = k-1; x <= k+1; x++){

									for(int y = l-1; y<= l+1; y++){

										sumred +=  (float)(kernel[ki][kj] * (float)local_matrix_aux[x][y].red) ;
										sumgreen += (float)(kernel[ki][kj] * (float)local_matrix_aux[x][y].green);
										sumblue +=  (float)(kernel[ki][kj] * (float)local_matrix_aux[x][y].blue);



										kj++;
									}
									ki++;
									kj=0;
								}


								local_matrix[k][l].red = (unsigned char)sumred;	
								local_matrix[k][l].green = (unsigned char)sumgreen;
								local_matrix[k][l].blue = (unsigned char)sumblue;

							}
						}




						count = 0;
						int send_recv_vector[size_of_vector - width * 3 *2 ]; //without the extra lines
						for(k = 1 ; k <  size_of_vector / (width * 3) -1; k++){
							
						
							for(l = 0 ; l < width ; l ++){
								
								
								send_recv_vector[count] = (int) local_matrix[k][l].red;
								
								count++;
								
								send_recv_vector[count] = (int) local_matrix[k][l].green;
								
								count++;
								
								send_recv_vector[count] = (int) local_matrix[k][l].blue;
								
								count++;
								

							}
						}

						int dimension = size_of_vector - width * 3 *2;
						MPI_Send(&dimension,1 , MPI_INT,0,4,MPI_COMM_WORLD);
						MPI_Send(&send_recv_vector,dimension , MPI_INT,0,4,MPI_COMM_WORLD);
					
				}

				
			} else if(type == '5'){ //grayscale

				for(i = 0; i < numberOfFilters; i ++){
					


					int size_of_vector;
					MPI_Recv(&size_of_vector,1,MPI_INT,0,MPI_ANY_TAG, MPI_COMM_WORLD,  NULL );


				
					int recv_vector[size_of_vector];
					MPI_Recv(&recv_vector,size_of_vector,MPI_INT,0,MPI_ANY_TAG, MPI_COMM_WORLD,  NULL );


					int count = 0;
					pixel_gray **local_matrix = (pixel_gray **)malloc(size_of_vector / width * sizeof(pixel_gray*));
					pixel_gray **local_matrix_aux = (pixel_gray **)malloc(size_of_vector / width * sizeof(pixel_gray*));
						int k,l;
						for(k = 0 ; k <  size_of_vector / width ; k++){
							local_matrix[k] = (pixel_gray *)malloc(width * sizeof(pixel_gray));
							local_matrix_aux[k] = (pixel_gray *)malloc(width * sizeof(pixel_gray));
						
							for(l = 0 ; l < width ; l ++){
								
								local_matrix[k][l].value = (unsigned char)recv_vector[count];
								local_matrix_aux[k][l].value = (unsigned char)recv_vector[count];
								count++;
								
								

							}
						}


						for(k = 1 ; k <  size_of_vector / width -1; k++){
							
						
							for(l = 1 ; l < width -1; l ++){
								
								float sum = 0;

								float kernel[3][3];

								updateMatrix(kernel,filters[i]);


								int ki = 0;
								int kj = 0;

								for(int x = k-1; x <= k+1; x++){

									for(int y = l-1; y<= l+1; y++){

										sum +=  (float)(kernel[ki][kj] * (float)local_matrix_aux[x][y].value) ;
										



										kj++;
									}
									ki++;
									kj=0;
								}


								local_matrix[k][l].value = (unsigned char)sum;	
								

							}
						}



						count = 0;
						int send_recv_vector[size_of_vector - width *2 ]; //without the extra lines
						for(k = 1 ; k <  size_of_vector / width -1; k++){
							
						
							for(l = 0 ; l < width ; l ++){
								
								
								send_recv_vector[count] = (int) local_matrix[k][l].value;
								
								count++;
								
								
								

							}
						}

						int dimension = size_of_vector - width *2;
						MPI_Send(&dimension,1 , MPI_INT,0,4,MPI_COMM_WORLD);
						MPI_Send(&send_recv_vector,dimension , MPI_INT,0,4,MPI_COMM_WORLD);
					
				}

			}


		


	}
	

    MPI_Finalize();
    return 0;

}
