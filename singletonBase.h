#pragma once

#ifndef SINGLE_H
#define SINGLE_H

template<typename T>
class SingletonBase
{
protected:
	//�̱��� �ν��Ͻ� ����
	static T* instance;

	SingletonBase() {}
	~SingletonBase() {}

public:
	static T* GetInstance();
	void ReleaseSingleton();

};

//�̱��� �ʱ�ȭ
template<typename T>
T* SingletonBase<T>::instance = NULL;

//�̱��� �� �������� �Լ�
template<typename T>
T* SingletonBase<T>::GetInstance()
{
	if (!instance) instance = new T;
	return instance;
}

//�̱��� �޸� ����
template <typename T>
void SingletonBase<T>::ReleaseSingleton()
{
	if (singleton)
	{
		delete singleton;
		singleton = NULL;
	}
}

#endif