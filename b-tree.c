#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

struct record {
	char *title;
	char *year;
	char *actor;
	char *description;
};

struct node {
	short leaf;
	unsigned int keysNo;
	struct pointer *listStart;
	struct node *rightSibling;
	struct node *leftSibling;
};

union TwoWayPointer {
	struct record *r;
	struct node *n;
};

struct pointer {
	union TwoWayPointer p;
	struct key *next;
};

struct key {
	char *key;
	struct pointer *next;
};

struct stack {
	struct node *parent;
	struct pointer *pointer;
	struct stack *next;
};

enum previousAction {NONE, MERGE_LEFT, MERGE_RIGHT, REDIST_LEFT, REDIST_RIGHT};

enum redistWith {LEFT, RIGHT};

struct Dstack {
	struct node *node;
	struct pointer *leftPointer;
	struct pointer *centralPointer;
	struct pointer *rightPointer;
	enum previousAction previousAction;
	char *newKey;
	struct Dstack *next;
};

struct leafStack {
	struct node *leaf;
	struct stack *stack;
};

/* Global variables */
struct node *B_tree = NULL;
int n;
/* End of global variables */

short isLeaf(struct node *node) {

	return ( node->leaf == 1 ) ? 1 : 0;

}

short isFull(struct node *node) {

	return ( node->keysNo > (n-1) ) ? 1 : 0;

}

char * copyKey(char *src) {

	char *dst = (char *)malloc(strlen(src)+1);
	strcpy(dst,src);
	return dst;

}

void insertInLeaf(struct node *leaf, char* key, struct record *r) {

	struct pointer *newPointer = (struct pointer *)malloc(sizeof(struct pointer));
	struct key *newKey = (struct key *)malloc(sizeof(struct key));
	newPointer->next = newKey;
	newPointer->p.r = r;
	newKey->key = copyKey(key);

	if (leaf->listStart == NULL) {//That can only happen with an empty root.
		newKey->next = NULL;
		leaf->listStart = newPointer;
		++(leaf->keysNo);
		return;
	}

	struct key *currK = leaf->listStart->next , *prevK = NULL;
	if (strcmp(key, currK->key) <= 0) { //Put the new pointer-key pair at the begining of the list
		newKey->next = leaf->listStart;
		leaf->listStart = newPointer;
	}
	else {
		int i = 1, t = leaf->keysNo;
		short atEnd = 0;
		while ((strcmp(key,currK->key) > 0)) {
			if (i == t) {
				atEnd = 1;
				break;
			}
			else {
				++i;
				prevK = currK;
				currK = (currK->next)->next;
			}
		}
		if (atEnd) {
			newKey->next = NULL;
			currK->next = newPointer;
		}
		else {
			newKey->next = prevK->next;
			prevK->next = newPointer;
		}
	}

	++(leaf->keysNo);

}

char* breakLeaf(struct node *oldLeaf, struct node *newLeaf) {

	assert(oldLeaf->keysNo == n);
	int t = (int)ceil(n/2.0);
	struct key *currK = (oldLeaf->listStart)->next;
	char *ret;
	int i = 1;

	while (i<t) {
		currK = (currK->next)->next;
		++i;
	}
	newLeaf->listStart = currK->next;
	currK->next = NULL;
	newLeaf->rightSibling = oldLeaf->rightSibling;
	oldLeaf->rightSibling = newLeaf;
	newLeaf->keysNo = oldLeaf->keysNo - t;
	oldLeaf->keysNo = t;
	newLeaf->leaf = 1;

	ret = copyKey(((newLeaf->listStart)->next)->key);
	return ret;

}

void createRootLeaf(char *key, struct record *r) {

	struct pointer *newPointer = (struct pointer *)malloc(sizeof(struct pointer));
	struct key *newKey = (struct key *)malloc(sizeof(struct key));
	newPointer->p.r = r;
	newPointer->next = newKey;
	newKey->key = copyKey(key);
	newKey->next = NULL;

	B_tree = (struct node *)malloc(sizeof(struct node));
	B_tree->leaf = 1;
	B_tree->rightSibling = NULL;
	B_tree->leftSibling = NULL;
	B_tree->listStart = newPointer;
	B_tree->keysNo = 1;

}

struct node * nextNodeSimple(struct node *curr, char *key) {

	int i = 0;
	struct pointer *currP = curr->listStart;
	struct key *currKey = currP->next;
	struct pointer *p;
	while ((strcmp(key,currKey->key) > 0)) {
		++i;
		if (i == curr->keysNo) {
			break;
		}
		else {
			currP = currKey->next;
			currKey = currP->next;
		}
	}

	if (i == curr->keysNo) { //The key is bigger than any other key in this node.
		p = currKey->next;
	}
	else if (strcmp(key, currKey->key) == 0) {
		p = currKey->next;
	}
	else { //strcmp(key, currKey->key) < 0
		p = currP;
	}

	return p->p.n;

}


struct node * nextNodeWithStack(struct node *curr, char *key, struct stack **stack) {

	int i = 0;
	struct stack *top = (struct stack *)malloc(sizeof(struct stack));
	struct pointer *currP = curr->listStart;
	struct key *currKey = currP->next;
	struct pointer *res;
	while ((strcmp(key,currKey->key) > 0)) {
		++i;
		if (i == curr->keysNo) {
			break;
		}
		else {
			currP = currKey->next;
			currKey = currP->next;
		}
	}

	if (i == curr->keysNo) { //The key is bigger than any other key in this node.
		res = currKey->next;
	}
	else if (strcmp(key, currKey->key) == 0) {
		res = currKey->next;
	}
	else { //strcmp(key, currKey->key) < 0
		res = currP;
	}

	top->next = *stack;
	top->parent = curr;
	top->pointer = res;
	*stack = top;
	return res->p.n;

}

struct node * findLeafSimple(char *key) {

	struct node *currNode = B_tree;

	while (!isLeaf(currNode)) {
		currNode = nextNodeSimple(currNode, key);
	}

	return currNode;

}



struct leafStack * findLeafWithStack(char *key) {

	struct leafStack *ret = (struct leafStack *)malloc(sizeof(struct leafStack));
	struct node *currNode = B_tree;
	struct stack *stack = NULL;

	while (!isLeaf(currNode)) {
		currNode = nextNodeWithStack(currNode, key, &stack);
	}

	ret->leaf = currNode;
	ret->stack = stack;
	return ret;

}

void insertInList(char *key, struct node *child, struct pointer *listPos, struct node *parent) {

	struct pointer *newP = (struct pointer *)malloc(sizeof(struct pointer));
	struct key *newK = (struct key *)malloc(sizeof(struct key));
	newP->p.n = child;
	newP->next = listPos->next;
	newK->key = key;
	newK->next = newP;
	listPos->next = newK;
	++(parent->keysNo);

}

char * breakInnerNode(struct node *old, struct node *new) {

	char *middleKey;
	int t = (int)ceil((n+1)/2.0), i = 1, tmp;
	struct pointer *oldLast = old->listStart;

	while (i < t) {
		oldLast = (oldLast->next)->next;
		++i;
	}
	new->listStart = (oldLast->next)->next;
	middleKey = (oldLast->next)->key;
	//free(oldLast->next);
	oldLast->next = NULL;
	new->rightSibling = old->rightSibling;
	new->leftSibling = old;
	old->rightSibling = new;
	tmp = ceil( (old->keysNo - 1) / 2.0 );
	new->keysNo = (old->keysNo - 1) - tmp;
	old->keysNo = tmp;
	new->leaf = 0;

	return middleKey;

}

void insertInParent(struct node *old, struct node *new, char *key, struct stack *stack) {

	if (stack == NULL) { //We got to the root, create a new root
		printf("[insertInParent] creating new root\n\n");
		struct node *newRoot = (struct node *)malloc(sizeof(struct node));
		struct pointer *p1 = (struct pointer *)malloc(sizeof(struct pointer));
		struct key *k1 = (struct key *)malloc(sizeof(struct key));
		struct pointer *p2 = (struct pointer *)malloc(sizeof(struct pointer));
		p1->p.n = old;
		p1->next = k1;
		k1->key = key;
		k1->next = p2;
		p2->p.n = new;
		p2->next = NULL;

		newRoot->listStart = p1;
		newRoot->leaf = 0;
		newRoot->keysNo = 1;
		newRoot->rightSibling = newRoot->leftSibling = NULL;
		B_tree = newRoot; //B_tree is a global variable, pointing to the root of the B-tree
	}
	else {
		/* Stack pop */
		struct stack *top = stack;
		stack = top->next;
		struct node *parent = top->parent;
		struct pointer *listPos = top->pointer;
		//free(top);

		insertInList(key, new, listPos, parent);
		if (isFull(parent)) {
			struct node *newNode = (struct node *)malloc(sizeof(struct node));
			char *middleKey = breakInnerNode(parent, newNode);
			insertInParent(parent, newNode, middleKey, stack);
		}
		/*
		else {
			freeStack(stack);
		}
		*/
	}

}

void insert(char *key, struct record *r) {

	if (B_tree == NULL) {
		createRootLeaf(key, r);
	}
	else {
		struct leafStack *res = findLeafWithStack(key);
		struct stack *stack = res->stack;
		struct node *leaf = res->leaf;
		//free(res);
		insertInLeaf(leaf, key, r);
		if (isFull(leaf)) {
			struct node *newLeaf = (struct node *)malloc(sizeof(struct node));
			char *middleKey = breakLeaf(leaf, newLeaf);
			insertInParent(leaf, newLeaf, middleKey, stack);
		}
	}

}

struct pointer * findFirstKeyInLeaf(struct node *leaf, char *key) {

	struct pointer *currP = leaf->listStart;
	if (currP == NULL) { //This can only happen with an empty root. It is checked before getting here, but check again for safety.
		return NULL;
	}

	while (strcmp((currP->next)->key, key) < 0) {
		currP = (currP->next)->next;
		if (currP == NULL) {
			break;
		}
	}
	if (currP != NULL) {
		if (strcmp((currP->next)->key, key) > 0) {
			currP = NULL;
		}
	}

	return currP;

}

void printRecord(struct record *r) {

	printf("\tTitle:\t\t%s\n", r->title);
	printf("\tYear:\t\t%s\n", r->year);
	printf("\tBest Actor:\t%s\n", r->actor);
	printf("\tDescription:\t%s\n", r->description);

}

void printAllWithSameKey(struct node *leaf, char *key) {

	struct pointer *currP = findFirstKeyInLeaf(leaf, key);
	if (currP == NULL) {
		printf("There is no record with key '%s'.\n", key);
		return;
	}
	printRecord(currP->p.r);

	/*
	short foundBigger = 0;
	while (!foundBigger && leaf != NULL) {
		while (!foundBigger && currP != NULL) {
			if (strcmp((currP->next)->key, key) == 0) {
				printRecord(currP->p.r);
				currP = (currP->next)->next;
			}
			else { //strcmp((currP->next)->key, key) > 0)
				foundBigger = 1;
			}
		}
		if (!foundBigger) {
			leaf = leaf->rightSibling;
			if (leaf != NULL) {
				currP = leaf->listStart;
			}
		}
	}
	*/

}
		       		       

void search(char *key) {

	struct node *leaf = findLeafSimple(key);
	printAllWithSameKey(leaf, key);

}


void deleteFromInnerNodeList(struct pointer *pointOfDeletion, struct node *node) {

	struct key *keyToDelete = pointOfDeletion->next;
	pointOfDeletion->next = (keyToDelete->next)->next;
	free(keyToDelete->key);
	free(keyToDelete->next);
	free(keyToDelete);
	--(node->keysNo);

}

short isShortInnerNode(struct node *node) {

	int threshold = (int)ceil( n / 2.0 );
	int pointersNo = node->keysNo + 1;
	if (pointersNo < threshold) {
		return 1;
	}
	else {
		return 0;
	}

}

short canMergeInnerNodes(struct node *left, struct node *right) {

	if (left == NULL || right == NULL) {
		return 0;
	}
	else if ((left->keysNo+1) + (right->keysNo+1) > n) {
		return 0;
	}
	else {
		return 1;
	}

}

struct pointer * getLastPointerFromInnerNode(struct pointer *start) {

	struct pointer *ret = start;
	while (ret->next != NULL) {
		ret = (ret->next)->next;
	}
	return ret;

}

void mergeInnerNodes(struct node *left, struct node *right, char *key) {

	struct pointer *last = getLastPointerFromInnerNode(left->listStart);
	struct key *newKeyStruct = (struct key *)malloc(sizeof(struct key));
	newKeyStruct->key = key;
	newKeyStruct->next = right->listStart;
	last->next = newKeyStruct;
	left->keysNo += right->keysNo + 1;
	free(right);

}

short canGiveInnerNode(struct node *node) {

	if (node == NULL) {
		return 0;
	}

	int pointersNo = node->keysNo + 1;
	int threshold = (int)ceil( n/2.0 );
	if (pointersNo > threshold) {
		return 1;
	}
	else {
		return 0;
	}

}

void freeDstack(struct Dstack *stack) {

	struct Dstack *top;
	while (stack != NULL) {
		top = stack;
		stack = top->next;
		free(top);
	}

}

char * redistributeInnerNodes(struct node *donor, struct node *receiver, enum redistWith with, char *key) {

	int totalPointers = (donor->keysNo+1) + (receiver->keysNo+1);
	int c = (int)ceil( totalPointers / 2.0 );
	int f = (int)floor( totalPointers / 2.0 );
	int moveRight, i;
	struct pointer *currP = donor->listStart;
	struct pointer *newStart, *lastPointer;
	struct key *currK;
	char *ret;

	if (with == LEFT) {

		struct pointer *newStart;
		moveRight = c;
		i = 1;
		while (i < moveRight) {
			currP = (currP->next)->next;
			++i;
		}

		//Now currP points to the last pointer of the donor.
		currK = currP->next;
		currP->next = NULL;
		//At this point we are done with the donor's list.

		newStart = currK->next;
		currK->next = receiver->listStart;
		receiver->listStart = newStart;
		lastPointer = getLastPointerFromInnerNode(newStart);
		lastPointer->next = currK;

		ret = currK->key;
		currK->key = key;

		receiver->keysNo = f - 1;
		donor->keysNo = c - 1;

	}
	else {

		moveRight = c - (receiver->keysNo + 1); //how many pointers the receiver wants to have minus what it already has
		i = 1;
		while (i < moveRight) {
			currP = (currP->next)->next;
			++i;
		}

		//Now currP points at the last pointer of the receiver
		currK = currP->next;
		newStart = currK->next;
		currK->next = donor->listStart;
		donor->listStart = newStart;

		lastPointer = getLastPointerFromInnerNode(receiver->listStart);
		lastPointer->next = currK;
		ret = currK->key;
		currK->key = key;

		receiver->keysNo = c - 1;
		donor->keysNo = f - 1;

	}

	return ret;

}

void deleteFromInnerNode(struct Dstack *stack) {

	if (stack->previousAction == NONE) {
		printf("\n[deleteFromInnerNode] previous action is NONE, all good\n");
		freeDstack(stack);
		return;
	}
	struct node *me = stack->node;
	if (stack->previousAction == MERGE_LEFT) {
		deleteFromInnerNodeList(stack->leftPointer, me);
	}
	else if (stack->previousAction == MERGE_RIGHT) {
		deleteFromInnerNodeList(stack->centralPointer, me);
	}
	else if (stack->previousAction == REDIST_LEFT) {
		((stack->leftPointer)->next)->key = stack->newKey;
	}
	else if (stack->previousAction == REDIST_RIGHT) {
		((stack->centralPointer)->next)->key = stack->newKey;
	}

	struct Dstack *top = stack;
	stack = top->next;
	free(top);
	if (stack == NULL) { // this means we are the root

		assert(B_tree == me);
		if (me->keysNo == 0) { //root is only left with one pointer, so remove it and make it's only child the new root
			printf("\n[deleteFromInnerNode] the root arrested having only one child\n");
			assert((me->listStart)->next == NULL);
			B_tree = (me->listStart)->p.n;
			free(me);
		}

	}
	else {

		if (isShortInnerNode(me)) {

			struct node *leftSister = stack->leftPointer->p.n;
			struct node *rightSister = stack->rightPointer->p.n;
			char *leftKey = ((stack->leftPointer)->next)->key, *rightKey = ((stack->centralPointer)->next)->key;
			char *mergeKey;
			if (canMergeInnerNodes(leftSister, me)) {
				mergeKey = copyKey(leftKey);//produce a copy of it, because the recursive call is gonna free leftKey
				mergeInnerNodes(leftSister, me, mergeKey);
				stack->previousAction = MERGE_LEFT;
			}
			else if (canMergeInnerNodes(me, rightSister)) {
				mergeKey = copyKey(rightKey);//produce a copy of it, because the recursive call is gonna free rightKey
				mergeInnerNodes(me, rightSister, mergeKey);
				stack->previousAction = MERGE_RIGHT;
			}
			else if (canGiveInnerNode(leftSister)) {
				stack->newKey = redistributeInnerNodes(leftSister, me, LEFT, leftKey);
				stack->previousAction = REDIST_LEFT;
			}
			else if (canGiveInnerNode(rightSister)) {
				stack->newKey = redistributeInnerNodes(rightSister, me, RIGHT, rightKey);
				stack->previousAction = REDIST_RIGHT;
			}

			deleteFromInnerNode(stack);

		}
		else {
			freeDstack(stack);
		}

	}

}

struct node * nextNodeWithDstack(struct node *curr, char *key, struct Dstack **stack) {

	int i = 0;
	struct Dstack *top = (struct Dstack *)malloc(sizeof(struct Dstack));
	struct pointer *prevP = NULL, *currP = curr->listStart;
	struct key *currKey = currP->next;
	struct pointer *left, *central, *right;
	while ((strcmp(key,currKey->key) > 0)) {
		++i;
		if (i == curr->keysNo) {
			break;
		}
		else {
			prevP = currP;
			currP = currKey->next;
			currKey = currP->next;
		}
	}

	if (i == curr->keysNo) { //The key is bigger than any other key in this node.
		left = currP;
		central = currKey->next;
		assert(central->next == NULL);
		right = NULL;
	}
	else if (strcmp(key, currKey->key) == 0) {
		left = currP;
		central = currKey->next;
		//right = (central->next == NULL)?NULL:(central->next)->next;
		if (central->next == NULL) {
			right = NULL;
		}
		else {
			right = (central->next)->next;
		}
	}
	else { //strcmp(key, currKey->key) < 0
		left = prevP;
		central = currP;
		right = (central->next)->next;
	}

	top->next = *stack;
	top->node = curr;
	top->leftPointer = left;
	top->centralPointer = central;
	top->rightPointer = right;
	*stack = top;
	return central->p.n;

}

void findLeafWithDstack(char *key, struct node **leaf, struct Dstack **stack) {

	struct Dstack *Dstack = NULL;
	struct node *currNode = B_tree;
	while (!isLeaf(currNode)) {
		currNode = nextNodeWithDstack(currNode, key, &Dstack);
	}
	*leaf = currNode;
	*stack = Dstack;

}

short canMergeLeafs(struct node *left, struct node *right) {

	if (left == NULL || right == NULL) {
		return 0;
	}
	else if (left->keysNo + right->keysNo <= (n-1)) {
		return 1;
	}
	else return 0;
}

struct key * getLastKeyFromLeaf(struct pointer *p) {

	struct key *curr = p->next;
	while (curr->next != NULL) {
		curr = (curr->next)->next;
	}
	return curr;

}

void mergeLeafs(struct node *left, struct node *right) {

	struct node *shortOne = NULL;
	int threshold = (int)ceil( (n-1) / 2.0 );
	if (left->keysNo < threshold) {
		shortOne = left;
	}
	else if (right->keysNo < threshold) {
		shortOne = right;
	}
	assert(shortOne != NULL); //One has to be short. Otherwise why merge?
	if (shortOne->keysNo == 0) { //Can happen if n=2,3

		if (shortOne == left) {
			left->listStart = right->listStart;
			left->keysNo = right->keysNo;
			free(right);
		}
		return;

	}

	struct key *last = getLastKeyFromLeaf(left->listStart);
	last->next = right->listStart;
	left->keysNo += right->keysNo;
	free(right);

}

short canGiveLeaf(struct node *leaf) {

	if (leaf == NULL) {
		return 0;
	}
	else if ( leaf->keysNo > ceil( (n-1) / 2.0 ) ) {
		return 1;
	}
	else {
		return 0;
	}

}

short isShortLeaf(struct node *leaf) {

	if ( leaf->keysNo < ceil( (n-1) / 2.0 ) ) {
		return 1;
	}
	else {
		return 0;
	}

}

char * redistributeLeafs(struct node *donor, struct node *receiver, enum redistWith with) {

	int totalKeys = donor->keysNo + receiver->keysNo;
	int moveRight, i;
	struct key *keyPointer = (donor->listStart)->next; //points to first key struct of the donor
	struct key *lastKey;
	struct pointer *firstPointer;
	char *ret;

	if (with == LEFT) {

		moveRight = (int)ceil( totalKeys/2.0 );
		i = 1;
		while (i < moveRight) {
			keyPointer = (keyPointer->next)->next;
			++i;
		}

		//Now keyPointer points to the last key struct of the donor
		firstPointer = keyPointer->next;
		keyPointer->next = NULL;
		//At this point we are done with donor's list

		lastKey = getLastKeyFromLeaf(firstPointer);
		lastKey->next = receiver->listStart;
		receiver->listStart = firstPointer;
		ret = copyKey((firstPointer->next)->key);

		receiver->keysNo += donor->keysNo - moveRight;
		donor->keysNo = moveRight;

	}
	else { //with == RIGHT

		moveRight = (int)ceil( totalKeys/2.0 ); //how many keys we want the receiver to have
		moveRight -= receiver->keysNo; //subtrace what it already has to get how many it needs more
		i = 1;
		while (i < moveRight) {
			keyPointer = (keyPointer->next)->next;
			++i;
		}

		//Now keyPointer points to the last key struct of the receiver
		firstPointer = keyPointer->next;
		keyPointer->next = NULL;
		lastKey = getLastKeyFromLeaf(receiver->listStart);
		lastKey->next = donor->listStart;
		donor->listStart = firstPointer;
		ret = copyKey((firstPointer->next)->key);

		receiver->keysNo += moveRight;
		donor->keysNo -= moveRight;

	}

	return ret;

}
		

void freeRecord(struct record *r) {

	free(r->title);
	free(r->year);
	free(r->actor);
	free(r->description);
	free(r);

}

void deleteFromLeaf(char *key, struct node *leaf, struct Dstack *stack) {

	if (leaf->listStart == NULL) { //This is an empty root. Never gonna get here (catched before), but check again for safety.
		assert(B_tree == leaf); //Can't get a NULL listStart, unless we are leaf.
		printf("\nThe tree is empty!\n");
		return;
	}

	struct pointer *currP = leaf->listStart;
	struct key *prevK = NULL, *currK = currP->next;
	while (strcmp(key, currK->key) > 0) {
		prevK = currK;
		currP = currK->next;
		if (currP == NULL) {
			break;
		}
		else {
			currK = currP->next;

		}
	}
	if (currP == NULL || strcmp(currK->key, key) > 0) { //key not found
		printf("No record with key '%s' found.\n", key);
		if (stack != NULL) {
			stack->previousAction = NONE;
		}
		return;
	}
	
	if (prevK == NULL) {
		leaf->listStart = currK->next;
	}
	else {
		prevK->next = currK->next;
	}
	freeRecord(currP->p.r);
	free(currP);
	free(currK->key);
	free(currK);
	--(leaf->keysNo);
	printf("Key '%s' deleted successfully.\n", key);
	
	if ((stack != NULL) && (isShortLeaf(leaf))) { //if stack == NULL, the leaf is also the root

		struct node *leftSister = (stack->leftPointer != NULL) ? (stack->leftPointer->p.n) : NULL;
		struct node *rightSister = (stack->rightPointer != NULL) ? (stack->rightPointer->p.n) : NULL;
		char *parentNewKey;
		if (canMergeLeafs(leftSister, leaf)) {
			mergeLeafs(leftSister, leaf);
			stack->previousAction = MERGE_LEFT;
		}
		else if (canMergeLeafs(leaf, rightSister)) {
			mergeLeafs(leaf, rightSister);
			stack->previousAction = MERGE_RIGHT;
		}
		else if (canGiveLeaf(leftSister)) {
			parentNewKey = redistributeLeafs(leftSister, leaf, LEFT);
			free(((stack->leftPointer)->next)->key);
			stack->newKey = parentNewKey;
			stack->previousAction = REDIST_LEFT;
		}
		else if (canGiveLeaf(rightSister)) {
			parentNewKey = redistributeLeafs(rightSister, leaf, RIGHT);
			free(((stack->centralPointer)->next)->key);
			stack->newKey = parentNewKey;
			stack->previousAction = REDIST_RIGHT;
		}


	}
	else if (stack != NULL) {
		printf("\n[deleteFromLeaf] deleted but leaf is still ok\n");
		stack->previousAction = NONE;
	}
	else if (stack == NULL && leaf->keysNo == 0) { //We are an empty root.
		assert(leaf == B_tree); //if not, stack should not be NULL
		assert(leaf->listStart == NULL); //if we have 0 keys, then listStart must be NULL
		printf("\nThis was the last key to delete. The tree is empty now.\n");
		free(B_tree);
		B_tree = NULL;
	}

}

void delete(char *key) {

	struct Dstack *stack;
	struct node *leaf;
	findLeafWithDstack(key, &leaf, &stack);
	deleteFromLeaf(key, leaf, stack);
	if (stack != NULL) { //if the leaf is not also the root
		deleteFromInnerNode(stack);
	}

}


char * copyInput(char *src) {

	int l = strlen(src);
	src[l-1] = '\0';
	char *dst = (char *)malloc(l);
	strcpy(dst, src);
	return dst;

}

int main(int argc, char **argv) {

	if (argc != 2) {
		printf("Usage: ./b-tree <n>\n");
		exit(1);
	}
	n = atoi(argv[1]);
	char c;
	char *searchKey, *delKey;
	char inputBuffer[200];
	struct record *r;

	printf("\nWelcome to B-tree indexing demo in C!\n"
	       "You can insert movie records and they will be stored.\n"
	       "You can then query the program back for movies you have inserted using the movie title as a key.\n"
	       "You can also delete movie records that you have stored.\n"
	       "After you quit everything will be lost.\n");

	while (1) {
		printf("\ni(nsert)/s(earch)/d(elete)/q(uit): ");

		fgets(inputBuffer, 200, stdin);
		c = inputBuffer[0];
		printf("\n");

		if (c == 'q') {
			break;
		}

		else if (c == 'i') {
			r = (struct record *)malloc(sizeof(struct record));
			printf("Title: ");
			fgets(inputBuffer, 200, stdin);
			r->title = copyInput(inputBuffer);
			printf("Year: ");
			fgets(inputBuffer, 200, stdin);
			r->year = copyInput(inputBuffer);
			printf("Best actor: ");
			fgets(inputBuffer, 200, stdin);
			r->actor = copyInput(inputBuffer);
			printf("Description: ");
			fgets(inputBuffer, 200, stdin);
			r->description = copyInput(inputBuffer);
			printf("\n");

			insert(r->title, r);
			printf("Record with key '%s' inserted successfully.\n", r->title);
		}
		else if (c == 's') {
			if (B_tree == NULL || B_tree->keysNo == 0) {
				printf("There are no records stored. Add some first.\n");
			}
			else {
				printf("Key (movie name): ");
				fgets(inputBuffer, 200, stdin);
				printf("\n");
				searchKey = copyInput(inputBuffer);
				search(searchKey);
				free(searchKey);
			}
		}
		else if (c == 'd') {
			if (B_tree == NULL || B_tree->keysNo == 0) {
				printf("The tree is empty. There is nothing to delete.\n");
			}
			else {
				printf("Key (movie name) to delete: ");
				fgets(inputBuffer, 200, stdin);
				printf("\n");
				delKey = copyInput(inputBuffer);
				delete(delKey);
				free(delKey);
			}
		}
		else {
			printf("Please enter one and only character. i for insertion, s for seach, d for deletion or q to quit.\n");
		}

	}

	printf("Bye\n");
	return 0;

}
