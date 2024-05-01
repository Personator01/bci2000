#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <atomic> 
template <typename T> class CircularBuffer
{
public:
	CircularBuffer(uint32_t size = 1) : _size(size), _curr_pos(0)
	{
		_buffer = new T[_size];
		_items_in_buffer = 0;
	}
	~CircularBuffer() {
		delete[] _buffer;
	}
	void ResizeBuffer(uint32_t size)
	{
		if (size != _size)
		{
			_size = size;
			delete[] _buffer;
			_buffer = new T[_size];
		}
		_curr_pos = 0;
		_items_in_buffer = 0;

	}

	T* CopyToBuffer(T* item, bool ignoreItemCounter = false)
	{
		if (!ignoreItemCounter)
		{
			if (IsFull())
				return NULL;
			++_items_in_buffer;
		}
		T* insertion_pos = _buffer + _curr_pos;
		IncrementBuffer(false);
		memcpy(insertion_pos, item, sizeof(T));


		return insertion_pos;
	}

	T* GetCurrentItem()
	{
		return _buffer + _curr_pos;
	}

	void IncrementBuffer(bool increment_item_cnt = true)
	{
		if (increment_item_cnt)
			++_items_in_buffer;

		_curr_pos = (_curr_pos < (_size - 1)) ? _curr_pos + 1 : 0;

	}

	bool IsEmpty()
	{
		return _items_in_buffer == 0;
	}

	bool IsFull()
	{
		return _items_in_buffer >= _size;
	}

	int32_t ItemsInBuffer()
	{
		return _items_in_buffer;
	}

	T PopItem()
	{
		if (_items_in_buffer == 0)
			return {};
		int32_t currBuffpos = _curr_pos - 1 - (_items_in_buffer--);
		if (currBuffpos < 0) //integer underflow -1
			currBuffpos = _size + currBuffpos;

		T item = *(_buffer + currBuffpos);

		//--_items_in_buffer;
		return item;


	}

	T PeekItem()
	{
		if (_items_in_buffer == 0)
			return {};
		int32_t currBuffpos = _curr_pos - 1 - _items_in_buffer;
		if (currBuffpos < 0) //integer underflow -1
			currBuffpos = _size + currBuffpos;

		//--_items_in_buffer;
		return  *(_buffer + currBuffpos);;


	}

	//compares if the current content of the ring buffer equals rh
	//compares backwards from current position 
	bool equals(T* rh, uint32_t size)
	{


		uint32_t currBuffpos = _curr_pos - 1;
		for (uint32_t i = size - 1; i > 0; i--)
		{
			if (currBuffpos > (_size - 1)) //integer underflow -1
				currBuffpos = _size - 1;

			if (!(rh[i] == _buffer[currBuffpos]))
				return false;
			currBuffpos--;


		}
		return true;
	}


private:
	T* _buffer;
	std::atomic<std::int32_t> _size;
	std::atomic<std::int32_t> _curr_pos;
	std::atomic<std::int32_t> _items_in_buffer;


};


#endif // !