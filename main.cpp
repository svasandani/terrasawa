using namespace std;

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <cstdlib>
#include <atomic>

mutex mhead;
mutex mtail;
mutex msent;

class Node {
    string data;
    atomic<Node*> next;

    public: 

    Node(string data): data(data) {
        next.store(NULL);
    }

    string getData() {
        return data;
    }

    Node* getNext() {
        return next.load();
    }

    atomic<Node*>* getNextAtomicP() {
        return &next;
    }

    void setNext(Node *next) {
        this->next = next;
    }
};

void enqueue(atomic<Node*>* tail, string data) {
    Node *n = new Node(data);

    int i = 0;

    while(i < 1000) {
        atomic<Node*> *currentTail = new atomic<Node*>;

        currentTail->store(tail->load());

        atomic<Node*> *currentNext = currentTail->load()->getNextAtomicP();

        if (currentTail->load() == tail->load()) {
            if (currentNext->load() == NULL) {
                Node *o = (*tail).load()->getNext();
                if (currentNext->compare_exchange_weak(o, n)) {
                    Node *p = currentTail->load();
                    if (tail->compare_exchange_weak(p, n)) {
                        cout << "Successfully enqueued " << data << endl;
                        break;
                    } else {
                        cout << "Failed on fourth check" << endl;
                    }
                } else {
                    cout << "Failed on third check" << endl;
                }
            } else {
                cout << "Failed on second check" << endl;
                cout << "current tail is " << tail->load()->getData() << endl;
                cout << "trying to enqueue " << data << endl;
                cout << "value at tail is actually " << currentNext->load()->getData() << endl;
                i++;
            }
        } else {
            cout << "Failed on first check" << endl;
            i++;
        }
    }
}

Node* dequeue(atomic<Node*>* sentinel, atomic<Node*>* tail) {
    if (sentinel->load()->getNext() == NULL) {
        return NULL;
    }

    if (sentinel->load()->getNext() == tail->load()) {
        Node *toReturn = NULL;

        toReturn = sentinel->load()->getNext();

        sentinel->load()->setNext(NULL);

        return toReturn;
    }

    while(true) {
        atomic<Node*> *currentHead = new atomic<Node*>;

        currentHead->store(sentinel->load()->getNext());

        atomic<Node*> *currentNext = currentHead->load()->getNextAtomicP();

        Node *n = currentHead->load();
        if (sentinel->load()->getNextAtomicP()->compare_exchange_weak(n, currentNext->load())) {
            return currentHead->load();
        } else {
            cout << "Failed on first check" << endl;
        }
    }
}

void print(Node *head) {
    while (head != NULL) {
        cout << "|";
        cout << head->getData() << endl;

        head = head->getNext();
    }
}

void doDequeue(atomic<Node*>* sentinel, atomic<Node*>* tail) {
    // dequeue(head, tail);
    Node *d = dequeue(sentinel, tail);

    if (d == NULL) {
        cout << "!! Nothing to dequeue!" << endl;
    } else {
        cout << ">> Successfully dequeued " << d->getData() << endl;
    }
}

void sayHi(int i) {
    cout << "Hi from thread " << i << endl;
}

int main() {
    Node *head = new Node("head node");
    atomic<Node*> *tail = new atomic<Node*>;

    tail->store(head);

    atomic<Node*> *sentinel = new atomic<Node*>;
    
    Node *sent = new Node("");
    sent->setNext(head);

    sentinel->store(sent);

    enqueue(tail, "next 1");
    enqueue(tail, "next 2");
    enqueue(tail, "next 3");
    enqueue(tail, "next 4");
    enqueue(tail, "next 5");

    print(sentinel->load()->getNext());

    int i = 0;

    while (true) {
    
    thread threads[4096];

    for (int i=0; i<2048; ++i) {
        if (rand() % 3 == 0) {
            threads[i] = thread(doDequeue, sentinel, tail);
        } else {
            threads[i] = thread(enqueue, tail, "next " + to_string(i));
        } 
    }

    for (int i=2048; i<4096; ++i) {
        if (rand() % 3 == 0) {
            threads[i] = thread(enqueue, tail, "next " + to_string(i));
        } else {
            threads[i] = thread(doDequeue, sentinel, tail);
        } 
    }
        
    for (auto& th : threads) th.join();

    cout << endl << endl;

    print(sentinel->load()->getNext());

    i++;

    }

    return 0;
}
