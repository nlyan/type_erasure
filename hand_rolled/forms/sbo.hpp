%struct_prefix%
{
public:
    // Contructors
    %struct_name% () :
        handle_ (nullptr),
        buffer_ {}
    {}

    template <typename T>
    %struct_name% (T value) :
        handle_ (nullptr),
        buffer_ {}
    { handle_ = clone_impl(std::move(value), buffer_); }

    %struct_name% (const %struct_name% & rhs) :
        handle_ (nullptr),
        buffer_ {}
    {
        if (rhs.handle_)
            handle_ = rhs.handle_->clone_into(buffer_);
    }

    %struct_name% (%struct_name% && rhs) noexcept :
        handle_ (nullptr),
        buffer_ {}
    { swap(rhs.handle_, rhs.buffer_); }

    // Assignment
    template <typename T>
    %struct_name% & operator= (T value)
    {
        reset();
        handle_ = clone_impl(std::move(value), buffer_);
        return *this;
    }

    %struct_name% & operator= (const %struct_name% & rhs)
    {
        %struct_name% temp(rhs);
        swap(temp.handle_, temp.buffer_);
        return *this;
    }

    %struct_name% & operator= (%struct_name% && rhs) noexcept
    {
        %struct_name% temp(std::move(rhs));
        swap(temp.handle_, temp.buffer_);
        return *this;
    }

    ~%struct_name% ()
    { reset(); }

    %nonvirtual_members%

private:
    typedef std::array<unsigned char, 24> buffer;

    struct handle_base
    {
        virtual ~handle_base () {}
        virtual handle_base * clone_into (buffer & buf) const = 0;
        virtual bool heap_allocated () const = 0;
        virtual void destroy () = 0;

        %pure_virtual_members%
    };

    template <typename T, bool HeapAllocated>
    struct handle :
        handle_base
    {
        template <typename U = T>
        handle (T value,
                typename std::enable_if<
                    std::is_reference<U>::value
                >::type * = 0) :
            value_ (value)
        {}

        template <typename U = T>
        handle (T value,
                typename std::enable_if<
                    !std::is_reference<U>::value,
                    int
                >::type * = 0) noexcept :
            value_ (std::move(value))
        {}

        virtual handle_base * clone_into (buffer & buf) const
        { return clone_impl(value_, buf); }

        virtual bool heap_allocated () const
        { return HeapAllocated; }

        virtual void destroy ()
        {
            if (HeapAllocated)
                delete this;
            else
                this->~handle();
        }

        %virtual_members%

        T value_;
    };

    template <typename T, bool HeapAllocated>
    struct handle<std::reference_wrapper<T>, HeapAllocated> :
        handle<T &, HeapAllocated>
    {
        handle (std::reference_wrapper<T> ref) :
            handle<T &, HeapAllocated> (ref.get())
        {}
    };

    template <typename T>
    static handle_base * clone_impl (T value, buffer & buf)
    {
        handle_base * retval = nullptr;
        typedef typename std::remove_reference<T>::type handle_t;
        void * buf_ptr = &buf;
        std::size_t buf_size = sizeof(buf);
        const std::size_t alignment = alignof(handle<handle_t, false>);
        const std::size_t size = sizeof(handle<handle_t, false>);
        buf_ptr = std::align(alignment, size, buf_ptr, buf_size);
        if (buf_ptr) {
            new (buf_ptr) handle<handle_t, false>(std::move(value));
            retval = static_cast<handle_base *>(buf_ptr);
        } else {
            retval = new handle<handle_t, true>(std::move(value));
        }
        return retval;
    }

    void swap (handle_base * & rhs_handle, buffer & rhs_buffer)
    {
        const bool this_heap_allocated =
            !handle_ || handle_->heap_allocated();
        const bool rhs_heap_allocated =
            !rhs_handle || rhs_handle->heap_allocated();

        if (this_heap_allocated && rhs_heap_allocated) {
            std::swap(handle_, rhs_handle);
        } else if (this_heap_allocated) {
            const std::ptrdiff_t offset = handle_offset(rhs_handle, rhs_buffer);
            rhs_handle = handle_;
            buffer_ = rhs_buffer;
            handle_ = handle_ptr(char_ptr(&buffer_) + offset);
        } else if (rhs_heap_allocated) {
            const std::ptrdiff_t offset = handle_offset(handle_, buffer_);
            handle_ = rhs_handle;
            rhs_buffer = buffer_;
            rhs_handle = handle_ptr(char_ptr(&rhs_buffer) + offset);
        } else {
            const std::ptrdiff_t this_offset =
                handle_offset(handle_, buffer_);
            const std::ptrdiff_t rhs_offset =
                handle_offset(rhs_handle, rhs_buffer);
            std::swap(buffer_, rhs_buffer);
            handle_ = handle_ptr(char_ptr(&buffer_) + rhs_offset);
            rhs_handle = handle_ptr(char_ptr(&rhs_buffer) + this_offset);
        }
    }

    void reset ()
    {
        if (handle_)
            handle_->destroy();
    }

    template <typename T>
    static unsigned char * char_ptr (T * ptr)
    { return static_cast<unsigned char *>(static_cast<void *>(ptr)); }

    static handle_base * handle_ptr (unsigned char * ptr)
    { return static_cast<handle_base *>(static_cast<void *>(ptr)); }

    static std::ptrdiff_t handle_offset (handle_base * h, buffer & b)
    {
        assert(h);
        unsigned char * const char_handle = char_ptr(h);
        unsigned char * const char_buffer = char_ptr(&b);
        return char_handle - char_buffer;
    }

    handle_base * handle_;
    buffer buffer_;
};
