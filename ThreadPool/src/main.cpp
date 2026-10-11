#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
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

    void submit(std::function<void()> task) {
        //Ajouter la tâche à la queue de tâches (thread-safe)
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            if(stop_) {
                throw std::runtime_error("ThreadPool is stopped");
            }
            tasks.push(std::move(task));
        }

        //Notifier un thread en attente qu'une nouvelle tâche est disponible
        condition_.notify_one();
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

    pool.submit([](){
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          std::cout << "Task 1 done" << std::endl;
        });

    pool.submit([](){
          std::this_thread::sleep_for(std::chrono::milliseconds(200));
          std::cout << "Task 2 done" << std::endl;
        });

    pool.submit([](){
          std::cout << "Task 3 done" << std::endl;
        });

}


int main() {
    testThreadPool();
    return 0;
}
