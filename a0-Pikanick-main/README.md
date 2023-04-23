## CMPT 300 Assignment 0: C Programming Basics

**Total points**: 100

**Overall percentage**: 5%

**Due:** Tue, Jan 17, 23:59:59 PT

This assignment should be completed individually.

### 1. Overview

This assignment will get you started in C programming which will be needed in future assignments.
All code **must** run on a Linux machine; when in doubt, use a CSIL machine/server. We will grade
your code on a Linux machine.  You may organize your work by creating a directory for this
assignment, such as `~/cmpt300/` and clone this repo under it.

### 2. Tasks
There are three tasks in this assignment, and you should finish them **using C**.

#### 2.0. Task 0: Cloning the Repo

The first thing you need to do is to clone this assignment repo. You should have accepted the invite
from GitHub Classroom and it should've given you the URL of your own repo, which looks something
like `https://github.com/CMPT-300-Spring-2023/a0-[your identifier]`.

Once you go there, you should see your repo with the code template for Assignment 0. Clone the code
locally and start working on the assignment. You should do something like the following for cloning
(modulo the actual URL):

```
$ git clone git@github.com:CMPT-300-Spring-2023/a0-[your identifier].git
```

If you click the green `<> Code` button on the repo webpage, you will get the URL to clone. Once you
have cloned the repo locally, you are ready to do the assignment.

#### 2.1. Task 1: String Manipulation

In this task, you will develop a few string manipulation functions similar to the standard C string
functions. You are **not** allowed to use any standard string functions (e.g., `strcpy`, `strlen`,
etc.) in your code. However, your functions may call each other as necessary.

Go to `mystring/` under your cloned repo directory. You will find the following files:

- `mystring.h`: header file containing the signatures for the functions that you will develop.
- `mystring.c`: source file in which you will do your coding.
- `test_mystring.c`: a test file to help you test your code. (For actual grading, we *may* use more
  test cases.)
- `Makefile`: this file is used by the Unix command `make` to compile your source code and build the
  executable files. **Do not** change anything in this `Makefile`. If you do, we won't be able to
  grade your submission.

**Do not** add any other files or change the file names. If you do, we won't be able to grade your
submission.

Start by issuing the command `make` to ensure that you got everything ready to start coding. It
should throw some errors as it runs the test cases. Since you don't have your implementation yet,
the test cases will fail. Nevertheless, the code should compile without a problem and you should
find an executable file named `test_mystring`. Try it by issuing the command `./test_mystring`.
Again, the tests there will fail. Now, start coding the functions in `mystring.c` one at a time and
run `./test_mystring` to test the code. However, **do not use `./test_mystring` for debugging
purposes.** It's meant for testing, not debugging. Use a debugger for debugging. You can modify
`test_mystring.c` to do more tests as necessary.

In general, you can assume that all pointers passed into your code will be valid, and that there
will be enough space allocated for any copy operations to succeed. We must assume this because in C
we have no way of checking if there is enough space. You may `#include` any extra `.h` files as
required (such as `stdlib.h`).

#### 2.2. Task 2: Linked List

The linked list is a simple, yet powerful, data structure that appears---in one way or another---in
almost all reasonable-size programs that you will encounter in your career. In this problem, you
will implement several functions that create and manipulate a linked list. A node in the list is
defined as:

```
struct nodeStruct {
  int item;
  struct nodeStruct *next;
};
```
You must implement at least the following functions:

```
/*
 * Allocate memory for a node of type struct nodeStruct and initialize
 * it with the value item. Return a pointer to the new node.
 */
struct nodeStruct* List_createNode(int item);

/*
 * Insert node at the head of the list.
 */
void List_insertHead (struct nodeStruct **headRef, struct nodeStruct *node);

/*
 * Insert node after the tail of the list.
 */
void List_insertTail (struct nodeStruct **headRef, struct nodeStruct *node);

/*
 * Count number of nodes in the list.
 * Return 0 if the list is empty, i.e., head == NULL
 */
int List_countNodes (struct nodeStruct *head);

/*
 * Return the first node holding the value item, return NULL if none found
 */
struct nodeStruct* List_findNode(struct nodeStruct *head, int item);

/*
 * Delete node from the list and free memory allocated to it.
 * This function assumes that node has been properly set to a valid node 
 * in the list. For example, the client code may have found it by calling 
 * List_findNode(). If the list contains only one node, the head of the list 
 * should be set to NULL.
 */
void List_deleteNode (struct nodeStruct **headRef, struct nodeStruct *node);

/*
 * Sort the list in ascending order based on the item field.
 * Any sorting algorithm is fine.
 */
void List_sort (struct nodeStruct **headRef);
```

Each function (other than sort) should not alter the item value of nodes in the linked list. For
example, to insert a new node at the head of the list, a new node must be linked in at the front,
rather than all the item values shifted. (This does not apply to the sort function, which may alter
item values.)

Note that `struct nodeStruct **headRef` in the functions above enables you to modify the memory
location referred to by the variable `headRef` (e.g., you can write: `*headRef = node`). This is
important to handle boundary conditions such as inserting the first node in the list or deleting the
last node in the list. In such cases, you would need to change the head of the list itself, it is
why we pass pointer to the pointer. You can find more information on linked lists [in this
document](linkedLists.pdf).

To start writing your code, first go to `list/` directory and it should have the following files to
help you get started:
- `test_list.c`: to test your code, contains the `main()` function. Again, for actual grading, we
  *may* use more test cases.
- `Makefile`: for `make`. This compiles the program, generates an executable named `test_list`, and
  runs it. **Do not** change anything in this `Makefile`. If you do, we won't be able to grade your
  submission.


Now, you need to create the following files:
- `list.h`: should contain the definitions of `struct nodeStruct` and the function signatures. No
  `head` or `tail` pointer should be defined in there; they are just in the application (such as
  `main.c`).
- `list.c`: should contain your implementation of the above functions.

**Do not** add any other files or change the file names. If you do, we won't be able to grade your
submission.

Test your code very carefully. Your code will be run through the instructor's tests which don't
respond well to segfaults! In general, you can assume that all pointers passed into your code will
be valid.

#### 2.3. Task 3: Doubly-Linked List

In task 2 we focused on singly-linked list, i.e., we can only traverse the list from head to tail in
a single direction, by following the `next` pointer in each node. A doubly-linked list allows
one to traverse the list in both directions, with an extra `previous` pointer to the previous
node in the list. This is useful in many occasions but adds more complexity to the code.

In this problem, you need to implement a doubly-linked list. You should start by following the
exactly same structure and function definitions used in Problem 2 (i.e., `nodeStruct`, `List_*`
functions, etc.), and extend them with necessary additions: you will need to add a `previous`
pointer in each node, and link nodes together in both directions. More information about
doubly-linked list can also be found in [in this document](linkedLists.pdf).

You should structure your code similarly to the code in Problem 2.

To start writing your code, go to `dlist/` directory and it should have the following files to help
you get started:
- `test_dlist.c`: to test your code, contains the `main()` function. Again, for actual grading, we
  *may* use more test cases.
- `Makefile`: for `make`. This compiles the program, generates an executable named `test_dlist`, and
  runs it. **Do not** change anything in this `Makefile`. If you do, we won't be able to grade your
  submission.

Now, you need to create the following files:

- dlist.h: should contain the definition of `struct nodeStruct` and the function signatures. No
  `head` or `tail` pointers should be defined in there; they are just in the application (such as
  `main.c`).
- dlist.c: should contain your implementation of the above functions.

**Do not** add any other files or change the file names. If you do, we won't be able to grade your
submission.

Test your code very carefully. Your code will be run through the instructor's tests which don't
respond well to segfaults! Also, your doubly-linked list should also pass all the tests for the
singly-linked list in Problem 2.  

In general, you can assume that all pointers passed into your code will be valid.

### What to Submit and How

**Most importantly, make sure you push your code before the deadline.**

Make sure that your files are organized as follows:

- `mystring/`: everything related to the mystring part.
- `list/`: everything related the linked list part.
- `dlist/`: everything related the doubly-linked list part.

Add, commit, and push frequently to the remote repo. E.g.,

```
$ git add mystring/*.h mystring/*.c list/*.h list/*.c dlist/*.h dlist/*.c
$ git commit -m "your commit message (explain your commit)"
$ git push
```

**Again, make sure you push your code before the deadline.**

### Grading Policies

Make sure you are familiar with the course policies. Especially, we do not accept late submissions,
so please submit on time by the deadline.  

Your code must compile and run on Linux; **you will receive a 0 if your code does not compile.**
Sample solutions will **not** be provided for assignments. Your code should also not leak any
memory; we will check using Valgrind. Any memory leak will lead to -10 points.

### Acknowledgment

Created by Mohamed Hefeeda, modified by Brian Fraser, Keval Vora, Tianzheng Wang, and Steve Ko.
