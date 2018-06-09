#ifndef SimpleMap_h
#define SimpleMap_h

#include <stddef.h>
#include <functional>

template<class T, class U>
struct Node {
    T           key;
    U           data;
    Node<T, U>* next;
};

template<typename T, typename U>
class SimpleMap {
    public:
        int (*compare)(T a, T b);

        SimpleMap(int (*compare)(T a, T b));
        ~SimpleMap();

        virtual int size();
        virtual void clear();
        virtual void remove(T key);
        virtual void remove(int i);
        virtual void put(T key, U obj);
        virtual U get(T key);
        virtual T getKey(int i);
        virtual U getData(int i);
        virtual int getIndex(T key);
        virtual bool has(T key);

    protected:
        int listSize;
        Node<T, U>* listBegin;
        Node<T, U>* listEnd;

        // Helps get() method by saving last position
        Node<T, U>* lastNodeGot = NULL;
        int lastIndexGot        = -1;
        bool isCached           = false;

        virtual Node<T, U>* getNode(T key);
        virtual Node<T, U>* getNodeIndex(int index);
};

template<typename T, typename U>
SimpleMap<T, U>::SimpleMap(int (*compare)(T a, T b)) {
    SimpleMap<T, U>::compare = compare;
    listBegin                = NULL;
    listEnd                  = NULL;
    listSize                 = 0;
    isCached                 = false;
    lastIndexGot             = -1;
    lastNodeGot              = NULL;
}

// Clear Nodes and free Memory
template<typename T, typename U>
SimpleMap<T, U>::~SimpleMap() {
    clear();
}

template<typename T, typename U>
U SimpleMap<T, U>::get(T key) {
    Node<T, U>* h = getNode(key);
    return h ? h->data : U();
}

template<typename T, typename U>
Node<T, U>* SimpleMap<T, U>::getNode(T key) {
    if (listSize > 0) {
        if ((compare(key, listBegin->key) < 0) || (compare(key, listEnd->key) > 0)) return NULL;

        Node<T, U>* h = listBegin;

        int lowerEnd = 0;
        int upperEnd = listSize - 1;
        int res;
        int mid = (lowerEnd + upperEnd) / 2;

        int hIndex = 0;

        while (lowerEnd <= upperEnd) {
            h      = lastNodeGot;
            hIndex = lastIndexGot;

            res = compare(key, getNodeIndex(mid)->key);

            if (res == 0) {
                return lastNodeGot;
            } else if (res < 0) {
                // when going left, set cached node back to previous cached node
                lastNodeGot  = h;
                lastIndexGot = hIndex;
                isCached     = true;

                upperEnd = mid - 1;
                mid      = (lowerEnd + upperEnd) / 2;
            } else if (res > 0) {
                lowerEnd = mid + 1;
                mid      = (lowerEnd + upperEnd) / 2;
            }
        }
    }
    return NULL;
}

template<typename T, typename U>
Node<T, U>* SimpleMap<T, U>::getNodeIndex(int index) {
    if ((index < 0) || (index >= listSize)) {
        return NULL;
    }

    Node<T, U>* hNode = listBegin;
    int c             = 0;

    if (isCached && (index >= lastIndexGot)) {
        c     = lastIndexGot;
        hNode = lastNodeGot;
    }

    while (hNode != NULL && c < index) {
        hNode = hNode->next;
        c++;
    }

    if (hNode) {
        isCached     = true;
        lastIndexGot = c;
        lastNodeGot  = hNode;
    }

    return hNode;
}

template<typename T, typename U>
void SimpleMap<T, U>::clear() {
    Node<T, U>* h = listBegin;
    Node<T, U>* toDelete;

    while (h != NULL) {
        toDelete = h;
        h        = h->next;
        delete toDelete;
    }

    listBegin = NULL;
    listEnd   = NULL;
    listSize  = 0;

    isCached     = false;
    lastIndexGot = -1;
    lastNodeGot  = NULL;
}

template<typename T, typename U>
int SimpleMap<T, U>::size() {
    return listSize;
}

template<typename T, typename U>
void SimpleMap<T, U>::put(T key, U obj) {
    // create new node
    Node<T, U>* newNode = new Node<T, U>();
    newNode->next = NULL;
    newNode->data = obj;
    newNode->key  = key;

    // look if already in list
    Node<T, U>* h = listBegin;
    Node<T, U>* p = NULL;
    bool found    = false;
    int  c        = 0;

    if (listSize > 0) {
        while (h != NULL && !found) {
            if (h->key == key) {
                found = true;
            } else {
                p = h;
                h = h->next;
                c++;
            }
        }
    }

    // replace old node with new one
    if (found) {
        if (h == listBegin) listBegin = newNode;

        if (h == listEnd) listEnd = newNode;

        if (p) p->next = newNode;
        newNode->next = h->next;
        delete h;

        lastIndexGot = c;
    }

    // create new node
    else {
        if (listSize == 0) {
            // add at start (first node)
            listBegin = newNode;
            listEnd   = newNode;

            lastIndexGot = 0;
        } else {
            if (key >= listEnd->key) {
                // add at end
                listEnd->next = newNode;
                listEnd       = newNode;

                lastIndexGot = listSize;
            } else if (key <= listBegin->key) {
                // add at start
                newNode->next = listBegin;
                listBegin     = newNode;

                lastIndexGot = 0;
            } else {
                // insertion sort
                h     = listBegin;
                p     = NULL;
                found = false;
                c     = 0;

                while (h != NULL && !found) {
                    if (compare(h->key, key) > 0) {
                        found = true;
                    } else {
                        p = h;
                        h = h->next;
                        c++;
                    }
                }
                newNode->next = h;

                if (p) p->next = newNode;

                lastIndexGot = c;
            }
        }

        listSize++;
    }

    isCached    = true;
    lastNodeGot = newNode;
}

template<typename T, typename U>
void SimpleMap<T, U>::remove(T key) {
    if (listSize > 0) {
        if ((compare(key, listBegin->key) < 0) || (compare(key, listEnd->key) > 0)) return;

        Node<T, U>* h = listBegin;
        Node<T, U>* p = NULL;
        bool found    = false;

        while (h != NULL && !found) {
            if (h->key == key) {
                found = true;
            } else {
                p = h;
                h = h->next;
            }
        }

        if (found) {
            if (p) p->next = h->next;
            else listBegin = h->next;

            if (listEnd == h) listEnd = p;
            listSize--;
            delete h;
        }
    }
}

template<typename T, typename U>
void SimpleMap<T, U>::remove(int i) {
    if (listSize > 0) {
        Node<T, U>* h = getNodeIndex(i);

        if (h != NULL) {
            Node<T, U>* p = getNodeIndex(i);

            if (p != NULL) p->next = h->next;

            if (h == listBegin) listBegin = h->next;

            if (h == listEnd) listEnd = p;

            listSize--;
            delete h;
        }
    }
}

template<typename T, typename U>
bool SimpleMap<T, U>::has(T key) {
    return getNode(key) != NULL;
}

template<typename T, typename U>
T SimpleMap<T, U>::getKey(int i) {
    Node<T, U>* h = getNodeIndex(i);
    return h ? h->key : T();
}

template<typename T, typename U>
U SimpleMap<T, U>::getData(int i) {
    Node<T, U>* h = getNodeIndex(i);
    return h ? h->data : U();
}

template<typename T, typename U>
int SimpleMap<T, U>::getIndex(T key) {
    return getNode(key) ? lastIndexGot : -1;
}

#endif // ifndef SimpleMap_h