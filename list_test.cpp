// [genius_naming][nolint]

#define CATCH_CONFIG_MAIN

#include "list.hpp"
#include "utils.hpp"
#include "memory_utils.hpp"
#include "catch.hpp"

size_t MemoryManager::type_new_allocated = 0;
size_t MemoryManager::type_new_deleted = 0;
size_t MemoryManager::allocator_allocated = 0;
size_t MemoryManager::allocator_deallocated = 0;
size_t MemoryManager::allocator_constructed = 0;
size_t MemoryManager::allocator_destroyed = 0;

template <typename T, bool PropagateOnConstruct, bool PropagateOnAssign>
size_t WhimsicalAllocator<T, PropagateOnConstruct, PropagateOnAssign>::counter = 0;

size_t Accountant::ctor_calls = 0;
size_t Accountant::dtor_calls = 0;

bool ThrowingAccountant::need_throw = false;

void SetupTest() {
  MemoryManager::type_new_allocated = 0;
  MemoryManager::type_new_deleted = 0;
  MemoryManager::allocator_allocated = 0;
  MemoryManager::allocator_deallocated = 0;
  MemoryManager::allocator_constructed = 0;
  MemoryManager::allocator_destroyed = 0;
}

TEST_CASE("DefaultConstruct", "[List: Construct]") {
SetupTest();
{
List<TypeWithFancyNewDeleteOperators> l;
// NO LINT
REQUIRE(l.Size() == 0);
REQUIRE(l.Empty());
REQUIRE(MemoryManager::type_new_allocated == 0);
}
REQUIRE(MemoryManager::type_new_deleted == 0);
}

TEST_CASE("List:: List(size_t count)", "[List: Construct]") {
SetupTest();
{
constexpr size_t kSize = 10;
List<TypeWithFancyNewDeleteOperators> l(kSize);
REQUIRE(l.Size() == kSize);
REQUIRE(MemoryManager::type_new_allocated == 0);
}
REQUIRE(MemoryManager::type_new_deleted == 0);
}

TEST_CASE("List(size_t count, Allocator alloc)", "[List: Construct]") {
SetupTest();
AllocatorWithCount<TypeWithFancyNewDeleteOperators> alloc;
constexpr size_t kSize = 10;
{
List<TypeWithFancyNewDeleteOperators,
     AllocatorWithCount<TypeWithFancyNewDeleteOperators>> l(kSize, alloc);
REQUIRE(l.Size() == kSize);
REQUIRE(MemoryManager::type_new_allocated == 0);
REQUIRE(MemoryManager::type_new_deleted == 0);
REQUIRE(MemoryManager::allocator_allocated != 0);
REQUIRE(MemoryManager::allocator_constructed == kSize);
REQUIRE(alloc.allocator_allocated == 0);
REQUIRE(alloc.allocator_constructed == 0);
}
REQUIRE(MemoryManager::allocator_deallocated != 0);
REQUIRE(MemoryManager::allocator_destroyed == kSize);
REQUIRE(alloc.allocator_destroyed == 0);
REQUIRE(alloc.allocator_deallocated == 0);
}

TEST_CASE("List(size_t count, const T& value, Allocator alloc)", "[List: Construct]") {
SetupTest();
constexpr size_t kSize = 10;
AllocatorWithCount<TypeWithFancyNewDeleteOperators> alloc;
TypeWithFancyNewDeleteOperators default_value(1);
List<TypeWithFancyNewDeleteOperators, AllocatorWithCount<TypeWithFancyNewDeleteOperators>> l(kSize, default_value, alloc);
REQUIRE(l.Size() == kSize);
REQUIRE(MemoryManager::type_new_allocated == 0);
REQUIRE(MemoryManager::type_new_deleted == 0);
REQUIRE(MemoryManager::allocator_allocated != 0);
REQUIRE(MemoryManager::allocator_constructed == kSize);
REQUIRE(alloc.allocator_allocated == 0);
REQUIRE(alloc.allocator_constructed == 0);

for (auto it = l.Begin(); it != l.End(); ++it) {
REQUIRE(it->value == default_value.value);
}
}

TEST_CASE("List(std::initializer_list<T> init, const Allocator& alloc = Allocator())", "[List: Construct]") {
SetupTest();
AllocatorWithCount<int> alloc;
List<int, AllocatorWithCount<int>> l({1, 2, 3, 4, 5}, alloc);
REQUIRE(l.Size() == 5);
REQUIRE(MemoryManager::allocator_allocated != 0);
REQUIRE(MemoryManager::allocator_constructed == 5);
REQUIRE(alloc.allocator_allocated == 0);
REQUIRE(alloc.allocator_constructed == 0);
}

TEST_CASE("List(const List& other)", "[List: Construct]") {
SetupTest();
const size_t size = 5;
List<TypeWithCounts, AllocatorWithCount<TypeWithCounts>> l1 = {1, 2, 3, 4, 5};
List<TypeWithCounts, AllocatorWithCount<TypeWithCounts>> l2(l1);

REQUIRE(l2.Size() == size);
REQUIRE(MemoryManager::allocator_allocated != 0);
REQUIRE(MemoryManager::allocator_constructed == size * 2);
REQUIRE(AreListsEqual(l1, l2));
for (auto& value : l1) {
REQUIRE(*value.copy_c == 2);
}
}

TEST_CASE("OnlyMovable", "[List: Construct]") {
SetupTest();
List<OnlyMovable> list;
// NO LINT
REQUIRE(list.Size() == 0);
  list.EmplaceBack(0);
REQUIRE(list.Size() == 1);
OnlyMovable om(0);
  list.PushBack(std::move(om));
REQUIRE(list.Size() == 2);
}

TEST_CASE("List(List&& other)", "[List: Construct]") {
SetupTest();
const size_t size = 5;
List<TypeWithCounts, AllocatorWithCount<TypeWithCounts>> l1 = {1, 2, 3, 4, 5};
List<TypeWithCounts, AllocatorWithCount<TypeWithCounts>> l2(std::move(l1));

// NO LINT
REQUIRE(l1.Size() == 0);
REQUIRE(l2.Size() == size);
REQUIRE(MemoryManager::allocator_allocated != 0);
REQUIRE(MemoryManager::allocator_constructed == size);
for (auto& value : l1) {
REQUIRE(*value.copy_c == 1);
REQUIRE(*value.move_c == 1);
}
}

TEST_CASE("List& operator=(const List& other)", "[List: AssOperator]") {
SetupTest();
const size_t size = 5;
List<TypeWithCounts> l1 = {1, 2, 3, 4, 5};
List<TypeWithCounts> l2;
l2 = l1;
REQUIRE(l2.Size() == size);
REQUIRE(AreListsEqual(l1, l2));
for (auto& value : l1) {
REQUIRE(*value.copy_c == 2);
}
}

TEST_CASE("Basic func", "[List: basic funct]") {
SetupTest();
List<int> lst;

// NO LINT
REQUIRE(lst.Size() == 0);

  lst.PushBack(3);
  lst.PushBack(4);
  lst.PushFront(2);
  lst.PushBack(5);
  lst.PushFront(1);

std::reverse(lst.Begin(), lst.End());

REQUIRE(lst.Size() == 5);

std::string s;
for (int x: lst) {
s += std::to_string(x);
}
REQUIRE(s == "54321");
}

TEST_CASE("PropagateOnConstruct and PropagateOnAssign", "[List: TestWhimsicalAllocator]") {
SetupTest();
List<int, WhimsicalAllocator<int, true, true>> lst;

  lst.PushBack(1);
  lst.PushBack(2);

auto copy = lst;
assert(copy.GetAllocator() != lst.GetAllocator());

lst = copy;
assert(copy.GetAllocator() == lst.GetAllocator());
}

TEST_CASE("not PropagateOnConstruct and not PropagateOnAssign", "[List: TestWhimsicalAllocator]") {
SetupTest();
List<int, WhimsicalAllocator<int, false, false>> lst;

  lst.PushBack(1);
  lst.PushBack(2);

auto copy = lst;
assert(copy.GetAllocator() == lst.GetAllocator());

lst = copy;
assert(copy.GetAllocator() == lst.GetAllocator());
}

TEST_CASE("PropagateOnConstruct and not PropagateOnAssign", "[List: TestWhimsicalAllocator]") {
SetupTest();
List<int, WhimsicalAllocator<int, true, false>> lst;

  lst.PushBack(1);
  lst.PushBack(2);

auto copy = lst;
assert(copy.GetAllocator() != lst.GetAllocator());

lst = copy;
assert(copy.GetAllocator() != lst.GetAllocator());
}

TEST_CASE("TestAccountant", "[List: TestAccountant]") {
Accountant::reset();
{
List<Accountant> lst(5);
REQUIRE(lst.Size() == 5);
REQUIRE(Accountant::ctor_calls == 5);

List<Accountant> another = lst;
REQUIRE(another.Size() == 5);
REQUIRE(Accountant::ctor_calls == 10);
REQUIRE(Accountant::dtor_calls == 0);

  another.PopBack();
  another.PopFront();
REQUIRE(Accountant::dtor_calls == 2);

lst = another; // dtor_calls += 5, ctor_calls += 3
REQUIRE(another.Size() == 3);
REQUIRE(lst.Size() == 3);

REQUIRE(Accountant::ctor_calls == 13);
REQUIRE(Accountant::dtor_calls == 7);
} // dtor_calls += 6

REQUIRE(Accountant::ctor_calls == 13);
REQUIRE(Accountant::dtor_calls == 13);
}

TEST_CASE("Test exception safety", "[List: exception safety]") {
Accountant::reset();

ThrowingAccountant::need_throw = true;

try {
List<ThrowingAccountant> lst(8);
} catch (...) {
REQUIRE(Accountant::ctor_calls == 4);
REQUIRE(Accountant::dtor_calls == 4);
}

ThrowingAccountant::need_throw = false;
List<ThrowingAccountant> lst(8);

List<ThrowingAccountant> lst2;
for (int i = 0; i < 13; ++i) {
  lst2.PushBack(i);
}

Accountant::reset();
ThrowingAccountant::need_throw = true;

try {
auto lst3 = lst2;
} catch (...) {
REQUIRE(Accountant::ctor_calls == 4);
REQUIRE(Accountant::dtor_calls == 4);
}

Accountant::reset();

try {
lst = lst2;
} catch (...) {
REQUIRE(Accountant::ctor_calls == 4);
REQUIRE(Accountant::dtor_calls == 4);

// Actually it may not be 8 (although de facto it is), but the only thing we can demand here
// is the abscence of memory leaks
//
//assert(lst.Size() == 8);
}
}

TEST_CASE("Move", "[List: EmplaceBack/PushBack(T&&)]") {
TypeWithCounts t(0);
List<TypeWithCounts> l;
  l.EmplaceBack(0);
REQUIRE(*l.Begin()->int_c == 1);
REQUIRE(*l.Begin()->move_c == 0);
REQUIRE(*l.Begin()->copy_c == 0);

  l.PushFront(std::move(t));
REQUIRE(*l.Begin()->move_c == 1);
REQUIRE(*l.Begin()->copy_c == 0);
}
