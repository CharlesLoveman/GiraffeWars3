#ifndef _ARRAYQUEUE_H_
#define _ARRAYQUEUE_H_

constexpr int ARRAYQUEUELENGTH = 12;

class ArrayQueue {
public:
	ArrayQueue();
	void Push(int x);
	int Peek();
	bool Contains(int x);
	void Clear();
private:
	int data[ARRAYQUEUELENGTH];
	int head;
};

#endif // !_ARRAYQUEUE_H_
