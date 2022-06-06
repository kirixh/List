#include <iostream>
#include <list>

template <typename T, typename Allocator = std::allocator<T>>
class List {
  template <bool is_const>
  class ListIterator;

  public:
  using value_type = T;
  using allocator_type = Allocator;
  using iterator = ListIterator<false>;
  using const_iterator = ListIterator<true>;

  List();

  explicit List(size_t count, const T& value,
                const Allocator& alloc = Allocator());

  explicit List(size_t count, const Allocator& alloc = Allocator());

  List(const List& other);

  List(List&& other) noexcept;

  List(std::initializer_list<value_type> init,
       const Allocator& alloc = Allocator());

  ~List();

  List& operator=(const List& other);
  List& operator=(List&& other) noexcept;

  [[nodiscard]] size_t Size() const { return size_; };

  [[nodiscard]] bool Empty() const { return size_ == 0; }
  [[nodiscard]] allocator_type GetAllocator() const noexcept { return alloc_; }

  [[nodiscard]] iterator Begin() const;
  [[nodiscard]] const_iterator Cbegin() const;
  [[nodiscard]] iterator End() const;
  [[nodiscard]] const_iterator Cend() const;

  value_type& Front();
  [[nodiscard]] const value_type& Front() const;
  value_type& Back();
  [[nodiscard]] const value_type& Back() const;

  template <typename... Args>
  void EmplaceBack(Args&&... args);

  template <typename... Args>
  void EmplaceFront(Args&&... args);

  template <typename U>
  void PushBack(U&& value);

  template <typename U>
  void PushFront(U&& value);

  void PopBack();
  void PopFront();

  private:
  struct Node;
  Node* head_;
  Node* tail_;
  Node* x_;
  size_t size_ = 0;

  using alloc_traits = typename std::allocator_traits<
      allocator_type>::template rebind_traits<Node>;
  using alloc_type = typename std::allocator_traits<
      allocator_type>::template rebind_alloc<Node>;
  alloc_type alloc_;

  void SetEnds();

  Node* MakeNode();

  Node* MakeNode(value_type& value);

  Node* MakeNode(const Node& other);

  template <typename... Args>
  Node* MakeNode(Args&&... args);

  void FillList(size_t count);

  void FillList(size_t count, T value);

  void FillList(const List<T, Allocator>& other);

  void FillList(std::initializer_list<T> init_list);

  void CleanList(Node* current, Node* next_node, bool dealloc = true);
};

template <typename T, typename Allocator>
template <bool is_const>
class List<T, Allocator>::ListIterator
    : public std::iterator<std::bidirectional_iterator_tag, T> {
  friend class List<T, Allocator>;

  public:
  using node = std::conditional_t<is_const, const Node, Node>;
  using value_type = List<T, Allocator>::value_type;
  using ptr =
      std::conditional_t<is_const, const List<T, Allocator>::value_type*,
                         List<T, Allocator>::value_type*>;
  using ref =
      std::conditional_t<is_const, const List<T, Allocator>::value_type&,
                         List<T, Allocator>::value_type&>;

  using iterator_category = std::bidirectional_iterator_tag;
  explicit ListIterator(node* node_p) : node_p_(node_p){};
  ListIterator(const ListIterator<is_const>& it) : node_p_(it.node_p_) {}

  value_type operator*() const { return node_p_->value; }

  ref operator*() { return node_p_->value; }

  ptr operator->() const { return &(node_p_->value); }

  ListIterator& operator++() {
    node_p_ = node_p_->next;
    return *this;
  }

  ListIterator& operator--() {
    node_p_ = node_p_->prev;
    return *this;
  }

  bool operator==(const ListIterator& other) {
    return node_p_ == other.node_p_;
  }

  bool operator!=(const ListIterator& other) {
    return node_p_ != other.node_p_;
  }

  private:
  node* node_p_;
};

////////////////////////////////////////////////////////////////////////////////

template <typename T, typename Allocator>
struct List<T, Allocator>::Node {
  Node() = default;

  Node(const Node& other)
      : next(other.next), prev(other.prev), value(other.value) {}

  explicit Node(const T& value) : value(value){};

  explicit Node(T&& value) : value(std::move(value)) {}

  Node(Node&& other) noexcept
      : next(other.next), prev(other.prev), value(std::move(other.value)) {
    next = nullptr;
    prev = nullptr;
  }

  template <typename... Args>
  explicit Node(Node* fictive, Args&&... args)
      : next(fictive), value(std::forward<Args>(args)...) {}

  ~Node() = default;

  Node* next = nullptr;
  Node* prev = nullptr;
  T value;
};

template <typename T, typename Allocator>
void List<T, Allocator>::SetEnds() {
  head_->prev = x_;
  tail_->next = x_;
  x_->next = head_;
  x_->prev = tail_;
}

template <typename T, typename Allocator>
typename List<T, Allocator>::Node* List<T, Allocator>::MakeNode(
    const List::Node& other) {
  Node* new_node = alloc_traits::allocate(alloc_, 1);
  try {
    alloc_traits::construct(alloc_, new_node, other);
  } catch (...) {
    alloc_traits::deallocate(alloc_, new_node, 1);
    throw;
  }
  return new_node;
}

template <typename T, typename Allocator>
typename List<T, Allocator>::Node* List<T, Allocator>::MakeNode(
    value_type& value) {
  Node* new_node = alloc_traits::allocate(alloc_, 1);
  try {
    alloc_traits::construct(alloc_, new_node, value);
  } catch (...) {
    alloc_traits::deallocate(alloc_, new_node, 1);
    throw;
  }
  return new_node;
}

template <typename T, typename Allocator>
typename List<T, Allocator>::Node* List<T, Allocator>::MakeNode() {
  Node* new_node = alloc_traits::allocate(alloc_, 1);
  try {
    alloc_traits::construct(alloc_, new_node);
  } catch (...) {
    alloc_traits::deallocate(alloc_, new_node, 1);
    throw;
  }
  return new_node;
}

template <typename T, typename Allocator>
template <typename... Args>
typename List<T, Allocator>::Node* List<T, Allocator>::MakeNode(
    Args&&... args) {
  Node* new_node = alloc_traits::allocate(alloc_, 1);
  try {
    alloc_traits::construct(alloc_, new_node, std::forward<Args>(args)...);
  } catch (...) {
    alloc_traits::deallocate(alloc_, new_node, 1);
    throw;
  }
  return new_node;
}

template <typename T, typename Allocator>
void List<T, Allocator>::CleanList(List::Node* current, List::Node* next_node,
                                   bool dealloc) {
  if (Empty()) {
    return;
  }
  if (current == next_node) {
    current = current->prev;
  }
  while (next_node != x_) {
    alloc_traits::destroy(alloc_, next_node);
    if (dealloc) {
      alloc_traits::deallocate(alloc_, next_node, 1);
    }
    next_node = current;
    current = current->prev;
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::FillList(size_t count, value_type value) {
  Node* current = x_;
  Node* next_node;
  try {
    for (size_t i = 0; i < count; ++i) {
      next_node = MakeNode(value);
      current->next = next_node;
      next_node->prev = current;
      current = next_node;
    }
  } catch (...) {
    CleanList(current, next_node);
    alloc_traits::deallocate(alloc_, x_, 1);
    throw;
  }
  head_ = x_->next;
  tail_ = next_node;
  SetEnds();
}

template <typename T, typename Allocator>
void List<T, Allocator>::FillList(size_t count) {
  Node* current = x_;
  Node* next_node;
  try {
    for (size_t i = 0; i < count; ++i) {
      next_node = MakeNode();
      current->next = next_node;
      next_node->prev = current;
      current = next_node;
    }
  } catch (...) {
    CleanList(current, next_node);
    alloc_traits::deallocate(alloc_, x_, 1);
    throw;
  }
  head_ = x_->next;
  tail_ = next_node;
  SetEnds();
}

template <typename T, typename Allocator>
void List<T, Allocator>::FillList(const List<T, Allocator>& other) {
  Node* current = x_;
  Node* next_node;
  Node* other_current = other.x_;
  try {
    while (other_current != other.tail_) {
      other_current = other_current->next;
      next_node = MakeNode(*other_current);
      current->next = next_node;
      next_node->prev = current;
      current = next_node;
    }
  } catch (...) {
    CleanList(current, next_node);
    alloc_traits::deallocate(alloc_, x_, 1);
    throw;
  }
  head_ = x_->next;
  tail_ = next_node;
  SetEnds();
}

template <typename T, typename Allocator>
void List<T, Allocator>::FillList(std::initializer_list<value_type> init_list) {
  Node* current = x_;
  Node* next_node;
  try {
    for (auto iter = init_list.begin(); iter != init_list.end(); ++iter) {
      next_node = MakeNode(*iter);
      current->next = next_node;
      next_node->prev = current;
      current = next_node;
    }
  } catch (...) {
    CleanList(current, next_node);
    alloc_traits::deallocate(alloc_, x_, 1);
    throw;
  }
  head_ = x_->next;
  tail_ = next_node;
  SetEnds();
}

/// -------------------------------Constructors---------------------------------

template <typename T, typename Allocator>
List<T, Allocator>::List() {
  x_ = alloc_traits::allocate(alloc_, 1);
  x_->next = head_;
  x_->prev = tail_;
}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const T& value, const Allocator& alloc) {
  size_ = count;
  alloc_ = alloc;
  x_ = alloc_traits::allocate(alloc_, 1);
  x_->next = head_;
  x_->prev = tail_;
  FillList(count, value);
}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const Allocator& alloc) {
  size_ = count;
  alloc_ = alloc;
  x_ = alloc_traits::allocate(alloc_, 1);
  x_->next = head_;
  x_->prev = tail_;
  FillList(count);
}
template <typename T, typename Allocator>
List<T, Allocator>::List(const List& other) {
  size_ = other.size_;
  alloc_ = alloc_traits::select_on_container_copy_construction(other.alloc_);
  x_ = alloc_traits::allocate(alloc_, 1);
  x_->next = head_;
  x_->prev = tail_;
  FillList(other);
}

template <typename T, typename Allocator>
List<T, Allocator>::List(List&& other) noexcept
    : head_(std::move(other.head_)),
      tail_(std::move(other.tail_)),
      x_(std::move(other.x_)),
      size_(other.size_),
      alloc_(std::move(other.alloc_)) {
  x_->next = head_;
  x_->prev = tail_;
  other.head_ = nullptr;
  other.tail_ = nullptr;
  other.size_ = 0;
  other.x_ = alloc_traits::allocate(other.alloc_, 1);
  other.x_->next = other.head_;
  other.x_->prev = other.tail_;
}

template <typename T, typename Allocator>
List<T, Allocator>::List(std::initializer_list<value_type> init,
                         const Allocator& alloc) {
  size_ = init.size();
  alloc_ = alloc;
  x_ = alloc_traits::allocate(alloc_, 1);
  x_->next = head_;
  x_->prev = tail_;
  FillList(init);
}

template <typename T, typename Allocator>
List<T, Allocator>::~List() {
  if (Empty()) {
    alloc_traits::deallocate(alloc_, x_, 1);
    return;
  }
  CleanList(tail_->prev, tail_);
  size_ = 0;
  head_ = nullptr;
  tail_ = nullptr;
  x_->next = nullptr;
  x_->prev = nullptr;
  alloc_traits::deallocate(alloc_, x_, 1);
}

/// -------------------------------Operators------------------------------------

template <typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(
    const List<T, Allocator>& other) {
  if (this == &other) {
    return *this;
  }
  List<T, Allocator> tmp = other;
  std::swap(size_, tmp.size_);
  std::swap(head_, tmp.head_);
  std::swap(tail_, tmp.tail_);
  std::swap(x_, tmp.x_);

  if (alloc_traits::propagate_on_container_copy_assignment::value &&
      alloc_ != other.alloc_) {
    alloc_ = other.alloc_;
  }
  return *this;
}

template <typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(
    List<T, Allocator>&& other) noexcept {
  List<T, Allocator> tmp = std::move(other);
  std::swap(size_, tmp.size_);
  std::swap(head_, tmp.head_);
  std::swap(tail_, tmp.tail_);
  std::swap(x_, tmp.x_);
  if (alloc_traits::propagate_on_container_move_assignment::value &&
      alloc_ != tmp.alloc_) {
    std::swap(alloc_, tmp.alloc_);
  }
  return *this;
}

/// -------------------------------Iterators------------------------------------

template <typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::Begin() const {
  if (head_ == nullptr) {
    return List::iterator(x_);
  }
  return List::iterator(head_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::Cbegin() const {
  if (head_ == nullptr) {
    return List::const_iterator(x_);
  }
  return List::const_iterator(head_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::End() const {
  return List::iterator(x_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::Cend() const {
  return List::const_iterator(x_);
}

/// -----------------------Element access methods-------------------------------

template <typename T, typename Allocator>
T& List<T, Allocator>::Front() {
  return head_->value;
}

template <typename T, typename Allocator>
const T& List<T, Allocator>::Front() const {
  return head_->value;
}

template <typename T, typename Allocator>
T& List<T, Allocator>::Back() {
  return tail_->value;
}

template <typename T, typename Allocator>
const T& List<T, Allocator>::Back() const {
  return tail_->value;
}

/// ------------------------------Modifiers-------------------------------------

template <typename T, typename Allocator>
template <typename... Args>
void List<T, Allocator>::EmplaceBack(Args&&... args) {
  Node* next_node = nullptr;
  try {
    next_node = MakeNode(nullptr, std::forward<Args>(args)...);
  } catch (...) {
    alloc_traits::destroy(alloc_, next_node);
    alloc_traits::deallocate(alloc_, next_node, 1);
    alloc_traits::deallocate(alloc_, x_, 1);
    throw;
  }
  if (Empty()) {
    ++size_;
    head_ = next_node;
    tail_ = next_node;
    SetEnds();
    return;
  }
  ++size_;
  Node* x_prev = x_->prev;
  x_prev->next = next_node;
  tail_ = next_node;
  x_->prev = next_node;
  next_node->prev = x_prev;
  next_node->next = x_;
}

template <typename T, typename Allocator>
template <typename... Args>
void List<T, Allocator>::EmplaceFront(Args&&... args) {
  Node* next_node = nullptr;
  try {
    next_node = MakeNode(nullptr, std::forward<Args>(args)...);
  } catch (...) {
    alloc_traits::destroy(alloc_, next_node);
    alloc_traits::deallocate(alloc_, next_node, 1);
    alloc_traits::deallocate(alloc_, x_, 1);
    throw;
  }
  if (Empty()) {
    ++size_;
    head_ = next_node;
    tail_ = next_node;
    SetEnds();
    return;
  }
  ++size_;
  Node* head_prev = head_;
  x_->next = next_node;
  head_ = next_node;
  head_prev->prev = next_node;
  next_node->prev = x_;
  next_node->next = head_prev;
}

template <typename T, typename Allocator>
template <typename U>
void List<T, Allocator>::PushBack(U&& value) {
  EmplaceBack(std::forward<U>(value));
}

template <typename T, typename Allocator>
template <typename U>
void List<T, Allocator>::PushFront(U&& value) {
  EmplaceFront(std::forward<U>(value));
}

template <typename T, typename Allocator>
void List<T, Allocator>::PopBack() {
  if (Empty()) {
    return;
  }
  Node* old_tail = tail_;
  x_->prev = tail_->prev;
  tail_->prev->next = x_;
  tail_ = tail_->prev;
  alloc_traits::destroy(alloc_, old_tail);
  alloc_traits::deallocate(alloc_, old_tail, 1);
  --size_;
  if (Empty()) {
    head_ = nullptr;
    tail_ = nullptr;
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::PopFront() {
  if (Empty()) {
    return;
  }
  Node* old_head = head_;
  x_->next = head_->next;
  head_->next->prev = x_;
  head_ = head_->next;
  alloc_traits::destroy(alloc_, old_head);
  alloc_traits::deallocate(alloc_, old_head, 1);
  --size_;
  if (Empty()) {
    head_ = nullptr;
    tail_ = nullptr;
  }
}