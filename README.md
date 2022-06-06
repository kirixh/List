# List

Реализован шаблонный класс List\<T, Allocator\>, аналог std::list из STL.

Реализация основана на стандартных нодах (класс Node), которые содержат указатели на лево и на право.

List поддерживает:

* Базовую функциональность + итераторы
* Move-семантику
* Аллокаторы
* Является exception-safety

## Методы:

### Using'и

* value_type
* allocator_type
* iterator

### Конструкторы

Класс реализовывает следующие конструкторы:

* List()
* List(size_t count, const T& value = T(), const Allocator& alloc = Allocator())
* explicit List(size_t count, const Allocator& alloc = Allocator())
* list(const list& other);
* List(List&& other);
* List(std::initializer_list\<T\> init, const Allocator& alloc = Allocator())

### Iterators (с поддержкой константных)

* begin()
* end()
* cbegin()
* cend()

List поддерживает range based for.

### operator=

* List& operator=(const List& other)
* List& operator=(list&& other) noexcept (std::allocator_traits\<Allocator\>::is_always_equal::value)


### element access methods

* T& front()
* const T& front() const
* T& back()
* const T& back() const


### Capacity

* bool empty()
* size_t size()

### Modifiers

* push_back(front)(const T&)
* push_back(front)(T&&)
* T& emplace_back(front)(Args&&... args)
* pop_back(front)();

### Поддержка move-семантики

* Класс умеет работать с OnlyMovable типами.

### Поддержка аллокаторов

* Работа с памятью только через аллокатор
* Конструирование и разрушение объектов только через аллокатор
* Поддержка propagate_on_container_copy(move) в соответствующих методах
* Используется rebind для аллоцирования и конструирования внутреннего класса Node

### Exception-safety

Общая концепция: если где-то выскочит исключение, контейнер возвращается в оригинальное состояние и пробрасывает исключение наверх.
