#pragma once

class ThreadNumRAII{
public:
	ThreadNumRAII(std::atomic<unsigned int> * p_thread_num)
	{
		m_p_thread_num = p_thread_num;
		m_p_thread_num->fetch_add(1);		
	}
	~ThreadNumRAII()
	{
		m_p_thread_num->fetch_sub(1);
	}
	
private:
	std::atomic<unsigned int>* m_p_thread_num;
};
