#include "udf.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



int n_time; 			/*create variable to store integer number of timesteps*/
real PressureAbove; 	/*Variable that stores average pressure above BOP*/
real PressureBelow; 	/*Variable that stores average pressure below BOP*/
real PressDiff;			/*Variable that stores piezometric head across BOP*/
int TS;
real nfaces;			/*Variable to store the number of faces that are being looped over - will be used to average pressure values*/
int counter; 			/*Something strange is going on for if(first_iteration)... it's looping 3 times through first iteration. This counter will prevent that--->after testing it... it doesn't. Still runs 3 times*/
real A,PBOP, Phead, Coef1, Coef2, Coef3,K,C; /*variables for hydraulic transient simulation*/
int x,t; /*variables for hydraulic transient simulation*/

real g = 9.81; /*gravity*/

/*****/
		real a; /*Sonic Velocity*/
		real Length; /*length of pipe*/
		real dt; /*timestep*/
		real f; /*friction factor, constant*/
		real D;  /*equivalent diameter of pipe/annulus*/
		real Vinit;  /*initial velocity*/
		int nx; /*number of discretizations in length-----hard coded because I can't figure out how to convert to int-----MUST BE CHANGED APPROPRIATELY*/
		FILE *inFile;
		char array[256];
		real ValIDM;
		real numberFromFile;
		int Zone_ID_Below;
/******/

real dx; /*discretization length*/
real V[10000],Vnew[10000],Hold[10000],Hnew[10000]; /*Again hard coded... because I need to fix this. current issue is that nx isn't known at compile time*/

real test = 0; /*Test for negative  at valve(representing shut in)*/
real fraction; /*fractional frictional pressure drop per unit discretization length*/


DEFINE_PROFILE(VelBC, thread, position)
{
	


	
	face_t face; 			/*Create variable for the face to loop through and assign BC*/
	Thread *tf; 		/*Create variable for the thread*/
	cell_t c; 			/*Create variable for the cells to loop through to grab pressure*/
	int Zone_ID; 		/*Create variable for the zone ID which is where pressure is grabbed from*/
	
	Domain *domain;          /* domain is declared as a variable   */
	domain = Get_Domain(1);  /* returns fluid domain pointer       */
	
	/*Calculate and initialize some things when the initialization button is pressed in Fluent*/
	if ( CURRENT_TIME == 0 )
	{
		
		n_time = 0;
		Message("Initializing variables and n_time \n");
		/*find file and open initial velocity reading---should also get well parameters, zone IDS, etc*/

		
		
		
		
		inFile = fopen("inputDeck.dat","r");
		ValIDM = 0;
		if (inFile == NULL)
		{
			Message("Can't find input file");
		}
		if (inFile != NULL)
		{
			Message("Found File");
		}
		
		/*Read values from text file*/
		fscanf(inFile, "%lg", &a);
		Message("a %lg \n",a);
		fscanf(inFile, "%lg", &Length);
		Message("Length %lg \n",Length);
		fscanf(inFile, "%lg", &dt);
		Message("dt %lg \n",dt);		
		fscanf(inFile, "%lg", &f);
		Message("f %lg \n",f);
		fscanf(inFile, "%lg", &D);
		Message("D %lg \n",D);
		fscanf(inFile, "%lg", &Vinit);
		Message("Vinit %lg \n",Vinit);
		fscanf(inFile, "%d", &nx);
		Message("nx %d \n",nx);
		fscanf(inFile, "%d", &Zone_ID_Below);
		Message("nx %d \n",Zone_ID_Below);		
		
		fclose(inFile);

		
		
		
		
		
		A = 3.14*pow(D,2)/4; /*area of pipe*/
		dx = dt*a; /*discretization length*/
		K = f*dt/2./D;
		C = g/a;

		
	
		
		/* ---------------------------------------------------------------------------------------------------------------------- */
		/* Grab pressure from Bottom for top of domain. We are initializing the piezometric head BC for the top of the pipe domain*/
		/* ---------------------------------------------------------------------------------------------------------------------- */
		Zone_ID = Zone_ID_Below;
		tf = Lookup_Thread(domain, Zone_ID);
			
		nfaces = 0; /*initialize the nfaces variable back to 0*/
		PressureBelow = 0; /*Initialize the pressure value*/

		/* loop over all cells within the cell thread where c is just an */
		/* integer number, enumerating the cells. If your thread holds for */
		/* instance 100 cells the loop wil be perfomed 100 times, each time */
		/* c is increased by one starting from zero --- from https://www.cfd-online.com/Forums/fluent-udf/38375-udf-problem.html */ 
		begin_c_loop(c,tf) 
			{ 
			nfaces = nfaces + 1;
			PressureBelow  = PressureBelow + C_P(c, tf); /*Sum up all pressure values across the cell faces*/
			}
		end_c_loop(c, tf)
		PressureBelow = PressureBelow/nfaces;
		PressureBelow = (PressureBelow)/9.81/1000;		/*Average the sum of all pressures*/
		PBOP = PressureBelow; 							/*initial hydraulic head*/
		
		Message("C %lf \n",C); /*Check some values to make sure they are calculated correctly/make sense*/
		Message("a %lf \n",a);
		Message("f %lf \n",f);
		Message("dt %lf \n",dt);
		Message("D %lf \n",D);
		Message("A %lf \n",A);
		Message("dx %lf \n",dx);
		Message("K %lf \n",K);
		Message("PBOP %lf \n",PBOP);
		
		
		
		
		/* */
		/* Storing old Head Values as initialization */
		/*  */
		/* */ 
		Message("Piezometric Head Values \n");
		inFile = fopen("PressureValuesPrevious.dat","r");
		if (inFile == NULL)
		{
			Message("Can't find input file");
		}
		if (inFile != NULL)
		{
			Message("Found File");
		}
		for(x = 0; x < nx; x++ ){
			ValIDM = 0;
			/*Read values from text file*/
			fscanf(inFile, "%lg", &Hold[x]);
			Message("Hold =  %lg \n",Hold[x]);	
		}
		fclose(inFile);
		
		for(x = 0; x < nx; x++ ){
			V[x] = Vinit;	
		}
		Phead = Hold[0]; /*constant reservoir boundary condition*/
		
		fraction = dx*pow(Vinit,2)*f/2.0/g/D;
		Message("Hold diff =  %lf \n",Hold[55]-Hold[54]);
		Message("fraction =  %lf \n",fraction);
		
		
	}
	
	counter = 0; /*reason for creation is stated above in declaration of counter*/
	
	/*Only perform these calculations during the first iteration*/
	if ( first_iteration )
		{
			if (counter == 0)
			{
			counter = 1; /*set counter to 1, so that only one set of calculations occur*/
			
			Message("PressureAbove %lf \n",PressureAbove);
			
			/* -------------------------------------------- */
			/* Grab pressure from Zone for bottom of domain */
			/* -------------------------------------------- */
			
			Zone_ID = Zone_ID_Below;
			tf = Lookup_Thread(domain, Zone_ID);
			nfaces = 0; /*initialize the nfaces variable back to 0*/
			PressureBelow = 0; /*Initialize the pressure value*/
			
			/*Again, loop through faces, but for a different zone*/
			begin_c_loop(c,tf) 
				{ 
				nfaces = nfaces + 1;
				PressureBelow  = PressureBelow + C_P(c, tf); /*Sum up all pressure values across the cell faces*/
				}
			end_c_loop(c, tf)
			PressureBelow = PressureBelow/nfaces; /*Average the sum of all pressures*/
			
			
			
			PressureBelow = (PressureBelow)/9.81/1000; /*Calculate the piezometric head below the valve*/
			Message("PressureBelow %lf \n",PressureBelow);
			
			
			/* --------------------------------------------------------------------------------------------------------------------*/
			/* ----------------------------------SECTION OF CODE FOR WATER HAMMER BELOW--------------------------------------------*/
			/* --------------------------------------------------------------------------------------------------------------------*/
			    
			if (test < 0.5){
				Vnew[0] = V[1]+C*(Hold[0]-Hold[1])-K*(V[1]*abs(V[1]));
				Hnew[0] = Phead;
				
				Hnew[nx-1] = PressureBelow;
				Vnew[nx-1] = V[nx-2]-C*(Hold[nx-1]-Hold[nx-2])+K*(V[nx-2]*abs(V[nx-2]));
			}
				
			else{
				Vnew[0] = V[1]+C*(Hold[0]-Hold[1])-K*(V[1]*abs(V[1]));
				Hnew[0] = Phead;

				Vnew[nx-1] = 0;
				Hnew[nx-1] = (V[nx-2]-V[nx-1]+K*V[nx-2]*abs(V[nx-2]))/C+Hold[nx-2];
			}

			if (Vnew[nx-1] < 0){
				test = test+1;
				Vnew[0] = V[1]+C*(Hold[0]-Hold[1])-K*(V[1]*abs(V[1]));
				Hnew[0] = Phead;

				Vnew[nx-1] = 0;
				Hnew[nx-1] = (V[nx-2]-V[nx-1]+K*V[nx-2]*abs(V[nx-2]))/C+Hold[nx-2];
			}

			for(x = 1; x < nx-1; x++){
				Coef1 = V[x-1]+V[x+1];
				Coef2 = g/a*(Hold[x-1]-Hold[x+1]);
				Coef3 = f*dt/2/D*(V[x-1]*abs(V[x-1])+V[x+1]*abs(V[x+1]));
				Vnew[x] = 1./2.*(Coef1+Coef2-Coef3);
				
				
				Coef1 = a/g*(V[x-1]-V[x+1]);
				Coef2 = (Hold[x-1]+Hold[x+1]);
				Coef3 = a/g*f*dt/2/D*(V[x-1]*abs(V[x-1])-V[x+1]*abs(V[x+1]));
				Hnew[x] = 1./2.*(Coef1+Coef2-Coef3);
			}
			for(x = 0; x < nx; x++){
				Hold[x] = Hnew[x];
				V[x] = Vnew[x];
			}
			
			/* --------------------------------------------------------------------------------------------------------------------*/
			/* ----------------------------------SECTION OF CODE FOR WATER HAMMER ABOVE--------------------------------------------*/
			/* --------------------------------------------------------------------------------------------------------------------*/
			
			} /* end if_counter*/
		} /*end if_iter*/
	
	
	
	
	
	
	
	

	/*Loop through and assign velocity to the faces of the cells*/
	begin_f_loop(face, thread)
	{
		F_PROFILE(face, thread, position) = V[nx-1];
	}
	end_f_loop(face, thread)
	
	
	
	/*Output Relevant Variables in text file*/
	inFile = fopen("OldPressureValues.dat", "w+");
	for(x = 0; x < nx; x++ ){
			fprintf(inFile, "%lg \n", Hold[x]);
		}
	fclose(inFile);
	
	inFile = fopen("NewPressureValues.dat", "w+");
	for(x = 0; x < nx; x++ ){
			fprintf(inFile, "%lg \n", Hnew[x]);
		}
	fclose(inFile);
	
	
}