#include "DynQueue.h"
template <class T>
DynQueue<T>::DynQueue()
{
	front = rear = nullptr;
	numItems = 0;
}
template <class T>
DynQueue<T>::~DynQueue()
{
	clear();
}
template <class T>
void DynQueue<T>::copy(DynQueue<T>* list)
{
	int max = list->getCount();
	for (int i = 0; i < max; i++)
	{
		DynQueue<T>::Node* curr;
		list->jump_to_index(i, curr);
		T temp = curr->data;
		enqueue(temp);
	}
}
template <class T>
void DynQueue<T>::enqueue(T data)
{
	Node* n = new Node;
	n->data = data;
	n->next = nullptr;
	n->prev = nullptr;
	if (isEmpty())
	{
		front = rear = n;
		numItems++;
	}
	else
	{
		rear->next = n;
		n->prev = rear;
		rear = n;
		numItems++;
	}
}
template <class T>
bool DynQueue<T>::dequeue(T& data)
{
	if (isEmpty())
	{
		return false; //There are no elements to dequeue
	}
	else
	{
		if (front == rear && numItems == 1)
		{
			rear = nullptr;
		}
		data = front->data; //why was front nullptr?
		Node* temp = front;
		front = front->next;
		delete temp;
		numItems--;
		return true;
	}
}
template <class T>
bool DynQueue<T>::dequeue_reverse(T& data)
{
	if (isEmpty())
	{
		return false;
	}
	else
	{
		if (front == rear && numItems == 1)
		{
			front = nullptr;
		}
		data = rear->data;
		Node* temp = rear;
		rear = rear->prev;
		if (rear)
		{
			rear->next = nullptr;
		}
		temp->prev = nullptr;
		delete temp;
		numItems--;
		return true;
	}
}
template <class T>
bool DynQueue<T>::updateNode(int idx, Node* pNode)
{
	Node* curr = front;
	for (int i = 0; i < idx; i++)
	{
		curr = curr->next;
	}
	curr->data = pNode->data;
	return true;
}
template <class T>
bool DynQueue<T>::Search(T query)
{
	Node* curr = front;
	while (curr)
	{
		if (curr->data == query)
		{
			return true;
		}
		else
		{
			traverse(&curr);
		}
	}
	return false;
}
template <class T>
void DynQueue<T>::traverse(Node** curr)
{
	if (*curr == nullptr)
	{
		*curr = front;
	}
	else
	{
		*curr = (*curr)->next;
	}
}
template <class T>
void DynQueue<T>::jump_to_index(int idx, Node*& pNode)
{
	Node* curr = nullptr;
	Node** ppcurr = &curr;
	traverse(ppcurr);
	for (int i = 0; i < idx; i++)
	{
		traverse(&curr);
	}
	if (curr != nullptr)
	{
		pNode = curr;
	}
	else
	{
		pNode = nullptr;
	}
}
template <class T>
void DynQueue<T>::deleteNode(int idx)
{
	Node* pCurr = new Node;
	jump_to_index(idx, pCurr);
	if (pCurr)
	{

		Node* temp = pCurr->prev;
		Node* dptr = pCurr;
		pCurr = pCurr->next;
		if (temp)
		{
			temp->next = pCurr;
		}
		if (pCurr)
		{
			pCurr->prev = temp;
		}
		if (dptr == front)
		{
			front = front->next;
		}
		if (dptr == rear)
		{
			rear = rear->prev;
		}
		numItems--;
		delete dptr;

	}

}
template <class T>
void DynQueue<T>::deleteNode(Node* pNode)
{
	Node* curr = front;
	while (curr)
	{
		if (curr == pNode)
		{
			Node* temp = curr->prev;
			Node* dptr = curr;
			curr = curr->next;
			if (temp)
			{
				temp->next = curr;
			}
			if (curr)
			{
				curr->prev = temp;
			}
			if (dptr == front)
			{
				front = front->next;
			}
			if (dptr == rear)
			{
				rear = rear->prev;
			}
			numItems--;
			delete dptr;
			return;
		}
		else
		{
			traverse(&curr);
		}
	}
}
template <class T>
void DynQueue<T>::print_list()
{
	Node* curr = front;
	while (curr != nullptr)
	{
		cout << curr->data;
		curr = curr->next;
	}
}
template <class T>
void DynQueue<T>::clear()
{
	T data;
	while (dequeue(data))
	{
	}
}
template <class T>
bool DynQueue<T>::isEmpty() const
{
	return numItems == 0;
}
template <class T>
int DynQueue<T>::getCount()
{
	return numItems;
}
