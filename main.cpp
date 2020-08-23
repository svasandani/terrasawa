using namespace std;

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <cstdlib>

mutex mhead;
mutex mtail;
mutex msent;

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

void enqueue(Node *sentinel, Node **tail, string data) {
    Node *n = new Node(data);
    
    mtail.lock();
    if (*tail == NULL) {
        sentinel->setNext(n);
        *tail = n;
    } else {
        (*tail)->setNext(n);

        *tail = n;
    }
    mtail.unlock();
}

Node* dequeue(Node *sentinel, Node **head) {
    mhead.lock();
    msent.lock();

    Node *toReturn = *head;

    *head = (*head)->getNext();

    sentinel->setNext(*head);

    msent.unlock();
    mhead.unlock();

    toReturn->setNext(NULL);

    return toReturn;
}

void print(Node *head) {
    while (head != NULL) {
        cout << head->getData() << endl;

        head = head->getNext();
    }
}

void doDequeue(Node *sentinel, Node **head) {
    // dequeue(head, tail);
    cout << dequeue(sentinel, head)->getData() << endl;
}

int main() {
    Node *head = new Node("head node");
    Node *tail = head;

    Node *sentinel = new Node("");
    sentinel->setNext(head);

    enqueue(sentinel, &tail, "next 1");
    enqueue(sentinel, &tail, "next 2");
    enqueue(sentinel, &tail, "next 3");
    enqueue(sentinel, &tail, "next 4");
    enqueue(sentinel, &tail, "next 5");

    print(head);
    
    thread threads[1000];

    for (int i=0; i<500; ++i) {
        if (rand() % 3 == 0) {
            threads[i] = thread(doDequeue, sentinel, &head);
        } else {
            threads[i] = thread(enqueue, sentinel, &tail, "next " + to_string(i));
        } 
    }

    for (int i=500; i<1000; ++i) {
        if (rand() % 3 == 0) {
            threads[i] = thread(enqueue, sentinel, &tail, "next " + to_string(i));
        } else {
            threads[i] = thread(doDequeue, sentinel, &head);
        } 
    }
        

    for (auto& th : threads) th.join();

    // print(head);

    return 0;
}
