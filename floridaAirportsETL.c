/* Zach Muller
   Homework 3

   "This program is entirely my own work and I have neither
    developed my code together with another person, nor copied
    from any other person, nor permitted my code to be copied or 
    otherwise used by any other person. I have neither copied, modified
    or otherwise used program code that I have found in any external
    source, including but not limited to online sources."
*/

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <regex.h>

#define BUFFER_SIZE 500 

//Data structure for Airport data
typedef struct airPdata {

	char *LocID;	//Airports "Short Name"
	char *fieldName;	//Actual name
	char *city;		//Location city
	float longitude;	
	float latitude;

} airPdata;

//Data structure for singly-linked list nodes
typedef struct lListAirPdata {
	airPdata *curAirPdata;
	struct lListAirPdata *nextAirPdataList;
} lListAirPdata;

//Data structure for Hash table entry
typedef struct hashTable {
	char key;
	struct airPdata *value;
} hashTable;

//Data structure for AVL tree node
typedef struct AVLtree {
	struct airPdata *data;
	struct AVLtree *left;
	struct AVLtree *right;
	int depth;
} AVLtree;


//Function declarations
//Functions for processing data structure
AVLtree *parseLine(char *line, airPdata *apd, hashTable *HTarray, AVLtree *currentAVLNode, int *depth);
float sexag2decimal(char *degreeString); 
lListAirPdata *populateList(airPdata *apd, lListAirPdata *oldNode);

//Functions used to sort by latitude
//AVLtree *sortByLatitude(AVLtree *currentAVLNode, lListAirPdata *currentNode);
AVLtree *insertNode(AVLtree *currentAVLNode, airPdata *apd);
int max(int a,int b);
int getdepth(AVLtree *Node);
int getBalance(AVLtree *Node);
AVLtree *leftRotate(AVLtree *Node);
AVLtree *rightRotate(AVLtree *Node);
int getseqNumber(int seqNumber);
void displayAVLtreeinOrder(AVLtree *currentAVLNode, int *seqNumber); 
void printData(int length, airPdata *apd);

//functions used to sort alphabetically
hashTable *insertHashValue(hashTable *HTarray, lListAirPdata *currentNode);
void displayHashTable(hashTable *HTarray);

//Kill them all
void deleteStruct(airPdata *apd);







int main (int argc, char *argv[]) {

	// Declare input buffer and other parameters 
	FILE *fid; 
	char buffer[BUFFER_SIZE]; 
	int count = 0;

	// Check for command line input and open input file. 
	if(argc==3){

		fid = fopen(argv[1], "r"); 
		if(fid==NULL){

			printf("File %s failed to open. Aborting.\n", argv[1]); 
			return 2;
		}
	} 
	else{

		printf("Incorect number of input parameters. Please specify the name of the input file and sort parameter.\n");
		printf("Syntax: ./hw1ecl.exe [input file][Sort parameter]\n"); 
		return 1;
	}

	// Determine length of the file. 
	while(fgets(buffer, BUFFER_SIZE, fid) != NULL){
		count++;
	} 
	rewind(fid);

	// Declare a struct array and allocate memory. 
	airPdata *data;
	data = (airPdata *)malloc(sizeof(airPdata)*count); 

	if(data == NULL){

		printf("Memory allocation for airPdata array failed. Aborting.\n"); return 2;
	}

	hashTable *HTarray = (hashTable *)malloc(sizeof(hashTable) * 26);
	AVLtree *currentAVLNode = NULL; /*(AVLtree *)malloc(sizeof(AVLtree));
	currentAVLNode->data = (airPdata *)malloc(sizeof(airPdata));
	currentAVLNode->data = NULL;*/

	int *depth = (int *)malloc(sizeof(int));
	*depth = 0;

	// Read and parse each line of the inputt file. 
	for(int i = 0; i < count; i++) { 

		fgets(buffer, BUFFER_SIZE, fid);

		// fgets() includes the New Line delimiter in the output string. 
		// i.e. "This is my string.\n\0" 
		// We will truncate the string to drop the '\n' if it is there.
		// Note: There will be no '\n' if the line is longer than the buffer.
		if(buffer[strlen(buffer) - 1] == '\n') {

			buffer[strlen(buffer)-1] = '\0';
		}

		currentAVLNode = parseLine(buffer, data+i, HTarray, currentAVLNode, depth);

	}

	// close the input file. 
	fclose(fid);

	//Display data
	char *choice = argv[2];
	

	int *seqNumber = (int *)malloc(sizeof(int));
	*seqNumber = 0;
	if(*choice == 't' || *choice == 'T') {		
		// Output the data to stdout. 
		printData(count, data);
	}
	else if(*choice == 'a' || *choice == 'A') {
		//display alphabetically	
		displayHashTable(HTarray);
	}
	else if(*choice == 'n' || *choice == 'N') {
		//Display South -> North
		printf("seqNumber,code,name,city,lat,lon\n");
		//*seqNumber = getseqNumber(*seqNumber);
		displayAVLtreeinOrder(currentAVLNode, seqNumber);
	}
	else {
		printf("ERROR: sortParameter invalid or not found.\n");
	}
		

	// Free the memory used for fields of the structs. 
	for(int i = 0; i < count; i++){
		deleteStruct(data + i);
	}

	// Free the memory for the struct array. 
	free(data);

	return 0;
}











//read in and format airport data line by line
AVLtree *parseLine(char *line, airPdata *apd, hashTable *HTarray, AVLtree *currentAVLNode, int *depth){
	int i = 0;
	int j = 0;
	int commas = 0;
	int foundHelipad;

	while(commas < 15){ 

		while(*(line + i) != ','){ 
			i++;
		} 

		// strncpy does not append a '\0' to the end of the 
		//copied sub-string, so we will
		// replace the comma with '\0'.
		*(line+i) = '\0';
		switch (commas){ 

			case 1:  
				//Grab the second "field" - Location ID 
				apd->LocID = (char *)malloc(sizeof(char)*(i-j+1));

				if(apd->LocID==NULL){
                    printf("malloc failed to initialize airPdata.LocID.\n");  
                    exit(-1);                        
                }                         

                //Check for Helipads1
                /* Flag didn't work on Eustis, fixed by returning AVLNode instead of setting flag */
  				if(((line+j)[0] - '0' <= 9 && (line+j)[0] - '0' >= 0) || ((line+j)[2] - '0' <= 9 && (line+j)[2] - '0' >= 0)) {
  					foundHelipad = 1;
  					break;
  				}
                strncpy(apd->LocID, line+j, i-j+1);  

                break;

			case 2:   
				//Grab the third "field" - Field Name 
				apd->fieldName = (char *)malloc(sizeof(char) * (i - j + 1));

				if(apd->fieldName == NULL){
					printf("malloc failed to initialize airPdata.fieldName.\n");
					exit(-1);
				} 

				//If we found helipad, skip it
				if(foundHelipad == 1) {
					break;
				}

				strncpy(apd->fieldName, line + j, i - j + 1); 

				break; 

			case 3:   
				//Grab the fourth "field" - City
				apd->city = (char *)malloc(sizeof(char) * (i - j + 1)); 

				if(apd->city == NULL){
					printf("malloc failed to initialize airPdata.city.\n"); 
					exit(-1);
				} 

				//If we found helipad, skip it
				if(foundHelipad == 1) {
					break;
				}

				strncpy(apd->city, line + j, i - j + 1); 

				break; 

			case 8:

				//If we found helipad, skip it
				if(foundHelipad == 1) {
					break;
				}

				{	
					//Grab the ninth "field" - Latitude (sexagesimal string)
					char *latSexString = (char *)malloc(sizeof(char) * strlen(line));

					apd->latitude = sexag2decimal(line+j);
				}	

				break;

			case 9:

				//If we found helipad, skip it
				if(foundHelipad == 1) {
					break;
				}

				{	
					//Grab the tenth "field" - Longitude (sexagesimal string)
					char *longSexString = (char *)malloc(sizeof(char) * strlen(line)); 

					//apd->longitude = sexag2decimal(longSexString);
					apd->longitude = sexag2decimal(line+j);
				}

				break;
		}

		j = ++i; 
		commas++;
	}

	//Create a return node
	lListAirPdata *currentNode = (lListAirPdata *)malloc(sizeof(lListAirPdata));
	//Create space within node to put stuff in
	currentNode->curAirPdata = (airPdata *)malloc(sizeof(airPdata));
	currentNode->nextAirPdataList = (lListAirPdata *)malloc(sizeof(lListAirPdata));

	//Fill linked list
	currentNode = populateList(apd, currentNode);
	currentAVLNode = insertNode(currentAVLNode, apd);
	//Fill hash table to sort alphabetically
	HTarray = insertHashValue(HTarray, currentNode);

	return currentAVLNode;
} 

//Convert latitude and longitude from sexagecimal to decimal degrees
float sexag2decimal(char *degreeString) {

	if(degreeString == NULL){ 
		printf("longitude/latitude = NULL.\n");				
		return 0.0;
	} 

	//return value
	float deciDegrees;
	//Components of return value
	int Degrees;
	float Minutes;
	float Seconds;
	char Direction;

	//Buffers to hold strings
	char *degreeBuffer = (char *)malloc(sizeof(char) * 3);
	char *minuteBuffer = (char *)malloc(sizeof(char) * 2);
	char *secondsBuffer = (char *)malloc(sizeof(char) * 2);

	//Length of sexagecimal string
	int length = strlen(degreeString);

	//First we need to loop through sexagecimal string
	int i;
	int j = 0;
	int dashCounter = 0;
	for(i = 0; i < length; i++) {

		//Skip and count the dashes
		if(degreeString[i] == '-') {
			dashCounter++;
			i++;
			j = 0;
		}

		if(i == (length - 1)) {
			dashCounter = 3;
		}
		switch(dashCounter) {

			//Degrees
			case 0:
				degreeBuffer[i] = degreeString[i];
				break;

			//Minutes
			case 1:
				minuteBuffer[j] = degreeString[i];
				j++;
				break;

			//Seconds
			case 2:
				secondsBuffer[j] = degreeString[i];
				j++;
				break;

			//Direction
			case 3:
				Direction = degreeString[i];
				break;
		}
	}

	//String -> Integer/Float
	//Handle out of range errors
	Degrees = atoi(degreeBuffer);
	if(Degrees < 0 || Degrees > 180) {
		printf("Degrees field out of range.\n");
		return 0.0;
	}

	Minutes = atoi(minuteBuffer);
	if(Minutes < 0.0 || Minutes > 59.0) {
		printf("Minutes field out of range.\n");
		return 0.0;
	}

	Seconds = atoi(secondsBuffer);
	if(Seconds < 0.0 || Seconds > 59.9999) {
		printf("Seconds field out of range.\n");
		return 0.0;
	}

	//Conversion algorithm
	deciDegrees = Degrees + (Minutes / 60) + (Seconds / 3600);

	//Make longitude/latitude negative if S or W
	if(Direction == 'S' || Direction == 'W') {
		deciDegrees *= -1;
	}

	return deciDegrees;
}

lListAirPdata *populateList(airPdata *apd, lListAirPdata *oldNode) {

	//If it's the first node
	if(oldNode == NULL) {
		oldNode->curAirPdata = apd;
		oldNode->nextAirPdataList = NULL;

		return oldNode;
	}
	else {
		//Create new node
		lListAirPdata *newNode = (lListAirPdata *)malloc(sizeof(lListAirPdata));
		newNode->curAirPdata = (airPdata *)malloc(sizeof(airPdata));
		newNode->nextAirPdataList = (lListAirPdata *)malloc(sizeof(lListAirPdata));
		//Populate it
		newNode->curAirPdata = apd;
		//point it to NULL
		newNode->nextAirPdataList = NULL;

		//Point previous node to new node
		*(oldNode->nextAirPdataList) = *newNode;

		return newNode;
	}
}

void printData(int length, airPdata *data){

	printf("seqNumber,code,name,city,lat,lon\n");

	for(int i = 1; i < length; i++){
		if(strcmp((data+i)->LocID, "") == 0) {
			i++;
		}
		else
			printf("%d,%s,%s,%s,%.4f,%.4f\n", i, (data+i)->LocID,(data+i)->fieldName, (data+i)->city, (data+i)->latitude, (data+i)->longitude);
	}
}










AVLtree *insertNode(AVLtree *currentAVLNode, airPdata *apd) {

	//Skip the Helipads
	if(apd->LocID[0] - '0' <= 9 || apd->LocID[2] - '0' <= 9) {
		return currentAVLNode;
	}

	//add node
	//if first node on tree
	if(currentAVLNode == NULL) {

		//allocate space for node elements
		AVLtree *currentAVLNode = (AVLtree *)malloc(sizeof(AVLtree));

		//Populate node
		currentAVLNode->data = apd;
		currentAVLNode->left = NULL;
		currentAVLNode->right = NULL;
		currentAVLNode->depth = 0;
 
		return currentAVLNode;
	}
	//if not first node on tree
	//Add to tree
	//If old node's data is less than new node's data
	if(currentAVLNode->data->latitude < apd->latitude) {
		//Point right pointer to new node
		currentAVLNode->right = insertNode(currentAVLNode->right, apd);	
	}	
	//If old node's data is greater than new node's data
	else if(currentAVLNode->data->latitude > apd->latitude) {
		//point left pointer to new node
		currentAVLNode->left = insertNode(currentAVLNode->left, apd);
	}

	//balance
	//Update depth of new node
	currentAVLNode->depth = max(getdepth(currentAVLNode->left), getdepth(currentAVLNode->right)) + 1;
	//Get balance factor
	int balance = getBalance(currentAVLNode);

	//if left heavy and old < new, left rotate then right rotate
	if(balance > 1 && (currentAVLNode->left->data->latitude < apd->latitude)) {
		currentAVLNode->left = leftRotate(currentAVLNode->left);
		currentAVLNode = rightRotate(currentAVLNode);
	}
	//if right heavy and  old < new, left rotate
	if(balance < -1 && (currentAVLNode->right->data->latitude < apd->latitude)) {
		currentAVLNode = leftRotate(currentAVLNode);
	}
	//left heavy and old > new, right rotate
	if(balance > 1 && (currentAVLNode->left->data->latitude > apd->latitude)) {
		currentAVLNode = rightRotate(currentAVLNode);
	}
	//right heavy and old > new, right rotate, then left rotate
	if(balance < -1 && (currentAVLNode->right->data->latitude > apd->latitude)) {
		currentAVLNode->right = rightRotate(currentAVLNode->right);
		currentAVLNode = leftRotate(currentAVLNode);
	}

	return currentAVLNode;
}

int max(int a, int b) {
	return (a > b)? a : b;
}

// A utility function to get height of the tree
int getdepth(AVLtree *Node)
{		
	    if (Node == NULL)
        return 0;

    return Node->depth;
}

int getBalance(AVLtree *Node) {

	if (Node == NULL)
        return 0;

    return (getdepth(Node->left) - getdepth(Node->right));
}

AVLtree *leftRotate(AVLtree *Node) {

	AVLtree *newRoot = Node->right;
    AVLtree *leftLeaf = newRoot->left;
 	
    // Perform rotation
    newRoot->left = Node;
    Node->right = leftLeaf;
 
    //  Update heights
    Node->depth = max(getdepth(Node->left), getdepth(Node->right))+1;
    newRoot->depth = max(getdepth(newRoot->left), getdepth(newRoot->right))+1;
 
    // Return new root
    return newRoot;
}

AVLtree *rightRotate(AVLtree *Node) {

	AVLtree *newRoot = Node->left;
    AVLtree *rightLeaf = newRoot->right;
 
    // Perform rotation
    newRoot->right = Node;
    Node->left = rightLeaf;
 
    // Update depths
    Node->depth = max(getdepth(Node->left), getdepth(Node->right))+1;
    newRoot->depth = max(getdepth(newRoot->left), getdepth(newRoot->right))+1;
  
    // Return new root
    return newRoot;
}

int getseqNumber(int seqNumber) {
	seqNumber+=1;
	return seqNumber;
}

void displayAVLtreeinOrder(AVLtree *currentAVLNode, int *seqNumber) {
	if(currentAVLNode != NULL) {
		displayAVLtreeinOrder(currentAVLNode->left, seqNumber);
		*seqNumber += 1;
    	printf("%d,%s,%s,%s,%.4f,%.4f\n", *seqNumber, currentAVLNode->data->LocID, currentAVLNode->data->fieldName, currentAVLNode->data->city, currentAVLNode->data->latitude, currentAVLNode->data->longitude);
      	displayAVLtreeinOrder(currentAVLNode->right, seqNumber);
	}
}









/* Function to insert struct node into
hash table as values */
hashTable *insertHashValue(hashTable *HTarray, lListAirPdata *currentNode) {

	//Get the first char from LocID for the current key
	char currentKey = currentNode->curAirPdata->LocID[0];

	//Skip LocID that start with a number
	switch(currentKey) {
		case '0':
			return HTarray;
		case '1':
			return HTarray;
		case '2':
			return HTarray;
		case '3':
			return HTarray;
		case '4':
			return HTarray;
		case '5':
			return HTarray;
		case '6':
			return HTarray;
		case '7':
			return HTarray;
		case '8':
			return HTarray;
		case '9':
			return HTarray;
	}

	int i;
	int j;
	for(i = 0; i < 26; i++) {
		//If the key letter already exists in table, skip it
		if(HTarray[i].key == currentKey) {
			return HTarray;
		}

		//If the slot is empty fill it 
		if(HTarray[i].key == '\0') {
			hashTable *hashSlot = (hashTable *)malloc(sizeof(hashTable));
			hashSlot->key = currentKey;
			hashSlot->value = (airPdata *)malloc(sizeof(airPdata));
			hashSlot->value = currentNode->curAirPdata;
			HTarray[i] = *hashSlot;

			return HTarray;
		}

		//Sort table (bubble)
		for(j = 0; j < 26 - i; j++) {
			if(((HTarray[i].key - '0') < (HTarray[i-1].key - '0')) && (i != 0)) {
				hashTable *tempSlot = (hashTable *)malloc(sizeof(hashTable));

				*tempSlot = HTarray[i];
				HTarray[i] = HTarray[i-1];
				HTarray[i-1] = *tempSlot;

				free(tempSlot);
			}
		}
	}
	return HTarray;
}

//Function to display hash table
void displayHashTable(hashTable *HTarray) {
	//Print header
	printf("seqNumber,code,name,city,lat,lon\n");

	int i;
	for(i = 0; i < 26; i++) {
		printf("%d,%s,%s,%s,%.4f,%.4f\n", i+1, (HTarray[i].value)->LocID, (HTarray[i].value)->fieldName, (HTarray[i].value)->city, (HTarray[i].value)->latitude, (HTarray[i].value)->longitude);
		//If we're at the last element in the table, return
		if(HTarray[i].key == 'Z') {
			return;
		}
	}
}







void deleteStruct(airPdata *apd){

	free(apd->city); 
	free(apd->fieldName); 
	free(apd->LocID); 
}