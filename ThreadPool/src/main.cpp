#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <iostream>




class ThreadPool {
public:
    ThreadPool(size_t numThreads):
        stop_(false) {
          //Créer numThreads threads et les ajouter au vecteur workers
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back(&ThreadPool::workerThread, this);
        }
    }

    template<typename F,typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {

      //type de retour de la fonction f avec les arguments args
        using return_type = decltype(f(args...));

        //Créer un packaged_task pour encapsuler la fonction f avec les arguments args
        auto task = std::make_shared<std::packaged_task<return_type()>>(
          [f=std::forward<F>(f), args...]() mutable { return f(args...); }
        );

        //Récupérer le futur avant de move la stack
        auto future = task->get_future();

        //Ajouter la tâche à la queue de tâches (thread-safe)
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            if(stop_) {
                throw std::runtime_error("ThreadPool is stopped");
            }
            tasks.push([task]() {(*task)();}); //Wrapper dans std::function<void()>
        }

        //Notifier un thread en attente qu'une nouvelle tâche est disponible
        condition_.notify_one();
        return future;
    }
      ~ThreadPool()
      {
        //signaler à tous les threads de s'arrêter et attendre qu'ils terminent
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            stop_ = true;
        }

        condition_.notify_all();

        for (std::thread &worker : workers) {
            worker.join();
        }
      }

  private:

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    bool stop_;

    void workerThread() {
        while (true) {
            std::function<void()> task;

            //Attendre une tache ou un signal d'arret
            {
                std::unique_lock<std::mutex> lock(queueMutex_);

                //Attendre jusqu'à ce qu'une tâche soit disponible ou que le thread pool soit arrêté
                condition_.wait(lock, [this] { return stop_ || !tasks.empty(); });

                //Si stop_ est vrai et qu'il n'y a plus de tâches, sortir de la boucle
                if (stop_ && tasks.empty()) {
                    return;
                }

                //prendre la première tâche de la queue et la retirer de la queue
                task = std::move(tasks.front());
                tasks.pop();
            }

            task();
        }
    }
};

void testThreadPool() {
    ThreadPool pool(4);

    auto future1 = pool.submit([](int x){
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          return x * x;
        }, 5);    
    
    auto future2 = pool.submit([](const std::string& s){
          std::this_thread::sleep_for(std::chrono::milliseconds(200));
          return s + " processed";
        }, std::string("hello"));  

    auto future3 = pool.submit([](){
          return 42;
        });

    std::cout << "Result of future1: " << future1.get() << std::endl; // Should print 25
    std::cout << "Result of future2: " << future2.get() << std::endl; // Should print "hello processed"
    std::cout << "Result of future3: " << future3.get() << std::endl; // Should print 42

}


int main() {
    testThreadPool();
    return 0;
}