#ifndef MYQUEUE_HPP
#define MYQUEUE_HPP

template<typename T, int Size>
class Myqueue
{
public:
    class Iterator
    {
    public:
        Iterator(T* _buffer, int _id) : buffer(_buffer), id(_id) {}
        Iterator(const Iterator& other)
        {
            id = other.id;
            buffer = other.buffer;
        }
        T& operator*() {return buffer[id];}
        void operator++() {id = (id + 1) % maxsize;}
        friend bool operator!=(const Iterator& a, const Iterator& b)
        {
            return a.id != b.id;
        }
        T* operator->() {return &buffer[id];}
        int getid() const {return id;}

    private:
        T* buffer;
        int id;
    };
    void operator=(const Myqueue& other)
    {
        head = other.head;
        tail = other.tail;
        for (int i = 0; i < maxsize; i++)
            buffer[i] = other.buffer[i];
    }
    bool available() const
    {
        return (tail + 1) % maxsize != head;
    }
    bool empty() const
    {
        return head == tail;
    }
    void push(const T& x)
    {
        buffer[tail] = x;
        tail = (tail + 1) % maxsize;
    }
    void pop()
    {
        head = (head + 1) % maxsize;
    }
    void erase(int id)
    {
        for (int i = id; i != head; i = (i + maxsize - 1) % maxsize)
            buffer[i] = buffer[(i + maxsize - 1) % maxsize];
        head = (head + 1) % maxsize;
    }
    void clear()
    {
        head = tail = 0;
    }
    Iterator begin()
    {
        return Iterator(buffer, head);
    }
    Iterator end()
    {
        return Iterator(buffer, tail);
    }
    Iterator access(int id)
    {
        return Iterator(buffer, id);
    }

private:
    static constexpr int maxsize = Size + 1;
    int head = 0, tail = 0;
    T buffer[maxsize];
};

#endif