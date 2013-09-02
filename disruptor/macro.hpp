#ifndef MACRO_HPP_
#define MACRO_HPP_

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;      \
  void operator=(const TypeName&) = delete

#define DISALLOW_MOVE(TypeName)     \
	TypeName(TypeName &&) = delete; \
	void operator=(TypeName&&) = delete

#define DISALLOW_COPY_ASSIGN_MOVE(TypeName) \
  DISALLOW_COPY_AND_ASSIGN(TypeName);       \
  DISALLOW_MOVE(TypeName)

#define DISALLOW_CREATION(TypeName)      \
	DISALLOW_COPY_ASSIGN_MOVE(TypeName); \
	TypeName() = delete;                 \
	void *operator new(std::size_t) = delete

#ifndef CACHE_LINE_SIZE_IN_BYTES
	#define CACHE_LINE_SIZE_IN_BYTES 64
#endif

#define ATOMIC_SEQUENCE_PADDING_LENGTH \
    (CACHE_LINE_SIZE_IN_BYTES - sizeof(std::atomic<long>))
#define SEQUENCE_PADDING_LENGTH \
    (CACHE_LINE_SIZE_IN_BYTES - sizeof(long))

//#define INTERNAL_NAMESPACE_BEGIN namespace disruptor { namespace detail {
//#define INTERNAL_NAMESPACE_END }}

#define INTERNAL_NAMESPACE_BEGIN namespace disruptor { namespace detail {
#define INTERNAL_NAMESPACE_END }}

#endif /* MACRO_HPP_ */
