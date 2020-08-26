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

    while(i < 5) {
        atomic<Node*> *currentTail;

        currentTail->store(tail->load());

        atomic<Node*> *currentNext = currentTail->load()->getNextAtomicP();

        if (currentTail->load() == tail->load()) {
            if (currentNext->load() == NULL) {
                Node *o = (*tail).load()->getNext();
                if (currentNext->compare_exchange_weak(o, n)) {
                    Node *p = currentTail->load();
                    cout << tail->load() << endl;
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
                cout << currentNext->load()->getData() << endl;
                i++;
            }
        } else {
            cout << "Failed on first check" << endl;
            i++;
        }
    }
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
        cout << "|";
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
    atomic<Node*> *tail = new atomic<Node*>;

    tail->store(head);

    Node *sentinel = new Node("");
    sentinel->setNext(head);

    enqueue(tail, "next 1");
    enqueue(tail, "next 2");
    enqueue(tail, "next 3");
    enqueue(tail, "next 4");
    enqueue(tail, "next 5");

    print(head);
    
    // thread threads[1000];

    // for (int i=0; i<500; ++i) {
    //     if (rand() % 3 == 0) {
    //         threads[i] = thread(doDequeue, sentinel, &head);
    //     } else {
    //         threads[i] = thread(enqueue, sentinel, tail, "next " + to_string(i));
    //     } 
    // }

    // for (int i=500; i<1000; ++i) {
    //     if (rand() % 3 == 0) {
    //         threads[i] = thread(enqueue, sentinel, tail, "next " + to_string(i));
    //     } else {
    //         threads[i] = thread(doDequeue, sentinel, &head);
    //     } 
    // }
        

    // for (auto& th : threads) th.join();

    // print(head);

    return 0;
}
