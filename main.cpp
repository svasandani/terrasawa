using namespace std;

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>

mutex mtx;

class Node {
    string data;
    Node *next;

    public: 

    Node(string data): data(data) {
        next = NULL;
    }

    string getData() {
        return data;
    }

    Node* getNext() {
        return this->next;
    }

    void setNext(Node *next) {
        this->next = next;
    }
};

void enqueue(Node **head, Node **tail, string data) {
    Node *n = new Node(data);
    
    mtx.lock();
    if (*tail == NULL) {
        *head = n;
        *tail = n;
    } else {
        (*tail)->setNext(n);

        *tail = n;
    }
    mtx.unlock();
}

Node* dequeue(Node **head, Node **tail) {
    mtx.lock();

    Node *toReturn = *head;

    *head = (*head)->getNext();

    if (*head == NULL) {
        *tail = NULL;
    }

    mtx.unlock();

    toReturn->setNext(NULL);

    return toReturn;
}

void print(Node *head) {
    while (head != NULL) {
        cout << head->getData() << endl;

        head = head->getNext();
    }
}

void doDequeue(Node **head, Node **tail) {
    // dequeue(head, tail);
    cout << dequeue(head, tail)->getData() << endl;
}

int main() {
    Node *head = new Node("head node");
    Node *tail = head;

    enqueue(&head, &tail, "next 1");
    enqueue(&head, &tail, "next 2");
    enqueue(&head, &tail, "next 3");
    enqueue(&head, &tail, "next 4");
    enqueue(&head, &tail, "next 5");

    print(head);
    
    thread dequeueThreads[5];

    for (int i=0; i<5; ++i)
        dequeueThreads[i] = thread(doDequeue, &head, &tail);

    for (auto& th : dequeueThreads) th.join();

    // print(head);

    return 0;
}
