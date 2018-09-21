#pragma once

#ifndef SINGLE_H
#define SINGLE_H

template<typename T>
class SingletonBase
{
protected:
	//싱글톤 인스턴스 선언
	static T* instance;

	SingletonBase() {}
	~SingletonBase() {}

public:
	static T* GetInstance();
	void ReleaseSingleton();

};

//싱글톤 초기화
template<typename T>
T* SingletonBase<T>::instance = NULL;

//싱글톤 값 가져오는 함수
template<typename T>
T* SingletonBase<T>::GetInstance()
{
	if (!instance) instance = new T;
	return instance;
}

//싱글톤 메모리 해제
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