#include <vector>
#include <queue>
#include <thread>
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
        //Ajouter la tâche à la queue de tâches (pas thread-safe)
        tasks.push(std::move(task));
    }

      ~ThreadPool()
      {
        //signaler à tous les threads de s'arrêter et attendre qu'ils terminent
        stop_ = true;

        for (std::thread &worker : workers) {
            worker.join();
        }
      }

  private:

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    bool stop_;

    void workerThread() {
        while (true) {
            //Si stop_ est vrai et qu'il n'y a plus de tâches, sortir de la boucle
            if (stop_ && tasks.empty()) {
                return;
            }

            //Attente active : on reboucle tant qu'il n'y a rien à faire
            if (tasks.empty()) {
                continue;
            }

            //prendre la première tâche de la queue et la retirer de la queue
            std::function<void()> task = std::move(tasks.front());
            tasks.pop();

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
