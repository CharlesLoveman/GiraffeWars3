#ifndef _ARRAYLIST_H_
#define _ARRAYLIST_H_

constexpr int ARRAYLISTLENGTH = 12;

template <class T>
class ArrayList {
public:
	ArrayList()
	{
		head = -1;
	}
	void Append(T x)
	{
		++head;
		if (head == ARRAYLISTLENGTH) {
			--head;
			for (int i = 0; i < head; ++i) {
				data[i] = data[i + 1];
			}
		}
		data[head] = x;
	}
	T& operator[](int index)
	{
		return data[index];
	}
	T Pop()
	{
		T temp = data[head];
		--head;
		return temp;
	}
	void Remove(int index)
	{
		if (index != head) {
			for (int i = index; i <= head; ++i) {
				data[i] = data[i + 1];
			}
		}
		--head;
	}
	int Size() {
		return head + 1;
	}
private:
	T data[ARRAYLISTLENGTH];
	int head;
};

#endif // !_ARRAYQUEUE_H_#pragma once
