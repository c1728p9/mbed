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

#include "LinkedList.h"
#include "LinkEntry.h"

LinkedList::LinkedList(): _head(0), _tail(0)
{

}

LinkedList::~LinkedList()
{

}

void LinkedList::enqueue(LinkEntry *entry)
{
    entry->_next = NULL;
    if (_tail == NULL) {
        _head = entry;
    } else {
        _tail->_next = entry;
    }
    _tail = entry;
}

LinkEntry *LinkedList::dequeue()
{
    if (_head == NULL) {
        return NULL;
    }
    if (_head->_next == NULL) {
        _tail = NULL;
    }
    LinkEntry *entry = _head;
    _head = _head->_next;
    entry->_next = NULL;
    return entry;
}
