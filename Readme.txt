				Homework 3
				Lidia Istrate 335CC



	In the file homework.c I create the structures : pixel, pixel_gray, image.

	I also create the functions : readInput , writeData.

	updateMatrix -> computes the kernel matrix from the filter name

	In main I spread the job across the processes : 

		- the first process (rank = 0) : 
					- reads the image
					- checks if it is color or grayscale and for the number of filters 
					computes the new image 

					- if there is only one process, than it does all the work 

					- if there are more processes , it spreads parts of the matrix to 
						the other processes : it keeps for itself a part meaning 
						numberOfLinesEach = input.height / nProcesses ---> rows but also +1 
						in order to compute the new pixels 
					- it sends to all the others  numberOfLinesEach rows through a vector and if
						 it is the last process it sends from where it left off until the end,
						 so the last process might at times have more rows than the others 
					- before sending the actual vectors i send through the MPI also the size of 
						the vector to know how many elements i will have
					- where i compute the new matrix I always use a matrix aux to keep the old pixel
						values and i leave the borders of the image unchanged meaning the first and
						last row and the first and last column 
					
					- in the end i assemble the matrix with the pieces received from the other 
						processes and i write out the image

		- for the other processes : i will receive a vector of numberOfLinesEach + 2 meaning those 2 will 
			be the upper and lower row used just to compute the needed pixels and after 
			i send the pixels to the first process

		- for the last process : i will receive a vector of numberOfLinesEach + 1 meaning also the upper row
			 in order to compute the needed pixels and after i send the pixels to the first process



		- the local matrices used are of type pixel or pixel_gray but the vectors sent between processed through MPI
			are of type MPI_INT and for a color image i send in order : 
				red_pixel1 , green_pixel1 , blue_pixel1 , red_pixel2 , green_pixel2 , blue_pixel2 

		- when a filter is over , all the data is sent to the first process so the new image is created and a new filter can be applied
						
