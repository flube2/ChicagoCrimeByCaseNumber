/*avl.h*/

///////////// AUTHOR /////////////
//
// AVL tree ADT.
//
// Prof. Joe Hummel
// Visual Studio 2015 on Windows
// U. of Illinois, Chicago
// CS251, Fall 2016
// HW #7: Solution
//


////////////// EDITOR ////////////
//
// Chicago Crime Lookup via hashing and AVL trees.
//
// Frank Lubek
// Cygwin for Windows 7 64-bit
// U. of Illinois, Chicago
// CS251, Fall 2016
// HW #9
//


typedef struct AVLElementType
{
  char  IUCR[5];
  char primary[128];
  char secondary[128];
} AVLElementType;

typedef struct AVLNode
{
  AVLElementType   value;
  int              height;
  int numTimesCommitted;
  struct AVLNode  *left;
  struct AVLNode  *right;
} AVLNode;

AVLNode *Contains(AVLNode *root, AVLElementType value);
AVLNode *Insert(AVLNode *root, AVLElementType value);

int Count(AVLNode *root);
int Height(AVLNode *root);


AVLNode *buildAVL(AVLNode *root, char* filename);

AVLNode *Build(char* filename);

void outputInfo1(AVLNode *root, char *value);

void PrintInorder(AVLNode *root);

void updateTimesEachCrimeCommitted(AVLNode *root, char *IUCR);

int outputTimesCommitted(AVLNode *root, char *IUCR);

void freeRoot(AVLNode *root);


