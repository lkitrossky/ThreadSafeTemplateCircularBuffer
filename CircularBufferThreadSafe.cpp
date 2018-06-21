// CircularBufferThreadSafe.cpp : Defines the entry point for the console application.
/*

The main idea of circle buffer is known.
How to make it thread safe?
We suppose that there are two threads: to put and to read
The idea is not to keep some variables that can be affected by both threads, like zie
Keep only pointer to he beginning of the data and to the enb of the data.
The start of the data is affected by Get/Read operations, while the pointer to the end is affected by Put operation

Another small difficulty is how to know what is the state of buffer when both pointers point to the same plac, 
is it empty or full?
To avoid the problem we take internally capacity one more than necessary and use it to avoid equality of pointers.

*/
#include "stdafx.h"  // for VS

#include <iostream>
using namespace std;

//special class for effective FILO: Circular Buffer
template<class A>
class CCBuffer {
public:
	CCBuffer(unsigned int capacity) :
		p_start_buffer(new A[capacity+1]), 
		p_after_buffer_end(p_start_buffer + capacity + 1){
		p_start_data = (A*)p_start_buffer;
		p_last_data = p_start_data;
	}
	~CCBuffer() { delete p_start_buffer; }
	unsigned int Capacity() const { 
		if (p_after_buffer_end >= p_start_buffer + 1)
			return p_after_buffer_end - p_start_buffer - 1;
		else
			return 0;
	}  
	unsigned int Contains() const { 
		if(p_last_data >= p_start_data)
			return p_last_data - p_start_data; 
		else
			return p_after_buffer_end - p_start_buffer  + p_last_data - p_start_data;
	}
	unsigned int Left()const { return Capacity() - Contains(); }
	bool Put(const A & a) {
		if (0 == Left())
			return false;
		*p_last_data = a;
		p_last_data++;
		if (p_last_data == p_after_buffer_end)
			p_last_data = (A*)p_start_buffer;
		return true;
	}
	bool GetPop(A & out_A) { //we suppose that Get also remove it from the container like Pop
		if (0 == Contains())
			return false;
		out_A = *p_start_data;
		p_start_data++;
		if (p_start_data == p_after_buffer_end)
			p_start_data = (A*)p_start_buffer;
		return true;
	}
	bool Put(unsigned int howMany, const A & a) {
		if (howMany > Left())
			return false;
		if (0 == howMany)
			return true;
		for (unsigned int i = 0; i < howMany; i++)
			Put(a);
		return true;
	}
	friend ostream & operator<<(ostream & os, const CCBuffer & cb) {
		cout << "Capacity: " << cb.Capacity() << " Contains: " << cb.Contains() << " Left: " << cb.Left() << endl;
		for (unsigned int i = 0; i < cb.Contains(); i++) {
			if (&cb.p_start_data[i] >= cb.p_after_buffer_end) {
				int shift = &cb.p_start_data[i] - cb.p_after_buffer_end;
				cout << "#" << i << "\t " << cb.p_start_buffer[shift] << endl;
			}
			else
				cout << "#" << i << "\t " << cb.p_start_data[i] << endl;
		}
		cout << endl;
		return os;
	}
private:
	const A *p_start_buffer;
	const A *p_after_buffer_end;
	A *p_start_data;
	A *p_last_data;
};

template<class A = int> using CCIntBuffer = CCBuffer<int>;


/*
Testing the class
First just how it works
Then thread safety: thread to put waits the thread to read 
*/
void testSimpleCases();
void testThreadCases();

int main() {
	testSimpleCases();
	testThreadCases();
	return 0;
}

#include <chrono>
#include <thread>

CCIntBuffer<> threadSafeBuffer(10);

void threadPutter()
{
	std::cout << "Starting concurrent thread Putter.\n";
	int counter = 0;
	auto start = chrono::high_resolution_clock::now();
	while (false == threadSafeBuffer.Put(22)) {
		this_thread::sleep_for(chrono::seconds(1));
		cout << "Waiting to put, sec " << ++counter << endl;
	}
	auto end = std::chrono::high_resolution_clock::now();
	chrono::duration<double, milli> elapsed = end - start;
	cout << "Waited " << elapsed.count() / 1000 << " s to put 22\n";
	cout << "Exiting concurrent thread.\n";
	cout << threadSafeBuffer;
}
void threadGetter()
{
	std::cout << "Starting concurrent thread Getter.\n";
	int counter = 0;
	auto start = chrono::high_resolution_clock::now();
	this_thread::sleep_for(chrono::seconds(10));
	int value;
	bool res = threadSafeBuffer.GetPop(value);
	auto end = std::chrono::high_resolution_clock::now();
	chrono::duration<double, milli> elapsed = end - start;
	if(res)
		cout << "Waited to get" << elapsed.count() / 1000 << " s to get value: " << value << endl;
	cout << "Exiting concurrent thread Getter.\n";
	cout << threadSafeBuffer;
}
void threadCaller()
{
	std::cout << "Starting thread Putter caller.\n";
	std::thread t1(threadPutter);
	t1.detach();
	std::thread t2(threadGetter);
	t2.detach();
	std::this_thread::sleep_for(std::chrono::seconds(15));
	std::cout << "Exiting thread caller.\n";
}



void testThreadCases() {
	
	cout << "Thread test cases" << endl;
	cout << "Fill buffer first, then run concurrently put and get" << endl;
	threadSafeBuffer.Put(threadSafeBuffer.Left(), 11);
	cout << threadSafeBuffer;
	threadCaller();
}
void testSimpleCases()
{
	cout << "test case empty from the beginning" << endl;
	CCBuffer<int> cbuffer0(0);
	bool fPush = cbuffer0.Put(1);
	cout << "Test case push  into empty, result: " << (fPush ? "success to push(wrong)" : "failure(correct)") << endl;
	cout << cbuffer0;
	cout << "Initiated as 10 from the beginning" << endl;
	CCIntBuffer<> cbuffer1(10);
	cout << cbuffer1;
	fPush = cbuffer1.Put(1);
	cout << "Push 1 result, must be success: " << (fPush ? "success" : "failure") << endl;
	cout << cbuffer1;
	int readValue = -1;
	bool fGet = cbuffer1.GetPop(readValue);
	cout << "Get 1 result, must be success: " << (fGet ? "success" : "failure");
	if(fGet) 
		cout << " Value extracted: " << readValue;
	cout << endl;
	cout << cbuffer1;
	cout << "test case boundary full. ALmost, ful, try to Push into a full" << endl;
	for (int i = 0; i < 9; i++) {
		bool fPush = cbuffer1.Put(11 + i);
		cout << "Push " << i << " result, must be success: " << (fPush ? "success" : "failure") << endl;
	}
	cout << cbuffer1;
	bool fPush1 = cbuffer1.Put(20);
	cout << "Push 20, result must be success: " << (fPush1 ? "success" : "failure") << endl;
	cout << cbuffer1;
	fPush1 = cbuffer1.Put(21);
	cout << "Push 21, result must be failure: " << (fPush1 ? "success" : "failure") << endl;
	cout << cbuffer1;
	cout << "test case: read one from full. The push succesfully and insuccessfully" << endl;
	int out;
	bool resGet = cbuffer1.GetPop(out);
	cout << "Get oldest, result must be OK: " << (resGet ? "OK " : " Get failed ");
	if (resGet)cout << endl << "Value: " << out;
	cout << endl;
	cout << cbuffer1;
	
	fPush1 = cbuffer1.Put(22);
	cout << "Push 22, result, must be success: " << (fPush1 ? "success" : "failure") << endl;
	cout << cbuffer1;
	fPush1 = cbuffer1.Put(23);
	cout << "Push 23, result, must be failure: " << (fPush1 ? "success" : "failure") << endl;
	cout << cbuffer1;

	for (int i = 0; i < 11; i++)
	{
		bool resGet = cbuffer1.GetPop(out);
		if(10 == i)
			cout << "Get oldest, result must be failure: " << (resGet ? "Get OK " : " Get failed ");
		else
			cout << "Get oldest, result must be success: " << (resGet ? "Get OK " : " Get failed ");

		if (resGet)cout << endl << "Value: " << out;
		cout << endl;
	}
	cout << cbuffer1;
	cout << "Test case refill from not-empty" << endl;
	fPush = cbuffer1.Put(100);
	cout << "Push result: " << (fPush ? "success" : "failure") << endl;
	cout << cbuffer1;
	for (int i = 0; i < 12; i++) {
		bool fPush = cbuffer1.Put(10 + i);
		if(i > 8)
			cout << "Push #" << i << " of value: " << 10 + i << " result, must be failure: " << (fPush ? "success" : "failure") << endl;
		else
			cout << "Push #" << i << " of value: " << 10 + i << " result, must be success: " << (fPush ? "success" : "failure") << endl;
	}
	cout << cbuffer1;

	cout << "Test case one out one in" << endl;
	for (int i = 0; i < 30; i++) {
		bool resGet = cbuffer1.GetPop(out);
		cout << "Get #" << i <<  " result, must be success: " << (resGet ? "success" : "failure") <<" Value: "<<out<< endl;
		bool fPush = cbuffer1.Put(200 + i);
		cout << "Push #" << i << " of value: " << 200 + i << " result, must be sucess: " << (fPush ? "success" : "failure") << endl;
		cout << cbuffer1;
	}
}

