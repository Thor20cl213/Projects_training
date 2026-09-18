#include <atomic>
#include <iostream>

template<typename T>
class WeakPtr;

template<typename T>
struct ControlBlock
{
    T* ptr;

    std::atomic<size_t> shared_count;
    std::atomic<size_t> weak_count;

    explicit ControlBlock(T* p)
        : ptr(p)
        , shared_count(1)
        , weak_count(0)
    {
    }
};

template<typename T>
class SharedPtr
{
public:

    SharedPtr()
        : control_(nullptr)
    {
    }

    explicit SharedPtr(T* ptr)
        : control_(new ControlBlock<T>(ptr))
    {
    }

    SharedPtr(const SharedPtr& other)
        : control_(other.control_)
    {
        add_ref();
    }

    SharedPtr(SharedPtr&& other) noexcept
        : control_(other.control_)
    {
        other.control_ = nullptr;
    }

    ~SharedPtr()
    {
        release();
    }

    SharedPtr& operator=(const SharedPtr& other)
    {
        if (this != &other)
        {
            release();

            control_ = other.control_;
            add_ref();
        }

        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept
    {
        if (this != &other)
        {
            release();

            control_ = other.control_;
            other.control_ = nullptr;
        }

        return *this;
    }

    T* get() const
    {
        return control_ ? control_->ptr : nullptr;
    }

    T& operator*() const
    {
        return *get();
    }

    T* operator->() const
    {
        return get();
    }

    size_t use_count() const
    {
        return control_
            ? control_->shared_count.load()
            : 0;
    }

private:

    friend class WeakPtr<T>;

    explicit SharedPtr(ControlBlock<T>* control)
        : control_(control)
    {
        add_ref();
    }

    void add_ref()
    {
        if (control_)
        {
            control_->shared_count.fetch_add(
                1,
                std::memory_order_relaxed);
        }
    }

    void release()
    {
        if (!control_)
            return;

        if (control_->shared_count.fetch_sub(
                1,
                std::memory_order_acq_rel) == 1)
        {
            delete control_->ptr;
            control_->ptr = nullptr;

            if (control_->weak_count.load(
                    std::memory_order_acquire) == 0)
            {
                delete control_;
            }
        }

        control_ = nullptr;
    }

private:

    ControlBlock<T>* control_;
};

template<typename T>
class WeakPtr
{
public:

    WeakPtr()
        : control_(nullptr)
    {
    }

    WeakPtr(const SharedPtr<T>& shared)
        : control_(shared.control_)
    {
        add_ref();
    }

    WeakPtr(const WeakPtr& other)
        : control_(other.control_)
    {
        add_ref();
    }

    WeakPtr(WeakPtr&& other) noexcept
        : control_(other.control_)
    {
        other.control_ = nullptr;
    }

    ~WeakPtr()
    {
        release();
    }

    WeakPtr& operator=(const WeakPtr& other)
    {
        if (this != &other)
        {
            release();

            control_ = other.control_;
            add_ref();
        }

        return *this;
    }

    bool expired() const
    {
        return !control_
            || control_->shared_count.load() == 0;
    }

    SharedPtr<T> lock() const
    {
        if (!control_)
            return SharedPtr<T>();

        size_t current =
            control_->shared_count.load(
                std::memory_order_acquire);

        while (current != 0)
        {
            if (control_->shared_count.compare_exchange_weak(
                    current,
                    current + 1,
                    std::memory_order_acq_rel))
            {
                return SharedPtr<T>(control_);
            }
        }

        return SharedPtr<T>();
    }

private:

    void add_ref()
    {
        if (control_)
        {
            control_->weak_count.fetch_add(
                1,
                std::memory_order_relaxed);
        }
    }

    void release()
    {
        if (!control_)
            return;

        if (control_->weak_count.fetch_sub(
                1,
                std::memory_order_acq_rel) == 1)
        {
            if (control_->shared_count.load(
                    std::memory_order_acquire) == 0)
            {
                delete control_;
            }
        }

        control_ = nullptr;
    }

private:

    ControlBlock<T>* control_;
};

struct MyObject
{
    MyObject()
    {
        std::cout << "MyObject ctor\n";
    }

    ~MyObject()
    {
        std::cout << "MyObject dtor\n";
    }

    void hello()
    {
        std::cout << "hello\n";
    }
};

int main()
{
    SharedPtr<MyObject> sp1(new MyObject());

    {
        SharedPtr<MyObject> sp2 = sp1;

        std::cout
            << "shared count = "
            << sp1.use_count()
            << '\n';

        WeakPtr<MyObject> wp(sp1);

        if (auto locked = wp.lock();
            locked.get())
        {
            locked->hello();

            std::cout
                << "shared count = "
                << locked.use_count()
                << '\n';
        }
    }

    std::cout
        << "shared count = "
        << sp1.use_count()
        << '\n';
}