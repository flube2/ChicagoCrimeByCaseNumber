/*avl.c*/

//////////// ORIGINAL AUTHOR ////////////

//
// AVL tree ADT.
//
// Prof. Joe Hummel
// Visual Studio 2015 on Windows
// U. of Illinois, Chicago
// CS251, Fall 2016
// HW #7: Solution
//


//////////// EDITOR //////////////////
//
// Chicago Crime Lookup via hashing and AVL trees.
//
// Frank Lubek
// Cygwin for Windows 7 64-bit
// U. of Illinois, Chicago
// CS251, Fall 2016
// HW #9
//


// Includes and Definitions

#define _CRT_SECURE_NO_WARNINGS
#define TRUE  1
#define FALSE 0
#include "avl.h"
#include "mymem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


//////////////////////// START OF FUNCTIONS ////////////////////////

//
// outputInfo1()
//
// This function is designed to print out crime code info based 
// on an input IUCR. Wasn't sure what to name it.
//
void outputInfo1(AVLNode *head, char *IUCR)
{
	// to pass to Contains() / set up to search
	AVLElementType tmpIUCR;
	strcpy(tmpIUCR.IUCR, IUCR);

	// search	
	AVLNode *temp = Contains(head, tmpIUCR);

	if (temp == NULL)
	{
		printf("UNKNOWN\n");
	}
	else
	{
		printf("%s: %s \n", temp->value.primary, temp->value.secondary);
	}
}

//////////////////////////////////////////////////////////////////////////

//
// outputTimesCommitted()
//
// This function is designed to return (not output to screen)
// the number of times a crime was committed.
//
int outputTimesCommitted(AVLNode *root, char *IUCR)
{
	AVLNode *cur = root;

	while (cur != NULL)
	{
		if (strcmp(IUCR, cur->value.IUCR) == 0)  // match!
		{
			return cur->numTimesCommitted;
		}
		else if (strcmp(IUCR, cur->value.IUCR) < 0)  // smaller, go left:
			cur = cur->left;
		else  // larger, go right:
			cur = cur->right;
	}
	// if get here, not found:
	return -1;
}

//////////////////////////////////////////////////////////////////////////

//
// updateTimesEachCrimeCommitted()
//
// This function is designed to update the number of times a crime 
// was committed.
//
void updateTimesEachCrimeCommitted(AVLNode *root, char *IUCR)
{
	AVLNode *cur = root;

	while (cur != NULL)
	{
		if (strcmp(IUCR, cur->value.IUCR) == 0)  // match!
		{
			cur->numTimesCommitted++;
			return;
		}
		else if (strcmp(IUCR, cur->value.IUCR) < 0)  // smaller, go left:
			cur = cur->left;
		else  // larger, go right:
			cur = cur->right;
	}

	// if get here, not found:
	return;
}

//////////////////////////////////////////////////////////////////////////

//
// freeRoot()
//
// This function is designed to free an AVL tree with a post-order traversal.
//
void freeRoot(AVLNode *root)
{
	if (root == NULL)  // base case: empty tree
	{
		return;
	}
	else  // recursive case: non-empty tree
	{
		freeRoot(root->left);
		//printf("%s: %s %s\n", root->value.IUCR, root->value.primary, root->value.secondary);
		freeRoot(root->right);
		myfree(root);
	}
}

//////////////////////////////////////////////////////////////////////////

//
// Contains()
//
// Searches for the given value, if found, returns
// pointer to node in tree, otherwise NULL is returned.
//
AVLNode *Contains(AVLNode *root, AVLElementType value)
{
	AVLNode *cur = root;

	while (cur != NULL)
	{
		if (strcmp(value.IUCR, cur->value.IUCR) == 0)  // match!
			return cur;
		else if (strcmp(value.IUCR, cur->value.IUCR) < 0)  // smaller, go left:
			cur = cur->left;
		else  // larger, go right:
			cur = cur->right;
	}

	// if get here, not found:
	return NULL;
}

//////////////////////////////////////////////////////////////////////////

//
// rotateRight(), height() helper, and max() helper
//
// Rotate right the sub-tree rooted at node N, return pointer
// to root of newly-rotated sub-tree --- i.e. return pointer
// to node L that was rotated up to top of sub-tree.  Heights
// are adjusted as well after rotation.
//
int _height(AVLNode *N)
{
	if (N == NULL)
		return -1;
	else
		return N->height;
}

///////////////////////////////////////

int _max(int a, int b)
{
	if (a > b)
		return a;
	else
		return b;
}

///////////////////////////////////////

AVLNode *RightRotate(AVLNode *N)
{
	assert(N->left != NULL);  // must have left child to rotate up:

	AVLNode *L = N->left;

	//AVLNode *A = L->left;
	AVLNode *B = L->right;
	//AVLNode *C = N->right;

	//
	// rotate L up, and N down to the right:
	//
	L->right = N;
	N->left = B;

	//
	// recompute heights of nodes that moved:  N, then L
	//
	N->height = 1 + _max(_height(N->left), _height(N->right));
	L->height = 1 + _max(_height(L->left), _height(L->right));

	return L;  // L is the new root of rotated sub-tree:
}

//////////////////////////////////////////////////////////////////////////

//
// LeftRotate()
//
// Rotate left the sub-tree rooted at node N, return pointer
// to root of newly-rotated sub-tree --- i.e. return pointer
// to node R that was rotated up to top of sub-tree.  Heights
// are adjusted as well after rotation.
//
AVLNode *LeftRotate(AVLNode *N)
{
	assert(N->right != NULL);  // must have right child to rotate up:

	AVLNode *R = N->right;

	//AVLNode *A = N->left;
	AVLNode *B = R->left;
	//AVLNode *C = R->right;

	//
	// rotate R up, and N down to the left:
	//
	R->left = N;
	N->right = B;

	//
	// recompute heights of nodes that moved:  N, then R
	//
	N->height = 1 + _max(_height(N->left), _height(N->right));
	R->height = 1 + _max(_height(R->left), _height(R->right));

	return R;  // R is the new root of rotated sub-tree:
}

//////////////////////////////////////////////////////////////////////////

//
// AVL Insert()
//
// Inserts the given value into the AVL tree, rebalancing
// the tree as necessary.  Returns a pointer to the root of
// the new tree; if the value to insert is already in the
// tree, nothing happens and a pointer to the root of the
// original tree is returned.
// 
//
AVLNode *Insert(AVLNode *root, AVLElementType value)
{
	AVLNode *prev = NULL;
	AVLNode *cur = root;

	AVLNode *stack[64];
	int      top = -1;
	int      counter = 0;


	while (cur != NULL)
	{
		top++;
		stack[top] = cur;

		//printf("%d\n", counter);

		if (strcmp(value.IUCR, cur->value.IUCR) == 0)  // already present:
		{
			return root;
		}
		else if (strcmp(value.IUCR, cur->value.IUCR) < 0)  // smaller, go left:
		{
			prev = cur;
			cur = cur->left;
		}
		else  // larger, go right:
		{
			prev = cur;
			cur = cur->right;
		}
		++counter;
	}

	//
	// when get here, insert:
	//
	AVLNode *newNode;

	newNode = (AVLNode *)mymalloc(sizeof(AVLNode));
	newNode->value = value;
	newNode->numTimesCommitted = 1;
	newNode->height = 0;
	newNode->left = NULL;
	newNode->right = NULL;

	if (prev == NULL)  // insert at root:
		root = newNode;
	else if (strcmp(value.IUCR, prev->value.IUCR) < 0)  // insert to left of prev:
		prev->left = newNode;
	else  // insert to the right:
		prev->right = newNode;

	//
	// Now walk back up the tree, updating heights and looking for
	// where the AVL balancing criteria may be broken.  If we reach
	// a node where the height doesn't change, then we're done -- the
	// tree is still balanced.  If we reach a node where the AVL 
	// condition is broken, we fix locally and we're done.  1 or 2 local
	// rotations is enough to re-balance the tree.
	//
	int rebalance = FALSE;

	while (top >= 0)  // walk back up the stack:
	{
		cur = stack[top];
		top--;

		// what's the new height of cur?
		int hl = _height(cur->left);
		int hr = _height(cur->right);
		int newH = 1 + _max(hl, hr);

		if (cur->height == newH)  // hasn't changed, nothing to do!
		{
			rebalance = FALSE;  // no rebalance, exit loop:
			break;
		}
		else if (abs(hl - hr) > 1)  // height changed --- is AVL condition broken?
		{
			rebalance = TRUE;  // yes, so rebalance tree and exit loop to fix:
			break;
		}
		else  // update height in current node and continue walking up tree:
		{
			cur->height = newH;
		}
	}//while

	 //
	 // Okay, does the tree need to be rebalanced?
	 //
	if (rebalance)
	{
		//
		// if we get here, then the AVL condition is broken at "cur".  So we
		// have to decide which of the 4 cases it is and then rotate to fix.
		//

		// we need cur's parent, so pop the stack one more time
		if (top < 0)     // stack is empty, ==> cur is root
			prev = NULL;   // flag this with prev == NULL
		else  // stack not empty, so obtain ptr to cur's parent:
			prev = stack[top];

		// which of the 4 cases?
		if (strcmp(newNode->value.IUCR, cur->value.IUCR)<0)  // to the left => case 1 or 2:
		{

			// case 1 or case 2?  either way, we know cur->left exists:
			AVLNode *L;
			L = cur->left;
			assert(L != NULL);

			// case 2 performs 2 rotations, so check that first:
			if (strcmp(newNode->value.IUCR, L->value.IUCR) > 0)  // to the right => case 2:
			{
				// case 2: left rotate @L 
				cur->left = LeftRotate(L);
			}

			// case 1 and 2: now we right rotate @cur:
			if (prev == NULL)
				root = RightRotate(cur);
			else if (prev->left == cur)
				prev->left = RightRotate(cur);
			else
				prev->right = RightRotate(cur);

		}
		else  // case 3 or 4:
		{
			assert(strcmp(newNode->value.IUCR, cur->value.IUCR)>0);  // was inserted to the right:

																	 // case 3 or case 4?  either way, we know cur->right exists:
			AVLNode *R;
			R = cur->right;
			assert(R != NULL);

			// case 3 performs 2 rotations, so check that first:
			if (strcmp(newNode->value.IUCR, R->value.IUCR)<0)  // to the left => case 3:
			{
				// case 3: right rotate @R 
				cur->right = RightRotate(R);
			}

			// case 3 and 4: now we left rotate @cur
			if (prev == NULL)
				root = LeftRotate(cur);
			else if (prev->left == cur)
				prev->left = LeftRotate(cur);
			else
				prev->right = LeftRotate(cur);

		}
	}

	//
	// done, return ptr to new tree:
	//
	return root;
}

//////////////////////////////////////////////////////////////////////////

//
// Build()
//
// This function is designed to build an AVL tree using parsing techniques 
// and file reading.
//
AVLNode *Build(char* filename)
{
	FILE *source;
	AVLElementType input;
	char line[300];
	char* tempS;
	AVLNode *root = NULL;

	source = fopen(filename, "r");  // open for reading:
	if (source == NULL)
	{
		printf("**Error: unable to open '%s'\n", filename);
		exit(-1);
	}

	fgets(line, 300, source);//Burn headers
	while (fgets(line, 300, source) != NULL)
	{
		line[strcspn(line, "\r\n")] = '\0';

		tempS = strtok(line, ",");//IUCR
		assert(tempS != NULL);
		strcpy(input.IUCR, tempS);

		tempS = strtok(NULL, ",");//Primary
		assert(tempS != NULL);
		strcpy(input.primary, tempS);


		tempS = strtok(NULL, ",");//Secondary
	    assert(tempS != NULL);
		strcpy(input.secondary, tempS);

		tempS = strtok(NULL, ",");

		//printf("Inserting %s %s %s\n", input.IUCR, input.primary, input.secondary);

		root = Insert(root, input);
	}// end while

	fclose(source);
	return root;
}

//////////////////////////////////////////////////////////////////////////

//
// Count()
//
// Returns the # of nodes in the tree.  This is a recursive call that traverses
// the entire tree.
//
int Count(AVLNode *root)
{
	if (root == NULL)  // base case: empty
		return 0;
	else
		return 1 + Count(root->left) + Count(root->right);
}

//////////////////////////////////////////////////////////////////////////

//
// Height()
//
// Returns the overall height of the tree.  Since this is an AVL tree,
// the heights are stored internally so we just look at root node.
//
int Height(AVLNode *root)
{
	if (root == NULL)
		return -1;
	else
		return root->height;
}

//////////////////////////////////////////////////////////////////////////

//
// PrintInorder()
//
// Prints the tree inorder to the console; a debugging function.
//
void PrintInorder(AVLNode *root)
{
	if (root == NULL)  // base case: empty tree
	{
		return;
	}
	else  // recursive case: non-empty tree
	{
		PrintInorder(root->left);
		printf("%s: %s %s\n", root->value.IUCR, root->value.primary, root->value.secondary);
		PrintInorder(root->right);
	}
}

