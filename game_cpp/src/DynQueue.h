#pragma once
#include <iostream>
using namespace std;
template <class T>
class DynQueue
{
public:
	struct Node
	{
		T data;
		Node* next;
		Node* prev;
	};
private:

	Node* front;
	Node* rear;
	int numItems;
public:
	DynQueue();
	~DynQueue();
	void copy(DynQueue<T>* list);
	void traverse(Node** curr);
	void jump_to_index(int idx, Node*& pNode);
	void print_list();
	bool updateNode(int idx, Node* pNode);
	bool Search(T query);
	void deleteNode(int idx);
	void deleteNode(Node* pNode);
	bool dequeue_reverse(T&);
	void enqueue(T);
	bool dequeue(T&);
	void clear();
	bool isEmpty() const;
	int getCount();
};


