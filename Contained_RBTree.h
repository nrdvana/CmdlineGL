/******************************************************************************\
*   Contained_RBTree.h              by: TheSilverDirk / Michael Conrad
*   Created: 06/23/2000             Last Modified: 04/29/2005
*
*   2005-04-29: Hacked-up sufficiently to be compilable under C.
*
*   This is a red/black binary search tree implementation using the
*   "contained class" system.  The "inorder" and "compare" function pointers
*   allow you to do custom sorting.  These pointers MUST point to valid
*   functions before you use the tree.
\******************************************************************************/

#include "Global.h"

#ifndef CONTAINED_RBTREE_H
#define CONTAINED_RBTREE_H

//namespace ContainedClass {

/******************************************************************************\
*   Contained RBTree Data Structure
\******************************************************************************/

#define RBTreeNode_Black 0
#define RBTreeNode_Red 1
#define RBTreeNode_Unassigned 2
typedef struct RBTreeNode_t {
//	enum NodeColor { Black= 0, Red= 1, Unassigned= 2 };

	struct RBTreeNode_t* Left;
	struct RBTreeNode_t* Right;
	struct RBTreeNode_t* Parent;
	int         Color;
	void*       Object;
} RBTreeNode;

/******************************************************************************\
*   Base RBTree functions - all functions required to manipulate a R/B tree.
*
*   These functions are all ordinary functions in order to be compatible with
*     other languages, such as C
\******************************************************************************/
typedef bool RBTree_inorder_func( const void* Obj_A, const void* Obj_B );
typedef int  RBTree_compare_func( const void* SearchKey, const void* Object );

void RBTreeNode_Init( RBTreeNode* Node );
bool RBTreeNode_IsSentinel( RBTreeNode *Node );
void RBTree_InitRootSentinel( RBTreeNode *RootSentinel );
void RBTree_Clear( RBTreeNode *RootSentinel );
RBTreeNode* RBTree_GetPrev( RBTreeNode* Node );
RBTreeNode* RBTree_GetNext( RBTreeNode* Node );
RBTreeNode* RBTree_GetRightmost( RBTreeNode* Node );
RBTreeNode* RBTree_GetLeftmost( RBTreeNode* Node );
//inline const RBTreeNode* RBTree_GetLeftmost ( const RBTreeNode* Node ) { return RBTree_GetLeftmost (const_cast<RBTreeNode*>(Node)); }
//inline const RBTreeNode* RBTree_GetRightmost( const RBTreeNode* Node ) { return RBTree_GetRightmost(const_cast<RBTreeNode*>(Node)); }
//inline const RBTreeNode* RBTree_GetPrev     ( const RBTreeNode* Node ) { return RBTree_GetPrev     (const_cast<RBTreeNode*>(Node)); }
//inline const RBTreeNode* RBTree_GetNext     ( const RBTreeNode* Node ) { return RBTree_GetNext     (const_cast<RBTreeNode*>(Node)); }
void RBTree_LeftSide_LeftRotate( RBTreeNode* Node );
void RBTree_LeftSide_RightRotate( RBTreeNode* Node );
void RBTree_RightSide_RightRotate( RBTreeNode* Node );
void RBTree_RightSide_LeftRotate( RBTreeNode* Node );
bool RBTree_Add( RBTreeNode *RootSentinel, RBTreeNode* NewNode, RBTree_inorder_func* inorder );
RBTreeNode* RBTree_Find( const RBTreeNode *RootSentinel, const void* SearchKey, RBTree_compare_func* compare );
void RBTree_Balance( RBTreeNode* Node );
bool RBTree_Prune( RBTreeNode* Node );
void RBTree_PruneLeaf( RBTreeNode* Node );

extern RBTreeNode Sentinel;

/******************************************************************************\
*   Contained RBTree Class                                                     *
\******************************************************************************/
/*
class ECannotAddNode {};
class ECannotRemoveNode {};

class RBTree {
public:
	class Node: public RBTreeNode {
	public:
		Node()  { RBTreeNode_Init(this); }
		~Node() { if (RBTreeNode::Color != RBTreeNode::Unassigned) RBTree_Prune(this); }

		// The average user shouldn't need these, but they might come in handy.
		void* Left()   const { return RBTreeNode::Left->Object;   }
		void* Right()  const { return RBTreeNode::Right->Object;  }
		void* Parent() const { return RBTreeNode::Parent->Object; }
		int   Color()        { return RBTreeNode::Color; }

		// These let you use your nodes as a sequence.
		void* Next()   const { return RBTree_GetNext(this)->Object; }
		void* Prev()   const { return RBTree_GetPrev(this)->Object; }

		bool IsSentinel() { return RBTreeNode_IsSentinel(this); }

		friend RBTree;
	};

private:
	RBTreeNode RootSentinel; // the left child of the sentinel is the root node
public:
	RBTree_inorder_func *inorder;
	RBTree_compare_func *compare;

	RBTree() { RBTree_InitRootSentinel(RootSentinel); }
	~RBTree() { Clear(); }

	void Clear() { RBTree_Clear(RootSentinel); }
	bool IsEmpty() const { return RBTreeNode_IsSentinel(RootSentinel.Left); }

	void* GetRoot()  const { return RootSentinel.Left->Object; }
	void* GetFirst() const { return RBTree_GetLeftmost(RootSentinel.Left)->Object; }
	void* GetLast()  const { return RBTree_GetRightmost(RootSentinel.Left)->Object; }

	void Add( Node* NewNode ) { if (!RBTree_Add(RootSentinel, NewNode, inorder)) throw ECannotAddNode(); }

	void* Find( const void *SearchKey ) const { return RBTree_Find(RootSentinel, SearchKey, compare)->Object; }

	static void Remove( Node* Node ) { if (!RBTree_Prune(Node)) throw ECannotRemoveNode(); }
};
*/

typedef struct RBTree_t {
	RBTreeNode RootSentinel; // the left child of the sentinel is the root node
	RBTree_inorder_func *inorder;
	RBTree_compare_func *compare;
} RBTree;

/******************************************************************************\
*   Type-Safe Contained Red/Black Tree Class                                   *
\******************************************************************************/
/*
template <class T>
class TypedRBTree: public RBTree {
public:
	typedef RBTree Inherited;

	class Node: public RBTree::Node {
	public:
		typedef RBTree::Node Inherited;

		// The average user shouldn't need these, but they might come in handy.
		T* Left()   const { return (T*) Inherited::Left();   }
		T* Right()  const { return (T*) Inherited::Right();  }
		T* Parent() const { return (T*) Inherited::Parent(); }
		int Color() const { return Inherited::Color(); }

		// These let you use your nodes as a sequence.
		T* Next()   const { return (T*) Inherited::Next(); }
		T* Prev()   const { return (T*) Inherited::Prev(); }

		friend TypedRBTree<T>;
	};

public:
	T* GetRoot()  const { return (T*) Inherited::GetRoot();  }
	T* GetFirst() const { return (T*) Inherited::GetFirst(); }
	T* GetLast()  const { return (T*) Inherited::GetLast();  }

	void Add( Node* NewNode ) { Inherited::Add(NewNode); }

	T* Find( const void *Val ) const { return (T*) Inherited::Find(Val); }

	static void Remove( Node* Node ) { Inherited::Remove(Node); }
};

}
*/
#endif

