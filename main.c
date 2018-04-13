/*main.c*/

//
// Chicago Crime Lookup via hashing and AVL trees.
//
// Frank Lubek
// Cygwin for Windows 7 64-bit
// Formatted in M.S. Visual Studio
// U. of Illinois, Chicago
// CS251, Fall 2016
// HW #9
//


//////////////Includes and Definitions and Global Variables////////////////

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "avl.h"
#include "mymem.h"

#define TRUE  1
#define FALSE 0


//
// Defines information about one crime:
//
typedef struct Crime
{
	char  caseNum[10];
	char  dateTime[24];
	char  IUCR[5];
	int   arrested;
	int   domestic;
	int   area;
	int   year;
	struct Crime *next;
} Crime;

//
// Defines crime information in different areas in Chicago:
//
typedef struct data
{
	int crimesInEachArea[78];
	int codesForEachArea[10000][78];
	int chicagoCode[10000];
	int total;
}data;

//
// Global variables:
//
int g_collisions = 0;  // instrument # of collisions for reporting:


//////////////////////// START OF FUNCTIONS ///////////////////////////////

//
// Hash function()
//
int hash(char *CaseNum, int N)
{
	//
	// case numbers are 7 or 8 chars long, with the last 6 as digits.  some examples:
	// HZ256372, HY266148, HY299741, HH254987, G219399.  Since there could be millions 
	// of cases, 6 digits are not enough.  so we need to take advantage of the letters 
	// too...
	//
	int len = strlen(CaseNum);

	if (len < 8 || len > 9)  // invalid, e.g. perhaps user enters an invalid case #:
	{
		return -1;
	}

	int advance = len - 6;  // come from the end in case there are more than 2 digits to start:
	char *p = CaseNum + advance;  // ptr to first digit:

	int i = atoi(p);

	assert(i > 0 && i < 1000000);  // 6 meaningful digits:

	if (len > 7) // use the 2nd letter:
	{
		char c = CaseNum[1];

		int diff = abs('Z' - c);
		i = (diff * 1000000) + i;
	}

	// return hash value
	return i % N;  // whatever we have, make sure it falls within 0..N-1:
}

//////////////////////////////////////////////////////////////////////////

//
// parseCrime()
//
// parses given line and stores the fields in the given Crime struct:
// 
//

void parseCrime(char *line, Crime *c, data *info)
{
	// format:
	//
	// Case Number, Date, IUCR, Arrest, Domestic, Beat, District, Ward, Community Area, Year
	// HZ256372, 01/01/2015 12:00:00 AM, 0281, false, false, 0334, 003, 7, 43, 2015
	// HZ257172, 11/24/2015 05:30:00 PM, 0820, false, false, 1124, 011, 28, 27, 2015
	// HY266148, 05/19/2015 01:12:00 AM, 0560, true, false, 1933, 019, 44, 6, 2015
	//
	char *token;

	// case number:
	token = strtok(line, ",");
	assert(token != NULL);
	strcpy(c->caseNum, token);

	// datetime:
	token = strtok(NULL, ",");
	assert(token != NULL);
	strcpy(c->dateTime, token);

	// IUCR:
	token = strtok(NULL, ",");
	assert(token != NULL);
	strcpy(c->IUCR, token);

	// arrested:
	token = strtok(NULL, ",");
	assert(token != NULL);
	if (strcmp(token, "true") == 0)
		c->arrested = TRUE;
	else
		c->arrested = FALSE;

	// domestic:
	token = strtok(NULL, ",");
	assert(token != NULL);
	if (strcmp(token, "true") == 0)
		c->domestic = TRUE;
	else
		c->domestic = FALSE;

	// beat: --- skip
	token = strtok(NULL, ",");
	assert(token != NULL);

	// district: --- skip
	token = strtok(NULL, ",");
	assert(token != NULL);

	// ward: --- skip
	token = strtok(NULL, ",");
	assert(token != NULL);

	// area:
	token = strtok(NULL, ",");
	assert(token != NULL);
	c->area = atoi(token);

	assert(c->area >= 0 && c->area <= 77);

	// year:
	token = strtok(NULL, ",");
	assert(token != NULL);
	c->year = atoi(token);

	token = strtok(NULL, ",");
	assert(token == NULL);  // no more tokens on this line:
}

//////////////////////////////////////////////////////////////////////////

//
//  numCrimesinArea()
//
//function to help compute the number of crimes in each area
//
int numCrimesinArea(int area, int ht_size, Crime **hashtable)
{
	int i;
	int crimeCounter = 0;
	for (i = 0; i < ht_size; ++i)
	{
		if (area == hashtable[i]->area)
		{
			++crimeCounter;
		}
	}
	return crimeCounter;
}

//////////////////////////////////////////////////////////////////////////

//
// inputCrimes()
//
// Inputs the crimes from the given file, building and storing these crimes 
// in a hash table.  Chaining is used to deal with collisions.  A pointer
// to the hash table is returned; the size of the hash table is also returned
// via the 2nd function argument.
//
Crime **inputCrimes(char *filename, int *ht_size, AVLNode *root, data *info)
{
	FILE *input;
	char  line[256];
	int   linesize = sizeof(line) / sizeof(line[0]);
	int   startYear, endYear;
	int   crimeCount = 0;

	//
	// open file, make sure it worked:
	//
	input = fopen(filename, "r");
	if (input == NULL)
	{
		printf("**Error: unable to open '%s'.\n\n", filename);
		exit(-1);
	}

	//
	// read the range of years from 1st line:
	//
	printf(">> reading:   '%s'\n", filename);
	fscanf(input, "%d %d", &startYear, &endYear);
	fgets(line, linesize, input);  // discard rest of line

	printf(">> years:      %d..%d\n", startYear, endYear);

	//
	// allocate space for hash table: assume 300,000 crimes/year, so compute size
	// we need with a 20% load factor (i.e. 80% unused):
	//
	int years = endYear - startYear + 1;
	int N = years * 300000 * 5 /*load factor, 5 => 20%*/;

	Crime **hashtable = (Crime **)mymalloc(N * sizeof(Crime));
	if (hashtable == NULL)  // alloc failed :-(
	{
		printf("**Error: unable allocate memory for hash table (%d).\n\n", N);
		exit(-1);
	}

	//
	// initialize hash table entries to NULL, i.e. empty chains:
	//
	for (int i = 0; i < N; ++i)
		hashtable[i] = NULL;

	// 
	// now start reading the crime data:
	//
	fgets(line, linesize, input);  // discard 2nd line --- column headers

	while (fgets(line, linesize, input))  // start reading data:
	{
		line[strcspn(line, "\r\n")] = '\0';  // strip EOL(s) char at the end:

											 // allocate memory for this crime:
		Crime *c = (Crime *)mymalloc(sizeof(Crime));
		if (c == NULL)  // alloc failed :-(
		{
			printf("**Error: unable allocate memory for Crime struct.\n\n");
			exit(-1);
		}

		// fill crime struct with data from file:
		parseCrime(line, c, info);
///////////////////////////////////////////////////////////
		//convert to int to use for index
		int IUCR2int = atoi(c->IUCR);
		//increment the "community" in 3 struct components
		info->chicagoCode[IUCR2int]++;
		info->crimesInEachArea[c->area]++;
		info->codesForEachArea[IUCR2int][c->area]++;
////////////////////////////////////////////////////////////

		// link into hashtable:
		int index = hash(c->caseNum, N);

		if (hashtable[index] != NULL)
			g_collisions++;

		c->next = hashtable[index];  // existing chain follows us:
		hashtable[index] = c;        // insert @ head:

		crimeCount++;
	}

	fclose(input);

	//
	// stats:
	//
	printf(">> # crimes:   %d\n", crimeCount);
	printf(">> ht size:    %d\n", N);
	printf(">> collisions: %d\n", g_collisions);

	//
	// return hash table pointer and size:
	*ht_size = N;
	info->total = crimeCount;

	return hashtable;
}

//////////////////////////////////////////////////////////////////////////

//
// freeAreas()
//
// Loops through the array of pointers and deallocates memory
//
void freeAreas(char **areas)
{
	// loop and free
	for (int i = 0; i < 78; i++)
	{
		myfree(areas[i]);
	}
	myfree(areas);
}

//////////////////////////////////////////////////////////////////////////

//
// freeCollisions()
//
// This function is designed to walk through a linked list, freeing all elements
//
void freeColisions(Crime *root)
{
	Crime *cur = root;
	Crime *next = NULL;

	while (cur != NULL)
	{
		next = cur->next;
		myfree(cur);
		cur = next;
	}// end while
}

//////////////////////////////////////////////////////////////////////////

//
// freeHT()
//
// This function is designed to loop through the hashtable. Upon reaching
// a non-NULL bucket, calls freeCollisions to empty the bucket's list.
//
void freeHT(Crime **hashtable, int N)
{
	for (int i = 0; i < N; i++)
	{
		if (hashtable[i] != NULL)
		{
			Crime *current = hashtable[i];

			if (current->next == NULL)
			{
				myfree(hashtable[i]);
			}
			else
			{
				freeColisions(hashtable[i]);
			}// end else
		}// end if
	}// end for 
	myfree(hashtable);
	return;
}

//////////////////////////////////////////////////////////////////////////

//
// inputAreas
//
// Reads the file of chicago community areas, of which there should be 78.
// Returns pointer to array of community names.
//
char **inputAreas(char *filename)
{
	FILE *input;
	char  line[256];
	int   linesize = sizeof(line) / sizeof(line[0]);

	//
	// open file, check to make sure it worked:
	//
	input = fopen(filename, "r");
	if (input == NULL)
	{
		printf("**Error: unable to open '%s'.\n\n", filename);
		exit(-1);
	}

	printf(">> reading:   '%s'\n", filename);

	//
	// allocate array for community names:
	//
	int N = 78;
	char **areas = (char **)mymalloc(N * sizeof(char *));

	//
	// now read the names and fill the array:
	//
	fgets(line, linesize, input);  // discard 1st line --- column headers

	int count = 0;

	while (fgets(line, linesize, input))  // start reading data:
	{
		char *token;

		line[strcspn(line, "\r\n")] = '\0';  // strip EOL(s) char at the end:

											 //
											 // format:  area #, area name
											 //
		token = strtok(line, ",");
		assert(token != NULL);

		int index = atoi(line);

		token = strtok(NULL, ",");
		assert(token != NULL);

		int len = strlen(token) + 1;
		char *p = (char *)mymalloc(len * sizeof(char));
		strcpy(p, token);

		areas[index] = p;

		token = strtok(NULL, ",");
		assert(token == NULL);  // no more tokens on this line:

		count++;
	}

	assert(count == N);

	fclose(input);

	//
	// return areas array pointer and size:
	//
	return areas;
}

//////////////////////////////////////////////////////////////////////////

//
// getRatio()
//
// This function is designed to perform integer division, checking for the 
// case where the denominator equals 0. 
//
double getRatio(int n, int d)
{
	double returnValue = 0.0;
	if (d == 0) 
	{
		printf("\n\n** Error: Dividing by 0 will result in the implosion of Earth!!\n\n");
		return returnValue;
	}
	returnValue = (n / (double)d) * 100;
	return returnValue;
}

//////////////////////////////////////////////////////////////////////////

//
// outputCrimeInfo()
//
// This function is designed to print the information requested upon 
// entering a valid case number
//
void outputCrimeInfo(AVLNode *root, Crime *c, char *areas[], data *info)
{

	int IUCR2int = atoi(c->IUCR);

	// Output case number information
	printf("%s:\n", c->caseNum);
	printf("  date/time: %s\n", c->dateTime);
	printf("  city area: %d => %s\n", c->area, areas[c->area]);
	printf("  IUCR code: %s => ", c->IUCR);
	outputInfo1(root, c->IUCR);
	printf("  arrested:  %s\n", ((c->arrested) ? "true" : "false"));
	printf("  domestic violence:  %s\n\n", ((c->domestic) ? "true" : "false"));

	// Output area information
	printf("AREA: %d => %s\n", c->area, areas[c->area]);
	printf("  # of crimes in area: %d\n", info->crimesInEachArea[c->area]);
	printf("  %% of Chicago crime:  %lf%%\n\n", getRatio(info->crimesInEachArea[c->area], info->total));

	// Output Crime information
	printf("CRIME: %s => ", c->IUCR);
	outputInfo1(root, c->IUCR);
	printf("  # of THIS crime in area:    %d\n", info->codesForEachArea[IUCR2int][c->area]);//outputTimesCommitted(root, c->IUCR));
	printf("  %% of THIS crime in area:    %lf%%\n", getRatio(info->codesForEachArea[IUCR2int][c->area], info->crimesInEachArea[c->area]));//temp);
	printf("  %% of THIS crime in Chicago: %lf%%\n", getRatio(info->chicagoCode[IUCR2int], info->total));//pCofC);

}

//////////////////////////////////////////////////////////////////////////

//
// initCrimeData()
//
// This function is designed to set all elements of the struct data to 0/
//
void initCrimeData(data *info)
{
	int i, j;
	for (i = 0; i < 10000; ++i)
	{
		info->chicagoCode[i] = 0;
		for (j = 0; j < 78; ++j)
		{
			info->codesForEachArea[i][j] = 0;
			info->crimesInEachArea[j] = 0;
		}//end for
	}//end for
}

//////////////////////////////////////////////////////////////////////////

//
// Main:
//
int main()
{
	printf("\n** Chicago Crime Lookup Program **\n\n");

	Crime **hashtable;  // array of Crime pointers:
	int     N;          // size of hash table
	char  **areas;      // array of community names (strings):
	AVLNode *root = NULL;
	data *info = (data*)mymalloc(sizeof(data));
	initCrimeData(info);


	areas = inputAreas("Areas.csv");
	hashtable = inputCrimes("Crimes.csv", &N, root, info);
	root = Build("Codes.csv");

	printf("\n");



	//
	// crime lookup loop:
	//
	char line[256];
	int  linesize = sizeof(line) / sizeof(line[0]);

	printf("Enter a case number> ");
	fgets(line, linesize, stdin);
	line[strcspn(line, "\r\n")] = '\0';  // strip EOL(s) char at the end:

	while (strlen(line) > 0)
	{
		int index = hash(line, N);

		if (index < 0)
		{
			printf("** invalid case #, try again...\n");
		}
		else
		{
			printf(">> hash index: %d <<\n", index);

			//
			// walk along the chain and see if we can find this case #:
			//
			Crime *c = hashtable[index];

			while (c != NULL)
			{
				if (strcmp(line, c->caseNum) == 0)  // found it!
					break;

				// otherwise keep looking:
				c = c->next;
			}

			// if get here, see if we found it:
			if (c == NULL)
				printf("** Case not found...\n");
			else
			{

				outputCrimeInfo(root, c, areas, info);
			}
		}

		printf("\n");
		printf("Enter a case number> ");
		fgets(line, linesize, stdin);
		line[strcspn(line, "\r\n")] = '\0';  // strip EOL(s) char at the end:
	}

	//
	// done:
	//
	printf("\n\n** Done **\n\n");


	// run Prof J Hum's memory statistics code and deallocate all memory
	mymem_stats();
	printf("** Freeing memory...\n");
	freeHT(hashtable, N);
	freeAreas(areas);
	freeRoot(root);
	myfree(info);
	mymem_stats();
	printf("\n\n");

	return 0;
}
