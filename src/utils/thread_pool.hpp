#pragma once

namespace utils
{
	class thread_pool
	{
	public:
		using job = std::function<void()>;
		using job_ptr = std::unique_ptr<job>;

		friend class worker;
		class worker
		{
		public:
			worker();

			friend class thread_pool;

			void start(thread_pool& pool);
			void stop();
			void loop(thread_pool& pool);

		private:
			std::thread thread_;

		};

		thread_pool() = default;
		thread_pool(const std::size_t num_workers);
		void initialize(const std::size_t num_workers);

		void start();
		void update();
		void stop();

		template <typename F>
		void push(F&& job)
		{
			if (this->stopped_)
			{
				return;
			}

			std::lock_guard lock(this->mutex_);
			this->queue_.emplace_back(std::make_unique<thread_pool::job>(std::forward<F>(job)));
			this->event_.notify_one();
		}

	private:
		thread_pool::job_ptr pop_job();
		void run_job();

		std::mutex mutex_;
		std::atomic_bool stopped_;
		std::deque<job_ptr> queue_;
		std::condition_variable event_;
		std::deque<std::unique_ptr<worker>> workers_;

	};
}
