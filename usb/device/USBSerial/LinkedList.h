/* mbed Microcontroller Library
 * Copyright (c) 2018-2018 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MBED_LINKED_LIST_H
#define MBED_LINKED_LIST_H

#include "LinkEntry.h"

class LinkedList<class T> {
public:
    LinkedList(): _head(0), _tail(0)
    {

    }

    ~LinkedList()
    {

    }

    /**
     * Return the element at the head of the list
     *
     * @return The element at the head of the list or NULL if the list is empty
     */
    T *head()
    {
        return _head;
    }

    /**
     * Add an element to the tail of the list
     *
     * @param entry New element to add
     */
    void enqueue(T *entry)
    {
        LinkEntry *link_entry = static_cast<LinkEntry*>(entry);
        link_entry->_next = NULL;
        if (_tail == NULL) {
            _head = entry;
        } else {
            _tail->_next = entry;
        }
        _tail = entry;
    }

    /**
     * Remove the element at the head of the list
     *
     * @return The element at the head of the list or NULL if the list is empty
     */
    T *dequeue()
    {
        if (_head == NULL) {
            return NULL;
        }
        if (_head->_next == NULL) {
            _tail = NULL;
        }
        T *entry = static_cast<T*>(_head);
        LinkEntry next = _head->_next;
        _head->_next = NULL;
        _head = next;
        return entry;
    }

private:
    LinkEntry *_head;
    LinkEntry *_tail;
};


#endif
