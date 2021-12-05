#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include "main.h"

class SymbolTable
{
public:
    int block;
    SymbolTable() : block(0) {}
    void run(string filename);
};
struct Slot
{
    string name;
    int scope, probing, index;
    enum Type
    {
        UNKNOWN,
        STRING,
        NUMBER,
        VOID
    } type;
    Type *param;
    int size_param;
    enum EntryType
    {
        ACTIVE,
        EMPTY,
        DELETED
    } info;
    Slot() : name(""), scope(0), probing(0), index(0), type(UNKNOWN), param(NULL), size_param(0), info(EMPTY) {}
    Slot(string name, int scope = 0, int n = 0, int index = 0, Type type = UNKNOWN, EntryType info = EMPTY) : name(name), scope(scope), probing(0), type(type), param(new Type[n]), size_param(n), info(info)
    {
        for (int i = 0; i < n; i++)
            param[i] = UNKNOWN;
    }
    long long encrypt()
    {
        string keygen;
        keygen += to_string(scope);
        for (int i = 0; i < (int)name.length(); i++)
            keygen += to_string(name[i] - 48);
        return stoll(keygen);
    }
    bool operator==(const Slot &other) const { return name == other.name && scope == other.scope; }
    bool operator!=(const Slot &other) const { return !(*this == other); }
    bool is_func()
    {
        return size_param != 0;
    }
};
class HashTable
{
private:
    Slot *arr_Slot;
    int currentSize, cap;
    bool isActive(int index) { return arr_Slot[index].info == Slot::ACTIVE; }
    virtual int findPost(Slot &) = 0;

public:
    HashTable() : arr_Slot(new Slot[10]), currentSize(0), cap(10)
    {
        for (int i = 0; i < cap; i++)
            arr_Slot[i].info = Slot::EMPTY;
    }
    HashTable(int size) : arr_Slot(new Slot[size]), currentSize(0), cap(size)
    {
        for (int i = 0; i < cap; i++)
            arr_Slot[i].info = Slot::EMPTY;
    }
    bool insert(Slot &slot)
    {
        int currentPos = findPost(slot);
        if (currentPos == -1)
            return false;
        arr_Slot[currentPos] = std::move(slot);
        arr_Slot[currentPos].info = Slot::ACTIVE;
        currentSize++;
        return true;
    }
    void remove(int block)
    {
        for (int i = 0; i < cap; i++)
            if (arr_Slot[i].scope == block)
            {
                arr_Slot[i].info = Slot::DELETED;
                currentSize--;
            }
    }
    int simpleHashing(long long key)
    {
        return key % cap;
    }
    int Capacity() { return cap; }
    Slot *arr_() { return arr_Slot; }
    Slot *find(string &name)
    {
        Slot *slot = NULL;
        for (int i = 0; i < cap; i++)
        {
            if (arr_Slot[i].info == Slot::ACTIVE && arr_Slot[i].name == name)
            {
                if (!slot || (slot && slot->scope < arr_Slot[i].scope))
                    slot = &arr_Slot[i];
            }
        }
        return slot;
    }
    void print()
    {
        string ss;
        for (int i = 0; i < cap; i++)
        {
            if (arr_Slot[i].info == Slot::ACTIVE)
            {
                ss += to_string(arr_Slot[i].index) + " " + arr_Slot[i].name + "//" + to_string(arr_Slot[i].scope) + ";";
            }
        }
        if (ss.length() > 0)
            cout << ss.substr(0, ss.length() - 1) << endl;
    }
    bool isFull() { return currentSize == cap; }
    void clear()
    {
        for (int i = 0; i < cap; i++)
        {
            arr_Slot[i].info = Slot::EMPTY;
        }
        delete arr_Slot;
    }
    ~HashTable()
    {
        clear();
    }
};
class LiHashTable : public HashTable
{
    int c;

public:
    LiHashTable() : HashTable(), c(0) {}
    LiHashTable(int size, int c) : HashTable(size), c(c) {}
    int findPost(Slot &slot)
    {
        int initPos = simpleHashing(slot.encrypt());
        int currentPos = initPos;
        int off = 1;
        while (arr_()[currentPos].info != Slot::EMPTY && arr_()[currentPos] != slot)
        {
            if (off == Capacity())
                return -1;
            currentPos = initPos + off * c;
            off++;
            while (currentPos >= Capacity())
                currentPos -= Capacity();
        }
        slot.probing = --off;
        cout << off << endl;
        slot.index = currentPos;
        return currentPos;
    }
};
class QuadHashTable : public HashTable
{
    int c1, c2;

public:
    QuadHashTable() : HashTable(), c1(0), c2(0) {}
    QuadHashTable(int size, int c1, int c2) : HashTable(size), c1(c1), c2(c2) {}
    int findPost(Slot &slot)
    {
        int initPos = simpleHashing(slot.encrypt());
        int currentPos = initPos;
        int off = 1;
        while (arr_()[currentPos].info != Slot::EMPTY && arr_()[currentPos] != slot)
        {
            if (off == Capacity())
                return -1;
            currentPos = initPos + off * c1 + off * off * c2;
            off++;
            while (currentPos >= Capacity())
                currentPos -= Capacity();
        }
        slot.probing = --off;
        cout << off << endl;
        slot.index = currentPos;
        return currentPos;
    }
};
class DouHashTable : public HashTable
{
    int c;

public:
    DouHashTable() : HashTable(), c(0) {}
    DouHashTable(int size, int c) : HashTable(size), c(c) {}
    int myHash2(long long key)
    {
        return 1 + key % (Capacity() - 2);
    }
    int findPost(Slot &slot)
    {
        int initPos = simpleHashing(slot.encrypt());
        int currentPos = initPos;
        int off = 1;
        while (arr_()[currentPos].info != Slot::EMPTY && arr_()[currentPos] != slot)
        {
            if (off == Capacity())
                return -1;
            currentPos = initPos + off * c * myHash2(slot.encrypt());
            off++;
            while (currentPos >= Capacity())
                currentPos -= Capacity();
        }
        slot.probing = --off;
        cout << off << endl;
        slot.index = currentPos;
        return currentPos;
    }
};
#endif