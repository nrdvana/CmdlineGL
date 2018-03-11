/******************************************************************************\
*   Contained_RBTree.cpp            by: TheSilverDirk / Michael Conrad         *
*   Created: 03/12/2000             Last Modified: 08/29/2002                  *
*   See Contained_RBTree.h for details                                         *
\******************************************************************************/

/*
Credits:

  Intrest in red/black trees was inspired by Dr. John Franco, and his animated
	red/black tree java applet.
  http://www.ececs.uc.edu/~franco/C321/html/RedBlack/redblack.html

  The node insertion code was written in a joint effort with Anthony Deeter.

  The red/black deletion algorithm was derived from the deletion patterns in
   "Fundamentals of Sequential and Parallel Algorithms",
	by Dr. Kenneth A. Berman and Dr. Jerome L. Paul

  I also got the sentinel node idea from this book.

*/
#include <config.h>
#include "Contained_RBTree.h"

//namespace ContainedClass {

RBTreeNode Sentinel= { &Sentinel, &Sentinel, &Sentinel, RBTreeNode_Black, 0 };

bool RBTreeNode_IsSentinel( RBTreeNode *Node ) {
	return Node->Left == Node;
}

void RBTreeNode_Init( RBTreeNode* Node ) {
	Node->Color= RBTreeNode_Unassigned;
}

void RBTree_InitRootSentinel( RBTreeNode *RootSentinel ) {
	RootSentinel->Color= RBTreeNode_Black;
	RootSentinel->Left= &Sentinel;
	RootSentinel->Right= &Sentinel;
	RootSentinel->Parent= 0; // this uniquely marks this as the root sentinel
	RootSentinel->Object= 0;
}

void RBTree_Clear( RBTreeNode *RootSentinel ) {
	RBTreeNode *Temp= RBTree_GetLeftmost(RootSentinel->Left);
	while (Temp != &Sentinel) {
		Temp->Color= RBTreeNode_Unassigned;
		Temp= RBTree_GetNext(Temp);
	}
	RootSentinel->Left= &Sentinel;
}

bool RBTree_Add( RBTreeNode *RootSentinel, RBTreeNode* NewNode, RBTree_inorder_func* inorder ) {
	RBTreeNode* Current;
	if (NewNode->Color != RBTreeNode_Unassigned)
		return false;

	NewNode->Color= RBTreeNode_Red;
	NewNode->Left=  &Sentinel;
	NewNode->Right= &Sentinel;

	Current= RootSentinel->Left;
	if (Current == &Sentinel) {
		RootSentinel->Left= NewNode;
		NewNode->Parent= RootSentinel;
	}
	else {
		do {
			// if the new node comes before the current node, go left
			if (inorder( NewNode->Object, Current->Object )) {
				if (Current->Left == &Sentinel) {
					Current->Left= NewNode;
					NewNode->Parent= Current;
					break;
				}
				else Current= Current->Left;
			}
			// else go right
			else {
				if (Current->Right == &Sentinel) {
					Current->Right= NewNode;
					NewNode->Parent= Current;
					break;
				}
				else Current= Current->Right;
			}
		} while (1);
		RBTree_Balance( Current );
	}
	RootSentinel->Left->Color= RBTreeNode_Black;
	return true;
}

RBTreeNode* RBTree_Find( const RBTreeNode *RootSentinel, const void* SearchKey, RBTree_compare_func* compare) {
	RBTreeNode* Current= RootSentinel->Left;
	while (Current != &Sentinel) {
		int i= compare( SearchKey, Current->Object );
		if      (i<0) Current= Current->Left;
		else if (i>0) Current= Current->Right;
		else return Current;
	}
	return &Sentinel;
}

RBTreeNode* RBTree_GetLeftmost( RBTreeNode* Node ) {
	while (Node->Left != &Sentinel)
		Node= Node->Left;
	return Node;
}

RBTreeNode* RBTree_GetRightmost( RBTreeNode* Node ) {
	while (Node->Right != &Sentinel)
		Node= Node->Right;
	return Node;
}

RBTreeNode* RBTree_GetPrev( RBTreeNode* Node ) {
	// If we are not at a leaf, move to the right-most node
	//  in the tree to the left of this node.
	if (Node->Left != &Sentinel) {
		RBTreeNode* Current= Node->Left;
		while (Current->Right != &Sentinel)
			Current= Current->Right;
		return Current;
	}
	// Else walk up the tree until we see a parent node to the left
	else {
		RBTreeNode* Parent= Node->Parent;
		while (Parent->Left == Node) {
			Node= Parent;
			Parent= Parent->Parent;
			// Check for RootSentinel
			if (!Parent) return &Sentinel;
		}
		return Parent;
	}
}

RBTreeNode* RBTree_GetNext( RBTreeNode* Node ) {
	// If we are not at a leaf, move to the left-most node
	//  in the tree to the right of this node.
	if (Node->Right != &Sentinel) {
		RBTreeNode* Current= Node->Right;
		while (Current->Left != &Sentinel)
			Current= Current->Left;
		return Current;
	}
	// Else walk up the tree until we see a parent node to the right
	else {
		RBTreeNode* Parent= Node->Parent;
		while (Parent->Right == Node) {
			Node= Parent;
			Parent= Node->Parent;
		}
		// Check for the RootSentinel
		if (!Parent->Parent) return &Sentinel;

		return Parent;
	}
}

void RBTree_RightSide_RightRotate( RBTreeNode* Node ) {
	RBTreeNode* temp= Node->Parent; // temp is used for parent
	RBTreeNode* child= Node->Left;

	temp->Right= child;
	child->Parent= temp;

	temp= child->Right; // temp is now used for grandchild
	Node->Left= temp;
	/*if (temp != Sentinel)*/
	temp->Parent= Node;

	child->Right= Node;
	Node->Parent= child;
}

void RBTree_LeftSide_LeftRotate( RBTreeNode* Node ) {
	RBTreeNode* temp= Node->Parent; // temp is used for parent
	RBTreeNode* child= Node->Right;

	temp->Left= child;
	child->Parent= temp;

	temp= child->Left; // temp is now used for grandchild
	Node->Right= temp;
	/*if (temp != Sentinel)*/
	temp->Parent= Node;

	child->Left= Node;
	Node->Parent= child;
}

void RBTree_LeftSide_RightRotate( RBTreeNode* Node ) {
	RBTreeNode* temp= Node->Parent; // temp is used for parent
	RBTreeNode* child= Node->Left;

	temp->Left= child;
	child->Parent= temp;

	temp= child->Right; // temp is now used for grandchild
	Node->Left= temp;
	/*if (temp != Sentinel)*/
	temp->Parent= Node;

	child->Right= Node;
	Node->Parent= child;
}

void RBTree_RightSide_LeftRotate( RBTreeNode* Node ) {
	RBTreeNode* temp= Node->Parent; // temp is used for parent
	RBTreeNode* child= Node->Right;

	temp->Right= child;
	child->Parent= temp;

	temp= child->Left; // temp is now used for grandchild
	Node->Right= temp;
	/*if (temp != Sentinel)*/
	temp->Parent= Node;

	child->Left= Node;
	Node->Parent= child;
}

// current is the parent node of the node just added.  The child is red.
void RBTree_Balance( RBTreeNode* Current ) {
	// if Current is a black node, no rotations needed
	while (Current->Color != RBTreeNode_Black) {
//		if (!Current->Parent) break;  XXX may not need this
		// Current is red, the imbalanced child is red, and parent is black.

		RBTreeNode *Parent= Current->Parent;

		// if the Current is on the right of the parent, the parent is to the left
		if (Parent->Right == Current) {
			// if the sibling is also red, we can pull down the color black from the parent
			if (Parent->Left->Color == RBTreeNode_Red) {
				Parent->Left->Color= RBTreeNode_Black;
				Current->Color= RBTreeNode_Black;
				Parent->Color= RBTreeNode_Red;
				// jump twice up the tree. if Current reaches the HeadSentinel (black node), the loop will stop
				Current= Parent->Parent;
				continue;
			}
			// if the imbalance (red node) is on the left, and the parent is on the left,
			//  a "prep-slide" is needed. (see diagram)
			if (Current->Left->Color == RBTreeNode_Red)
				RBTree_RightSide_RightRotate( Current );

			// Now we can do our left rotation to balance the tree.
			if (Parent->Parent->Left == Parent)
				RBTree_LeftSide_LeftRotate( Parent );
			else
				RBTree_RightSide_LeftRotate( Parent );
			Parent->Color= RBTreeNode_Red;
			Parent->Parent->Color= RBTreeNode_Black;
			return;
		}
		// else the parent is to the right
		else {
			// if the sibling is also red, we can pull down the color black from the parent
			if (Parent->Right->Color == RBTreeNode_Red) {
				Parent->Right->Color= RBTreeNode_Black;
				Current->Color= RBTreeNode_Black;
				Parent->Color= RBTreeNode_Red;
				// jump twice up the tree. if Current reaches the HeadSentinel (black node), the loop will stop
				Current= Parent->Parent;
				continue;
			}
			// if the imbalance (red node) is on the right, and the parent is on the right,
			//  a "prep-slide" is needed. (see diagram)
			if (Current->Right->Color == RBTreeNode_Red)
				RBTree_LeftSide_LeftRotate( Current );

			// Now we can do our left rotation to balance the tree.
			if (Parent->Parent->Left == Parent)
				RBTree_LeftSide_RightRotate( Parent );
			else
				RBTree_RightSide_RightRotate( Parent );
			Parent->Color= RBTreeNode_Red;
			Parent->Parent->Color= RBTreeNode_Black;
			return;
		}
	}
	return;
}

bool RBTree_Prune( RBTreeNode* Current ) {
	RBTreeNode* Temp;
	if (Current->Color == RBTreeNode_Unassigned)
		return false;

	// If this is a leaf node (or almost a leaf) we can just prune it
	if (Current->Left == &Sentinel || Current->Right == &Sentinel)
		RBTree_PruneLeaf(Current);

	// Otherwise we need a successor.  We are guaranteed to have one because
	//  the current node has 2 children.
	else {
		RBTreeNode* Successor= RBTree_GetNext( Current );
		// Do we like this successor?  If not, get the other one.
		if (Successor->Color == RBTreeNode_Black && Successor->Left == &Sentinel && Successor->Right == &Sentinel)
			Successor= RBTree_GetPrev( Current );

		RBTree_PruneLeaf( Successor );

		// now exchange the successor for the current node
		Temp= Current->Right;
		Successor->Right= Temp;
		Temp->Parent= Successor;

		Temp= Current->Left;
		Successor->Left= Temp;
		Temp->Parent= Successor;

		Temp= Current->Parent;
		Successor->Parent= Temp;
		if (Temp->Left == Current) Temp->Left= Successor; else Temp->Right= Successor;
		Successor->Color= Current->Color;
	}
	Current->Color= RBTreeNode_Unassigned;
	return true;
}

// PruneLeaf performs pruning of nodes with at most one child node.
void RBTree_PruneLeaf( RBTreeNode* Node ) {
	RBTreeNode *Parent= Node->Parent, *Current, *Sibling;
	bool LeftSide= (Parent->Left == Node);

	// if the node is red and has at most one child, then it has no child.
	// Prune it.
	if (Node->Color == RBTreeNode_Red) {
		if (LeftSide) Parent->Left= &Sentinel;
		else Parent->Right= &Sentinel;
		return;
	}

	// Node is black here.  If it has a child, the child will be red.
	if (Node->Left != &Sentinel) {
		// swap with child
		Node->Left->Color= RBTreeNode_Black;
		Node->Left->Parent= Parent;
		if (LeftSide) Parent->Left= Node->Left;
		else Parent->Right= Node->Left;
		return;
	}
	if (Node->Right != &Sentinel) {
		// swap with child
		Node->Right->Color= RBTreeNode_Black;
		Node->Right->Parent= Parent;
		if (LeftSide) Parent->Left= Node->Right;
		else Parent->Right= Node->Right;
		return;
	}

/*	Now, we have determined that Node is a black leaf node with no children.
	The tree must have the same number of black nodes along any path from root
	to leaf.  We want to remove a black node, disrupting the number of black
	nodes along the path from the root to the current leaf.  To correct this,
	we must either shorten all other paths, or add a black node to the current
	path.  Then we can freely remove our black leaf.

	While we are pointing to it, we will go ahead and delete the leaf and
	replace it with the sentinel (which is also black, so it won't affect
	the algorithm).
*/
	if (LeftSide) Parent->Left= &Sentinel; else Parent->Right= &Sentinel;

	Sibling= (LeftSide)? Parent->Right : Parent->Left;
	Current= Node;

	// Loop until the current node is red, or until we get to the root node.
	// (The root node's parent is the RootSentinel, which will have a NULL parent.)
	while (Current->Color == RBTreeNode_Black && Parent->Parent != 0) {
		// If the sibling is red, we are unable to reduce the number of black
		//  nodes in the sibling tree, and we can't increase the number of black
		//  nodes in our tree..  Thus we must do a rotation from the sibling
		//  tree to our tree to give us some extra (red) nodes to play with.
		// This is Case 1 from the text
		if (Sibling->Color == RBTreeNode_Red) {
			Parent->Color= RBTreeNode_Red;
			Sibling->Color= RBTreeNode_Black;
			if (LeftSide) {
				if (Parent->Parent->Left == Parent)
					RBTree_LeftSide_LeftRotate( Parent );
				else
					RBTree_RightSide_LeftRotate( Parent );
				Sibling= Parent->Right;
			}
			else {
				if (Parent->Parent->Left == Parent)
					RBTree_LeftSide_RightRotate( Parent );
				else
					RBTree_RightSide_RightRotate( Parent );
				Sibling= Parent->Left;
			}
			continue;
		}
		// Sibling will be black here

		// If the sibling is black and both children are black, we have to
		//  reduce the black node count in the sibling's tree to match ours.
		// This is Case 2a from the text.
		if (Sibling->Right->Color == RBTreeNode_Black && Sibling->Left->Color == RBTreeNode_Black) {
			Sibling->Color= RBTreeNode_Red;
			// Now we move one level up the tree to continue fixing the
			// other branches.
			Current= Parent;
			Parent= Current->Parent;
			LeftSide= (Parent->Left == Current);
			Sibling= (LeftSide)? Parent->Right : Parent->Left;
			continue;
		}
		// Sibling will be black with 1 or 2 red children here

		// << Case 2b is handled by the while loop. >>

		// If one of the sibling's children are red, we again can't make the
		//  sibling red to balance the tree at the parent, so we have to do a
		//  rotation.  If the "near" nephew is red and the "far" nephew is
		//  black, we need to do a "prep-slide" (aka "double rotation")
		// After doing a rotation and rearranging a few colors, the effect is
		//  that we maintain the same number of black nodes per path on the far
		//  side of the parent, and we gain a black node on the current side,
		//  so we are done.
		// This is Case 4 from the text. (Case 3 is the double rotation)
		if (LeftSide) {
			if (Sibling->Right->Color == RBTreeNode_Black) { // Case 3 from the text
				RBTree_RightSide_RightRotate( Sibling );
				Sibling= Parent->Right;
			}
			// now Case 4 from the text
			Sibling->Right->Color= RBTreeNode_Black;
			Sibling->Color= Parent->Color;
			Parent->Color= RBTreeNode_Black;

			Current= Parent;
			Parent= Current->Parent;
			// I would have assigned to LeftSide here,
			//   but we are exiting the function, so why bother?
			// LeftSide= (Parent->Left == Current);
			if /*(LeftSide)*/(Parent->Left == Current)
				RBTree_LeftSide_LeftRotate( Current );
			else
				RBTree_RightSide_LeftRotate( Current );
			return;
		}
		else {
			if (Sibling->Left->Color == RBTreeNode_Black) { // Case 3 from the text
				RBTree_LeftSide_LeftRotate( Sibling );
				Sibling= Parent->Left;
			}
			// now Case 4 from the text
			Sibling->Left->Color= RBTreeNode_Black;
			Sibling->Color= Parent->Color;
			Parent->Color= RBTreeNode_Black;

			Current= Parent;
			Parent= Current->Parent;
			// I would have assigned to LeftSide here,
			//   but we are exiting the function, so why bother?
			// LeftSide= (Parent->Left == Current);
			if /*(LeftSide)*/(Parent->Left == Current)
				RBTree_LeftSide_RightRotate( Current );
			else
				RBTree_RightSide_RightRotate( Current );
			return;
		}
	}

	// Now, make the current node black (to fulfill Case 2b)
	// Case 4 will have exited directly out of the function.
	// If we stopped because we reached the top of the tree,
	//   the head is black anyway so don't worry about it.
	Current->Color= RBTreeNode_Black;
}

//}
