

#include <iostream>
#include <string>
using namespace std;

struct Book {
    int code;
    string title;
    string author;
    int year;
    bool available;
    
    string requestQueue[20];
    int front = 0;
    int rear = -1;
    
    bool isQueueEmpty() { return front > rear; }
    bool isQueueFull() { return rear >= 19; }
    void addToQueue(string name) { 
        if(!isQueueFull()) requestQueue[++rear] = name; 
    }
    string getFromQueue() { 
        return !isQueueEmpty() ? requestQueue[front++] : ""; 
    }
    int queueSize() { 
        return rear - front + 1; 
    }
};

enum ActionType {
    ADD_BOOK,
    DELETE_BOOK,
    BORROW_BOOK,
    RETURN_BOOK,
    SORT_BOOKS
};

struct Action {
    ActionType type;
    Book book;
    string description;
};

Book books[200];
int bookCount = 0;

Action undoStack[10];
Action redoStack[10];
int undoTop = -1;
int redoTop = -1;

string history[10];
int historyCount = 0;

void saveToHistory(string desc) {
    if(historyCount < 10) {
        history[historyCount++] = desc;
    } else {
        for(int i = 0; i < 9; i++)
            history[i] = history[i + 1];
        history[9] = desc;
    }
}

void saveForUndo(ActionType type, Book b, string desc) {
    if(undoTop < 9) undoTop++;
    else {
        for(int i = 0; i < 9; i++) undoStack[i] = undoStack[i + 1];
        undoTop = 9;
    }
    undoStack[undoTop].type = type;
    undoStack[undoTop].book = b;
    undoStack[undoTop].description = desc;
    
    redoTop = -1;
    saveToHistory(desc);
}

int findBookByCode(int code) {
    for(int i = 0; i < bookCount; i++)
        if(books[i].code == code) return i;
    return -1;
}

void printBookInfo(Book b) {
    cout << "\n================================\n";
    cout << "Book Code: " << b.code << "\n";
    cout << "Title: " << b.title << "\n";
    cout << "Author: " << b.author << "\n";
    cout << "Year: " << b.year << "\n";
    cout << "Status: " << (b.available ? "Available" : "Borrowed") << "\n";
    if(b.queueSize() > 0)
        cout << "Waiting list: " << b.queueSize() << " person(s)\n";
    cout << "================================\n";
}

void addBook() {
    if(bookCount >= 200) {
        cout << "Library is full!\n";
        return;
    }
    
    Book newBook;
    cout << "\nAdd New Book:\n";
    cout << "Book Code: ";
    cin >> newBook.code;
    
    if(findBookByCode(newBook.code) != -1) {
        cout << "Duplicate book code!\n";
        return;
    }
    
    cin.ignore();
    cout << "Title: ";
    getline(cin, newBook.title);
    cout << "Author: ";
    getline(cin, newBook.author);
    cout << "Year: ";
    cin >> newBook.year;
    
    newBook.available = true;
    newBook.front = 0;
    newBook.rear = -1;
    
    books[bookCount++] = newBook;
    saveForUndo(ADD_BOOK, newBook, "Added book: " + newBook.title);
    cout << "Book added successfully.\n";
}

void removeBook() {
    int code;
    cout << "\nRemove Book:\n";
    cout << "Book Code: ";
    cin >> code;
    
    int index = findBookByCode(code);
    if(index == -1) {
        cout << "Book not found!\n";
        return;
    }
    
    Book deleted = books[index];
    saveForUndo(DELETE_BOOK, deleted, "Removed book: " + deleted.title);
    
    for(int i = index; i < bookCount - 1; i++)
        books[i] = books[i + 1];
    bookCount--;
    
    cout << "Book removed.\n";
}

void searchBook() {
    int code;
    cout << "\nSearch Book:\n";
    cout << "Book Code: ";
    cin >> code;
    
    int index = findBookByCode(code);
    if(index == -1) {
        cout << "Book not found!\n";
        return;
    }
    
    printBookInfo(books[index]);
    saveToHistory("Searched book: " + books[index].title);
}

void borrowBook() {
    int code;
    cout << "\nBorrow Book:\n";
    cout << "Book Code: ";
    cin >> code;
    
    int index = findBookByCode(code);
    if(index == -1) {
        cout << "Book not found!\n";
        return;
    }
    
    cin.ignore();
    string name;
    cout << "Borrower Name: ";
    getline(cin, name);
    
    if(books[index].available) {
        books[index].available = false;
        saveForUndo(BORROW_BOOK, books[index], 
                   "Borrowed by " + name + ": " + books[index].title);
        cout << "Book borrowed by " << name << ".\n";
    } else {
        if(books[index].isQueueFull()) {
            cout << "Waiting list is full!\n";
            return;
        }
        books[index].addToQueue(name);
        saveToHistory("Borrow request by " + name + ": " + books[index].title);
        cout << "Added to waiting list. Position: " 
             << books[index].queueSize() << "\n";
    }
}

void returnBook() {
    int code;
    cout << "\nReturn Book:\n";
    cout << "Book Code: ";
    cin >> code;
    
    int index = findBookByCode(code);
    if(index == -1) {
        cout << "Book not found!\n";
        return;
    }
    
    if(!books[index].isQueueEmpty()) {
        string person = books[index].getFromQueue();
        saveToHistory("Book given to " + person + ": " + books[index].title);
        cout << "Book given to " << person << ".\n";
    } else {
        books[index].available = true;
        saveForUndo(RETURN_BOOK, books[index], 
                   "Returned book: " + books[index].title);
        cout << "Book returned to library.\n";
    }
}

void sortByCode() {
    if(bookCount < 2) {
        cout << "Not enough books to sort.\n";
        return;
    }
    
    for(int i = 0; i < bookCount - 1; i++) {
        for(int j = 0; j < bookCount - i - 1; j++) {
            if(books[j].code > books[j + 1].code) {
                Book temp = books[j];
                books[j] = books[j + 1];
                books[j + 1] = temp;
            }
        }
    }
    
    saveForUndo(SORT_BOOKS, Book(), "Sorted by code");
    cout << "Books sorted by code.\n";
}

void sortByYear() {
    if(bookCount < 2) {
        cout << "Not enough books to sort.\n";
        return;
    }
    
    for(int i = 0; i < bookCount - 1; i++) {
        for(int j = 0; j < bookCount - i - 1; j++) {
            if(books[j].year > books[j + 1].year) {
                Book temp = books[j];
                books[j] = books[j + 1];
                books[j + 1] = temp;
            }
        }
    }
    
    saveForUndo(SORT_BOOKS, Book(), "Sorted by year");
    cout << "Books sorted by year.\n";
}

void displayAllBooks() {
    if(bookCount == 0) {
        cout << "Library is empty!\n";
        return;
    }
    
    cout << "\nAll Books (" << bookCount << " books):\n";
    for(int i = 0; i < bookCount; i++) {
        cout << "\n" << i + 1 << ". ";
        printBookInfo(books[i]);
    }
    saveToHistory("Displayed all books");
}

void undo() {
    if(undoTop < 0) {
        cout << "Nothing to undo!\n";
        return;
    }
    
    Action lastAction = undoStack[undoTop--];
    
    if(redoTop < 9) redoTop++;
    else {
        for(int i = 0; i < 9; i++) redoStack[i] = redoStack[i + 1];
        redoTop = 9;
    }
    redoStack[redoTop] = lastAction;
    
    switch(lastAction.type) {
        case ADD_BOOK:
            bookCount--;
            cout << "Undo: Book addition canceled.\n";
            break;
            
        case DELETE_BOOK:
            books[bookCount++] = lastAction.book;
            cout << "Undo: Book deletion canceled.\n";
            break;
            
        case BORROW_BOOK: {
            int idx = findBookByCode(lastAction.book.code);
            if(idx != -1) books[idx].available = true;
            cout << "Undo: Book borrowing canceled.\n";
            break;
        }
            
        case RETURN_BOOK: {
            int idx = findBookByCode(lastAction.book.code);
            if(idx != -1) books[idx].available = false;
            cout << "Undo: Book return canceled.\n";
            break;
        }
            
        case SORT_BOOKS:
            cout << "Undo: Sorting canceled.\n";
            break;
    }
    
    saveToHistory("Performed Undo");
}

void redo() {
    if(redoTop < 0) {
        cout << "Nothing to redo!\n";
        return;
    }
    
    Action nextAction = redoStack[redoTop--];
    saveForUndo(nextAction.type, nextAction.book, "Redo: " + nextAction.description);
    cout << "Redo performed.\n";
}

void showHistory() {
    if(historyCount == 0) {
        cout << "No history available.\n";
        return;
    }
    
    cout << "\nRecent Actions History:\n";
    cout << "================================\n";
    for(int i = historyCount - 1; i >= 0; i--) {
        cout << "- " << history[i] << "\n";
    }
    cout << "================================\n";
}

void showStatistics() {
    int available = 0, borrowed = 0, waiting = 0;
    
    for(int i = 0; i < bookCount; i++) {
        if(books[i].available) available++;
        else borrowed++;
        waiting += books[i].queueSize();
    }
    
    cout << "\nLibrary Statistics:\n";
    cout << "================================\n";
    cout << "Total Books: " << bookCount << "\n";
    cout << "Available Books: " << available << "\n";
    cout << "Borrowed Books: " << borrowed << "\n";
    cout << "Total Waiting Requests: " << waiting << "\n";
    cout << "Remaining Capacity: " << 200 - bookCount << "\n";
    cout << "================================\n";
    
    saveToHistory("Viewed library statistics");
}

int main() {
    int choice;
    
    cout << "==========================================\n";
    cout << "    Library Management System\n";
    cout << "==========================================\n";
    
    do {
        cout << "\n================================\n";
        cout << "        MAIN MENU\n";
        cout << "================================\n";
        cout << "1.  Add New Book\n";
        cout << "2.  Remove Book\n";
        cout << "3.  Search Book\n";
        cout << "4.  Borrow Book\n";
        cout << "5.  Return Book\n";
        cout << "6.  Sort Books by Code\n";
        cout << "7.  Sort Books by Year\n";
        cout << "8.  Display All Books\n";
        cout << "9.  Undo\n";
        cout << "10. Redo\n";
        cout << "11. Show History\n";
        cout << "12. Show Statistics\n";
        cout << "0.  Exit\n";
        cout << "================================\n";
        cout << "Your choice: ";
        cin >> choice;
        
        switch(choice) {
            case 1: addBook(); break;
            case 2: removeBook(); break;
            case 3: searchBook(); break;
            case 4: borrowBook(); break;
            case 5: returnBook(); break;
            case 6: sortByCode(); break;
            case 7: sortByYear(); break;
            case 8: displayAllBooks(); break;
            case 9: undo(); break;
            case 10: redo(); break;
            case 11: showHistory(); break;
            case 12: showStatistics(); break;
            case 0: cout << "\nThank you for using the system!\n"; break;
            default: cout << "Invalid choice!\n";
        }
        
    } while(choice != 0);
    
    return 0;
}