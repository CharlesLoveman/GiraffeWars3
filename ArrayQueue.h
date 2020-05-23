#ifndef _ARRAYQUEUE_H_
#define _ARRAYQUEUE_H_

constexpr int ARRAYQUEUELENGTH = 12;

template <class T>
class ArrayQueue {
public:
	ArrayQueue()
	{
		head = 0;
	}
	void Push(T x)
	{
		head = (head + 1) % ARRAYQUEUELENGTH;
		data[head] = x;
	}
	T Peek()
	{
		return data[head];
	}
	bool Contains(T x)
	{
		for (int i = 0; i < ARRAYQUEUELENGTH; ++i) {
			if (data[i] == x) {
				return true;
			}
		}
		return false;
	}
	void Fill(T x)
	{
		for (int i = 0; i < ARRAYQUEUELENGTH; ++i) {
			data[i] = x;
		}
		head = 0;
	}
private:
	T data[ARRAYQUEUELENGTH];
	int head;
};

#endif // !_ARRAYQUEUE_H_
