#include <atomic>
#include <cstdlib>
#include <iostream>
#include <utility>  // std::to_underlying (C++23)

class LockFreeStack {
private:
    struct Node {
        int data;
        Node* next;
        Node(int d) : data(d), next(nullptr) {}
    }; 

    std::atomic<Node*> head{nullptr};

public:
    void push(int data) {
        Node* newNode = new Node(data);
        newNode->next = head.load(std::memory_order_relaxed);
        while (!head.compare_exchange_weak(newNode->next, newNode,
                                           std::memory_order_release,
                                           std::memory_order_relaxed)) {
            // Loop until the head is successfully updated
        }
    }

    bool pop(int& result) {
        Node* oldHead = head.load(std::memory_order_relaxed);
        while (oldHead != nullptr) {
            if (head.compare_exchange_weak(oldHead, oldHead->next,
                                           std::memory_order_acquire,
                                           std::memory_order_relaxed)) {
                result = oldHead->data;
                delete oldHead;
                return true;
            }
        }
        return false; // Stack was empty
    }

    bool empty() const {
        return head.load(std::memory_order_relaxed) == nullptr;
    }

    void printStack() const {
        Node* current = head.load(std::memory_order_relaxed);
        std::cout << "Stack contents: ";
        while (current != nullptr) {
            std::cout << current->data << " -> ";
            current = current->next;
        }
        std::cout << std::endl;
    }
};

int main() {
    LockFreeStack stack;

    // Push elements onto the stack
    std::cout << "Test Basique" << std::endl;
    stack.push(10);
    stack.push(20);
    stack.push(30);

    // Print the stack contents
    stack.printStack();

    // Pop elements from the stack
    int value;
    if (stack.pop(value)) {
        std::cout << "Pop: " << value << std::endl;
    }

    stack.printStack();


    std::cout << "Test stack vide" << std::endl;
    LockFreeStack emptyStack;
    if (!emptyStack.pop(value)) {
        std::cout << "Stack Vide OK" << std::endl;
    }

    return 0;
}