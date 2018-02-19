/*
 *
 *  mooca_threadpool.hpp
 *  mooca
 *
 *  Created by jelly on 19/02/2018.
 *  Copyright © 2018 mooca.io. All rights reserved.
 *
 */


#ifndef mooca_threadpool_hpp
#define mooca_threadpool_hpp

#include <iostream>
#include <functional>
#include <condition_variable>
#include <queue>
#include <vector>
#include <thread>

namespace mooca {
	class ThreadPool {
private:
		std::queue<std::function<void()> >	tasks;
		std::vector<std::thread>		works;
		std::condition_variable			cv;
		std::mutex				queue_mutex;
		bool					stop;

public:
		ThreadPool( size_t threads );
		template <typename F, typename... Args> void enqueue( F && f, Args &&... args );
        ~ThreadPool();
	};


	inline ThreadPool::ThreadPool( size_t threads ) : stop( false )
	{
		for ( size_t i = 0; i < threads; i++ )
		{
			works.push_back( std::thread(
						 [this]() {
							 for (;; )
							 {
                                std::function<void()> task;
                                {
                                    std::unique_lock<std::mutex> lock( this->queue_mutex );
                                    this->cv.wait(lock,[this]{return this->stop || !this->tasks.empty();});

                                     if ( this->stop && this->tasks.empty() )
                                     {
                                         return;
                                     }
                                     
                                     task = std::move(this->tasks.front());
                                     this->tasks.pop();
                                }
                                task();
							 }
						 }
						 ) );
		}
    }

		template <typename F, typename... Args> 
        void ThreadPool::enqueue( F && f, Args &&... args )
		{
			auto task = std::bind( std::move<F>( f ), std::move<Args>( args ) ... );
			{
				std::unique_lock<std::mutex> lock( this->queue_mutex );
				if (stop)
				{
					throw std::runtime_error( "enqueue on stopped ThreadPool" );
				}
				tasks.push( std::move(task) );
			}
			cv.notify_one();
		}


		inline ThreadPool::~ThreadPool()
		{
			{
				std::unique_lock<std::mutex> lock( this->queue_mutex );
				this->stop = true;
			}

            cv.notify_all();
			for ( auto &thread : this->works )
			{
                if(thread.joinable()){
				    thread.join();
                }
			}
		}
}

#endif /* mooca_threadpool_hpp */

