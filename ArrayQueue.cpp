#include "ArrayQueue.h"

ArrayQueue::ArrayQueue()
{
	head = 0;
	//for (int i = 0; i < ARRAYQUEUELENGTH; ++i) {
	//	data[i] = 0;
	//}
}

void ArrayQueue::Push(int x)
{
	head = (head + 1) % ARRAYQUEUELENGTH;
	data[head] = x;
}

int ArrayQueue::Peek()
{
	return data[head];
}

bool ArrayQueue::Contains(int x)
{
	for (int i = 0; i < ARRAYQUEUELENGTH; ++i) {
		if (data[i] == x) {
			return true;
		}
	}
	return false;
}

void ArrayQueue::Clear()
{
	for (int i = 0; i < ARRAYQUEUELENGTH; ++i) {
		data[i] = 0;
	}
	head = 0;
}
